#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"

#if IBM
#include <windows.h>
#endif

#if LIN
#include <GL/gl.h>
#elif __GNUC__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifndef XPLM301
#error This is made to be compiled against the XPLM301 SDK
#endif

#include "XPLMDisplay.h"
#include "XPLMProcessing.h"
#include "XPLMPlugin.h"

#include <string>
#include <cstring>
#include <cstdio>
#include <ctime>

#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <google/protobuf/util/time_util.h>

#include "xplane.pb.h"

// TODO: How to support multiple X-Plane instances on the same machine?
constexpr auto& RPC_URL = "tcp://0.0.0.0:27471";
constexpr auto& PUB_URL = "tcp://0.0.0.0:27472";

// These globals are set during init
XPLMPluginID pluginId;
time_t startTime;
nng_socket pubSock;
XPLMFlightLoopID afterFlightLoopID;

xplane::Message infoMsg;
xplane::Message statsMsg;
xplane::Stats *stats;

void update_screen_info() {
  // TODO: These can change during a flight
  auto info = infoMsg.mutable_info();

  // Bounds of the global X-Plane desktop
  // Note that we're not guaranteed that the main monitor's lower left is at (0, 0)...
  // We'll need to query for the global desktop bounds!
  int left, bottom, right, top;
  XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
  info->clear_screen_bounds();
  auto bounds = info->mutable_screen_bounds();
  bounds->set_left(left);
  bounds->set_bottom(bottom);
  bounds->set_right(right);
  bounds->set_top(top);

  // Information about all full screens used by X-Plane (not in window mode!)
  info->clear_monitor_bounds();
  XPLMGetAllMonitorBoundsGlobal([] (int monIdx, int left, int top,
                                    int right, int bottom, void*) {
    auto info = infoMsg.mutable_info();
    auto bounds = info->add_monitor_bounds();
    bounds->set_left(left);
    bounds->set_bottom(bottom);
    bounds->set_right(right);
    bounds->set_top(top);
    bounds->set_id(monIdx);
    std::printf("[NanoMSG] XPLMGetAllMonitorBoundsGlobal: #%i [%i %i %i %i]\n",
                monIdx, left, top, right, bottom);
  }, nullptr);
}

void warn_nng(const std::string& name, int rv) {
  auto msg = nng_strerror(rv);
  std::printf("[NanoMSG] WARNING: nng error: %s: %s\n", name.c_str(), msg);
};

void publish(const std::string& topic, const std::string& pb) {
  if (pb.empty()) {
    std::printf("[NanoMSG] WARNING: empty protobuf\n");
  }
  std::string buf = topic;
  buf += '=';
  buf += pb;
  //std::cout << "@@@ " << pb.size() << " " << buf << std::endl;
  int rv;
  if ((rv = nng_send(pubSock, (void *) buf.c_str(), buf.length(), NNG_FLAG_NONBLOCK)) != 0) {
    std::string s = "nng_send #" + topic;
    warn_nng(s, rv);
    stats->set_last_publish_error_code(rv);
    stats->set_total_publish_errors(stats->total_publish_errors() + 1);
  }
}

// Type: XPLMFlightLoop_f
float afterFlightLoop(float  inElapsedSinceLastCall,
                      float  inElapsedTimeSinceLastFlightLoop,
                      int    inCounter,
                      void * inRefcon) {

  auto t0 = std::chrono::high_resolution_clock::now();

  // Publish stats
  stats->set_elapsed_since_last_call(inElapsedSinceLastCall);
  stats->set_elapsed_time_since_last_flight_loop(inElapsedTimeSinceLastFlightLoop);
  stats->set_flight_loop_counter(inCounter);
  publish("stats", statsMsg.SerializeAsString());

  update_screen_info(); // TODO: remove
  publish("info", infoMsg.SerializeAsString());

  // Store how long the handler took this time, and report the next time it is run
  auto t1 = std::chrono::high_resolution_clock::now();
  auto dt = t1 - t0;
  auto usec = std::chrono::duration_cast<std::chrono::microseconds>(dt);
  stats->set_handler_time_usec(usec.count());

  // TODO: Maybe too frequent?
  return -1.0; // Call again next frame
}

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
  std::strcpy(outName, "NanoMSG");
  std::strcpy(outSig, "nl.wojas.xplane.nanomsg");
  std::strcpy(outDesc, "A NanoMSG + Protobuf remote interface to X-Plane");

  XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", true);
  // TODO: Perhaps enable once we support drawing windows (allows VR)
  //XPLMEnableFeature("XPLM_USE_NATIVE_WIDGET_WINDOWS", true);

  // Open PUB socket
  int rv;
  auto report_fatal_nng = [&] (const std::string& name) -> bool {
    auto msg = nng_strerror(rv);
    std::printf("[NanoMSG] FATAL: nng init: %s: %s\n", name.c_str(), msg);
    return false; // Plugin init failed
  };
  if ((rv = nng_pub0_open(&pubSock)) != 0) {
    return report_fatal_nng("pub: nng_pub0_open");
  }
  if ((rv = nng_setopt_int(pubSock, NNG_OPT_SENDBUF, 1024)) < 0) {
    return report_fatal_nng("pub: nng_setopt_int NNG_OPT_SENDBUF");
  }
  if ((rv = nng_listen(pubSock, PUB_URL, NULL, 0)) < 0) {
    return report_fatal_nng("pub: nng_listen");
  }

  // Fill plugin globals
  pluginId = XPLMGetMyID();
  startTime = std::time(nullptr);

  // Fill our Stats message protobuf
  statsMsg.set_msg_type(xplane::Message_Type_Stats);
  stats = statsMsg.mutable_stats();

  // Fill our Info message protobuf
  infoMsg.set_msg_type(xplane::Message_Type_Info);
  infoMsg.clear_info();
  auto info = infoMsg.mutable_info();
  info->set_plugin_id(pluginId);
  info->set_start_time_unix(startTime);
  update_screen_info();

  // Publish Info message
  // TODO: Publish periodically
  publish("info", infoMsg.SerializeAsString());

  // Register flight loop callback
  // TODO: Does the called func keep a reference?
  XPLMCreateFlightLoop_t cfl;
  cfl.structSize = sizeof(cfl);
  cfl.phase = xplm_FlightLoop_Phase_AfterFlightModel;
  cfl.callbackFunc = afterFlightLoop;
  cfl.refcon = nullptr;
  afterFlightLoopID = XPLMCreateFlightLoop(&cfl);
  XPLMScheduleFlightLoop(afterFlightLoopID, -1.0, false);

  return true; // plugin init successful
}

PLUGIN_API void XPluginStop(void) {
  if (afterFlightLoopID != 0) {
    XPLMDestroyFlightLoop(afterFlightLoopID);
  }
  int rv;
  if ((rv = nng_close(pubSock)) != 0) {
    warn_nng("pub: nng_close", rv);
  }
}

PLUGIN_API void XPluginDisable(void) {
}

PLUGIN_API int XPluginEnable(void) {
  return true; // Required to indicate we can be enabled
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void *inParam) {
  xplane::Message m;
  m.set_msg_type(xplane::Message_Type_PluginMessage);
  auto p = m.mutable_plugin_message();
  p->set_plugin_id(inFrom);
  p->set_msg_id(inMsg);
  p->set_param((unsigned long long) inParam); // The pointer value
  // TODO: Perhaps lookup plugin name
  publish("receive_message", m.SerializeAsString());
}


#pragma clang diagnostic pop