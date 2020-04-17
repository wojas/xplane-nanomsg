#include <XPLMDisplay.h>
#include <XPLMPlugin.h>
#include "Info.h"

#include <ctime>
#include <cstdio>

Info::Info() {
  msg.set_msg_type(xplane::Message_Type_Info);
  info = msg.mutable_info();
  info->set_plugin_id(XPLMGetMyID());
  time_t startTime = std::time(nullptr);
  info->set_start_time_unix(startTime);
}

string Info::SerializeAsString() {
  return msg.SerializeAsString();
}

void Info::updateScreenInfo() {
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
                                    int right, int bottom, void* ctx) {
    Info *self = (Info *)(ctx);
    auto bounds = self->info->add_monitor_bounds();
    bounds->set_left(left);
    bounds->set_bottom(bottom);
    bounds->set_right(right);
    bounds->set_top(top);
    bounds->set_id(monIdx);
    std::printf("[NanoMSG] XPLMGetAllMonitorBoundsGlobal: #%i [%i %i %i %i]\n",
                monIdx, left, top, right, bottom);
  }, (void *) this);

}

