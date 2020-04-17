#if IBM
#include <windows.h>
#endif

// TODO: We do not use OpenGL yet
#if LIN
#include <GL/gl.h>
#elif __GNUC__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

// TODO: Perhaps 300 is sufficient?
#ifndef XPLM301
#error This is made to be compiled against the XPLM301 SDK
#endif

#include "XPLMProcessing.h"
#include "XPLMPlugin.h"

#include <string>
#include <cstring>
#include <ctime>

#include <fmt/format.h>

#include "xplane.pb.h"
#include "Publisher.h"
#include "Statistics.h"
#include "Info.h"

// TODO: How to support multiple X-Plane instances on the same machine?
constexpr auto& RPC_URL = "tcp://0.0.0.0:27471";
constexpr auto& PUB_URL = "tcp://0.0.0.0:27472";

// These globals are set during init
XPLMPluginID pluginId;
time_t startTime;
XPLMFlightLoopID afterFlightLoopID;

Statistics *stats;
Info *info;
Publisher *publisher;


// Type: XPLMFlightLoop_f
float afterFlightLoop(float  inElapsedSinceLastCall,
                      float  inElapsedTimeSinceLastFlightLoop,
                      int    inCounter,
                      void * inRefcon) {

  auto t0 = std::chrono::high_resolution_clock::now();

  // Publish stats
  stats->st->set_elapsed_since_last_call(inElapsedSinceLastCall);
  stats->st->set_elapsed_time_since_last_flight_loop(inElapsedTimeSinceLastFlightLoop);
  stats->st->set_flight_loop_counter(inCounter);
  publisher->publishStats();

  info->updateScreenInfo(); // TODO: remove
  publisher->publishInfo(info);

  // Store how long the handler took this time, and report the next time it is run
  auto t1 = std::chrono::high_resolution_clock::now();
  auto dt = t1 - t0;
  auto usec = std::chrono::duration_cast<std::chrono::microseconds>(dt);
  stats->st->set_handler_time_usec(usec.count());

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

  // Fill plugin globals
  pluginId = XPLMGetMyID();
  startTime = std::time(nullptr);
  fmt::print(FMT_STRING("[NanoMSG] XPluginStart pluginId={}\n"), pluginId);

  // Fill our Stats message protobuf
  stats = new Statistics;

  // Open PUB socket
  publisher = new Publisher(PUB_URL, stats);
  if (!publisher->open()) {
    fmt::print(FMT_STRING("[NanoMSG] FATAL: nng pub init: {}\n"), publisher->lastError());
    return false; // Plugin init failed
  }

  // Fill our Info message protobuf
  info = new Info;
  info->updateScreenInfo();

  // Publish Info message
  // TODO: Publish periodically
  publisher->publishInfo(info);

  // Register flight loop callback
  // TODO: Does the called func keep a reference?
  // TODO: Move to XPluginEnable?
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
  if (afterFlightLoopID != nullptr) {
    XPLMDestroyFlightLoop(afterFlightLoopID);
  }
  if (!publisher->close()) {
    //TODO: warn_nng("publisher.close: %s", publisher->lastError().c_str());
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
  publisher->publish("receive_message", m.SerializeAsString());
}

