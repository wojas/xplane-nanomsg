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
#include <memory>
#include <utility>

#include "xplane.pb.h"

#include "Config.h"
#include "Publisher.h"
#include "Statistics.h"
#include "Info.h"
#include "Utils.h"
#include "DataRefManager.h"
#include "Position.h"
#include "Commands.h"

// These globals are set during init
XPLMPluginID pluginId;
time_t startTime;
XPLMFlightLoopID afterFlightLoopID;

// Global instances are grouped together into a struct for easy resource cleanup
struct Globals {
  std::shared_ptr<Statistics> stats;
  std::shared_ptr<DataRefManager> dataRefManager;
  std::shared_ptr<Position> position;
  std::shared_ptr<SessionManager> sessionManager;
  std::unique_ptr<Info> info;
  std::unique_ptr<Publisher> publisher;
  std::unique_ptr<Commands> commands; // RPC commands, not X-Plane commands
};
// Initialized in XPluginStart and deleted in XPluginStop
std::unique_ptr<Globals> g(nullptr);


// Type: XPLMFlightLoop_f
float afterFlightLoop(float  inElapsedSinceLastCall,
                      float  inElapsedTimeSinceLastFlightLoop,
                      int    inCounter,
                      void * inRefcon) {

  auto t0 = std::chrono::high_resolution_clock::now();

  // Handle RPC commands
  g->commands->handle();

  // Publish stats
  g->stats->st->set_elapsed_since_last_call(inElapsedSinceLastCall);
  g->stats->st->set_elapsed_time_since_last_flight_loop(inElapsedTimeSinceLastFlightLoop);
  g->stats->st->set_flight_loop_counter(inCounter);
  g->publisher->publishStats();

  g->info->updateScreenInfo(); // TODO: remove
  g->publisher->publishInfo(g->info);

  xplane::Message m;
  m.set_msg_type(xplane::Message_Type_Position);
  auto p = m.mutable_position();
  g->position->update();
  g->position->toProtobufData(p);
  g->publisher->publish("position", m.SerializeAsString());

  // Store how long the handler took this time, and report the next time it is run
  auto t1 = std::chrono::high_resolution_clock::now();
  auto dt = t1 - t0;
  auto usec = std::chrono::duration_cast<std::chrono::microseconds>(dt);
  g->stats->st->set_handler_time_usec(usec.count());

  // TODO: Maybe too frequent?
  //return -1.0; // Call again next frame
  return 1.0; // Call again in one second
}

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
  std::strcpy(outName, "nanomsg");
  std::strcpy(outSig, "nl.wojas.xplane.nanomsg");
  std::strcpy(outDesc, "A nanomsg + protobuf remote interface to X-Plane");

  XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", true);
  // TODO: Perhaps enable once we support drawing windows (allows VR)
  //XPLMEnableFeature("XPLM_USE_NATIVE_WIDGET_WINDOWS", true);

  // Fill plugin globals
  pluginId = XPLMGetMyID();
  startTime = std::time(nullptr);
  LOG("XPluginStart pluginId={} rpcUrl={} pubUrl={} build={} {}",
      pluginId, config.rpcUrl(), config.pubUrl(), __DATE__, __TIME__);

  // Fill our Stats message protobuf
  auto stats = std::make_shared<Statistics>();

  // Open PUB socket
  auto publisher = std::make_unique<Publisher>(config.pubUrl(), stats);
  if (!publisher->open()) {
    LOG("FATAL: nng pub init: {}", publisher->lastError());
    return false; // Plugin init failed
  }

  // Fill our Info message protobuf
  auto info = std::make_unique<Info>();
  info->updateScreenInfo();

  // Publish Info message
  // TODO: Publish periodically
  publisher->publishInfo(info);

  auto dataRefManager = std::make_shared<DataRefManager>();
  auto position = std::make_shared<Position>(dataRefManager);

  auto sessionManager = std::make_shared<SessionManager>();
  auto commands = std::make_unique<Commands>(config.rpcUrl(), stats, position, sessionManager);
  if (!commands->open()) {
    LOG("FATAL: nng rep init: {}", commands->lastError());
    return false; // Plugin init failed
  }

  g = std::make_unique<Globals>(Globals{
      std::move(stats),
      std::move(dataRefManager),
      std::move(position),
      std::move(sessionManager),
      std::move(info),
      std::move(publisher),
      std::move(commands)
  });

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
  LOG("XPluginStop");
  if (afterFlightLoopID != nullptr) {
    XPLMDestroyFlightLoop(afterFlightLoopID);
  }
  if (!g->publisher->close()) {
    LOG("XPluginStop: close publisher: {}", g->publisher->lastError());
  }
  if (!g->commands->close()) {
    LOG("XPluginStop: close commands: {}", g->commands->lastError());
  }
  g = nullptr; // magic resource cleanup
}

PLUGIN_API void XPluginDisable(void) {
  LOG("XPluginDisable");
}

PLUGIN_API int XPluginEnable(void) {
  LOG("XPluginEnable");
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
  g->publisher->publish("receive_message", m.SerializeAsString());
}

