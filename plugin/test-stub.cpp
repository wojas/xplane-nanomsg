#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <cstdlib>
#include <cstdio>
#include <ctime>

#include <string>
#include <sstream>

#include <XPLMDefs.h>
#include <XPLMDisplay.h>
#include <XPLMProcessing.h>
#include <XPLMPlugin.h>

#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>
#include <XPLMDataAccess.h>
#include <fmt/core.h>
#include <nng/protocol/reqrep0/req.h>

#include "xplane.pb.h"

XPLMFlightLoop_f callback;

// These are the plugin entry points we want to call
PLUGIN_API int  XPluginStart(char *outName, char *outSig, char *outDesc);
PLUGIN_API void XPluginStop(void);
PLUGIN_API void XPluginDisable(void);
PLUGIN_API int  XPluginEnable(void);
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void *inParam);

void nng_fatal(const char *func, int rv) {
  fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
  exit(1);
}

nng_socket subsock;
nng_socket sock;
int rv;

void receive() {
  for (;;) {
    char *buf = nullptr;
    size_t sz;
    if ((rv = nng_recv(subsock, &buf, &sz, NNG_FLAG_ALLOC|NNG_FLAG_NONBLOCK)) != 0) {
      if (rv == NNG_EAGAIN) {
        return; // No messages
      }
      nng_fatal("nng_recv", rv);
    }
    std::string s(buf, sz);
    size_t idx = s.find('=');
    if (idx <= 0) {
      std::printf("ERROR: Ignoring message without '=' separator: %s", s.c_str());
      continue;
    }
    std::string topic = s.substr(0, idx);
    std::string pb = s.substr(idx+1);

    xplane::Message msg;
    msg.ParseFromString(pb);
    std::string p = msg.DebugString();

    printf("RECEIVED %zuB #%s =\n%s\n", sz, topic.c_str(), p.c_str());
    nng_free(buf, sz);
  }
}

int main() {
  std::printf("Starting tests..\n");

  char name[256], sig[256], desc[256];
  XPluginStart(name, sig, desc);
  XPluginEnable();

  // Connect SUB
  if ((rv = nng_sub0_open(&subsock)) != 0) {
    nng_fatal("nng_sub0_open", rv);
  }
  // subscribe to everything (empty means all topics)
  if ((rv = nng_setopt(subsock, NNG_OPT_SUB_SUBSCRIBE, "", 0)) != 0) {
    nng_fatal("nng_setopt", rv);
  }
  constexpr auto& url = "tcp://127.0.0.1:27472";
  if ((rv = nng_dial(subsock, url, nullptr, 0)) != 0) {
    nng_fatal("nng_dial", rv);
  }
  receive(); // TODO: Will miss the first Info message

  XPluginReceiveMessage(0, XPLM_MSG_PLANE_LOADED, 0);
  XPluginReceiveMessage(0, XPLM_MSG_AIRPORT_LOADED, (void *)42);

  std::printf("Running flight loop 100 times..\n");
  for (int i = 0; i < 10; ++i) {
    callback(0.050, 0.010, i, nullptr);
    usleep(10000); // 10 msec
    receive();
  }

  // Send command
  if ((rv = nng_req0_open(&sock)) != 0) {
    nng_fatal("nng_req0_open", rv);
  }
  if ((rv = nng_setopt_ms(sock, NNG_OPT_SENDTIMEO, 1000)) != 0) {
    nng_fatal("nng_setopt NNG_OPT_SENDTIMEO", rv);
  }
  if ((rv = nng_setopt_ms(sock, NNG_OPT_RECVTIMEO, 1000)) != 0) {
    nng_fatal("nng_setopt NNG_OPT_RECVTIMEO", rv);
  }
  constexpr auto& reqUrl = "tcp://127.0.0.1:27471";
  if ((rv = nng_dial(sock, reqUrl, nullptr, 0)) != 0) {
    nng_fatal("nng_dial", rv);
  }
  auto req = std::make_unique<xplane::Request>();
  req->set_command(xplane::Request_Command_SetPosition);
  auto pos = req->mutable_position();
  pos->set_lat(1);
  pos->set_lon(2);
  pos->set_elev(3);
  auto pb = req->SerializeAsString();
  req->PrintDebugString();
  fmt::print("Sending request\n");
  if ((rv = nng_send(sock, (void *) pb.c_str(), pb.length(), 0)) != 0) {
    nng_fatal("nng_send", rv);
  }
  fmt::print("Request sent\n");

  // Process request
  callback(0.050, 0.010, 123, nullptr);
  sleep(1);
  callback(0.050, 0.010, 123, nullptr);

  // Read response
  fmt::print("Reading response\n");
  size_t sz;
  char * buf = nullptr;
  if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
    nng_fatal("nng_recv", rv);
  }
  fmt::print("Read response\n");
  auto rep = std::make_unique<xplane::Response>();
  rep->ParseFromArray(buf, sz);
  rep->PrintDebugString();

  XPluginDisable();
  XPluginStop();
  receive();
  std::printf("Done.\n");
}


