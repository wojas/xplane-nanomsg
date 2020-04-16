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
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"

#include <string>
#include <cstring>
#include <cstdio>
#include <ctime>

#include <nng/nng.h>
#include <google/protobuf/util/time_util.h>

#include "xplane.pb.h"

// These globals are set during init
XPLMPluginID pluginId;
time_t startTime;
nng_socket pubSock;

xplane::Message infoMsg;

void update_screen_info() {
  // TODO: These can change during a flight
  auto info = infoMsg.info();

  // Bounds of the global X-Plane desktop
  // Note that we're not guaranteed that the main monitor's lower left is at (0, 0)...
  // We'll need to query for the global desktop bounds!
  int left, bottom, right, top;
  XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
  info.clear_screen_bounds();
  auto bounds = info.screen_bounds();
  bounds.set_left(left);
  bounds.set_bottom(bottom);
  bounds.set_right(right);
  bounds.set_top(top);

  // Information about all full screens used by X-Plane (not in window mode!)
  info.clear_monitor_bounds();
  XPLMGetAllMonitorBoundsGlobal([] (int monIdx, int left, int top,
                                    int right, int bottom, void*) {
    auto info = infoMsg.info();
    auto bounds = info.add_monitor_bounds();
    bounds->set_left(left);
    bounds->set_bottom(bottom);
    bounds->set_right(right);
    bounds->set_top(top);
    bounds->set_id(monIdx);
    std::printf("[NanoMSG] XPLMGetAllMonitorBoundsGlobal: #%i [%i %i %i %i]",
                monIdx, left, top, right, bottom);
  }, nullptr);
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

  // Fill our Info message protobuf
  infoMsg.set_msg_type(xplane::Message_Type_Info);
  infoMsg.clear_info();
  auto info = infoMsg.info();
  info.set_plugin_id(pluginId);
  info.set_start_time_unix(startTime);
  update_screen_info();

  return true;
}

PLUGIN_API void XPluginStop(void) {
}

PLUGIN_API void XPluginDisable(void) {
}

PLUGIN_API int XPluginEnable(void) {
  return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void *inParam) {
  xplane::Message m;
  m.set_msg_type(xplane::Message_Type_PluginMessage);
  auto p = m.plugin_message();
  p.set_plugin_id(inFrom);
  p.set_msg_id(inMsg);
  p.set_param((unsigned long long) inParam); // The pointer value
  // TODO: Perhaps lookup plugin name
}


#pragma clang diagnostic pop