PLUGIN_API void XPLMGetScreenBoundsGlobal(int *outLeft, int *outTop, int *outRight, int *outBottom) {
  // TODO: Did I get these right?
  *outLeft = 0;
  *outTop = 0;
  *outRight = 1024;
  *outBottom = 768;
}

PLUGIN_API void XPLMGetAllMonitorBoundsGlobal(
        XPLMReceiveMonitorBoundsGlobal_f inMonitorBoundsCallback,
        void *inRefcon) {
  inMonitorBoundsCallback(0, 0, 0, 1024, 768, inRefcon);
  inMonitorBoundsCallback(1, 0, 0, 1024, 768, inRefcon);
}

PLUGIN_API XPLMFlightLoopID XPLMCreateFlightLoop(XPLMCreateFlightLoop_t *inParams) {
  callback = inParams->callbackFunc;
  return (void *) callback;
}

PLUGIN_API XPLMPluginID XPLMGetMyID() {
  return 42;
}

PLUGIN_API void XPLMEnableFeature(const char *inFeature, int inEnable) {
}

PLUGIN_API void XPLMDestroyFlightLoop(XPLMFlightLoopID inFlightLoopID) {
}

PLUGIN_API void XPLMScheduleFlightLoop(
    XPLMFlightLoopID     inFlightLoopID,
    float                inInterval,
    int                  inRelativeToNow) {
}

PLUGIN_API void XPLMDebugString(const char *s) {
  std::cout << s;
}

PLUGIN_API XPLMDataRef XPLMFindDataRef(const char *inDataRefName) {
    return reinterpret_cast<XPLMDataRef>(42);
};

PLUGIN_API int XPLMCanWriteDataRef(XPLMDataRef inDataRef) {
  return true;
}

PLUGIN_API XPLMDataTypeID XPLMGetDataRefTypes(XPLMDataRef inDataRef) {
  return 63; // all of them
}

PLUGIN_API int XPLMGetDatai(XPLMDataRef inDataRef) {
  return 42;
}

PLUGIN_API float XPLMGetDataf(XPLMDataRef inDataRef) {
  return 42.0;
}

PLUGIN_API double XPLMGetDatad(XPLMDataRef inDataRef) {
  return 42.0;
}

PLUGIN_API int XPLMGetDatavf(
    XPLMDataRef          inDataRef,
    float *              outValues,    /* Can be NULL */
    int                  inOffset,
    int                  inMax) {
  if (inOffset > 0 || inMax < 1) {
    return 0;
  }
  outValues[0] = 42.0;
  return 1;
}

PLUGIN_API int XPLMGetDatavi(
    XPLMDataRef          inDataRef,
    int *                outValues,    /* Can be NULL */
    int                  inOffset,
    int                  inMax) {
  if (inOffset > 0 || inMax < 1) {
    return 0;
  }
  outValues[0] = 42;
  return 1;
}

PLUGIN_API int XPLMGetDatab(
    XPLMDataRef          inDataRef,
    void *               outValues,    /* Can be NULL */
    int                  inOffset,
    int                  inMax) {
  if (inOffset > 0 || inMax < 1) {
    return 0;
  }
  char * buf = static_cast<char *>(outValues);
  buf[0] = 42;
  return 1;
}

PLUGIN_API void XPLMSetDatai(XPLMDataRef inDataRef, int inValue) {}
PLUGIN_API void XPLMSetDataf(XPLMDataRef inDataRef, float inValue) {}
PLUGIN_API void XPLMSetDatad(XPLMDataRef inDataRef, double inValue) {}

PLUGIN_API void XPLMSetDatavf(
    XPLMDataRef          inDataRef,
    float *              inValues,
    int                  inoffset,
    int                  inCount) {
}

PLUGIN_API void XPLMWorldToLocal(
    double               inLatitude,
    double               inLongitude,
    double               inAltitude,
    double *             outX,
    double *             outY,
    double *             outZ) {
}

