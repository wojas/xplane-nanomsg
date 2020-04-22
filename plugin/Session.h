
#ifndef XPLANE_NANOMSG_SESSION_H
#define XPLANE_NANOMSG_SESSION_H

#include <ctime>
#include <memory>
#include <string>

#include <xplane.pb.h>

// DefaultTTL is the default session ttl in seconds if none was set
static const int DEFAULT_TTL = 10;

class Session {
public:
  // The session ID should be a long unique string like a UUID
  std::string id;

  // If set, this session will be automatically removed if a new
  // session is registered with the same uniqueAppId.
  // This allows you to guarantee that only a single instance of
  // a client controls X-Plane.
  std::string uniqueAppId; // optional

  // Unix timestamp we last heard from the client.
  time_t lastSeen;

  // Session options set during creation.
  xplane::SessionOptions opt;

  // The session will be automatically removed if we do not hear from
  // the client for ttl seconds.
  // If you created visible objects in your session, this means that
  // they will disappear from the SIM after this period.
  // If the SIM paused for a long time, like for a scenery load,
  // we keep the session alive longer to not drop sessions
  // during such a load.
  int ttl; // seconds

  Session(const std::string & id, const xplane::SessionOptions & opt);

};

using S_Session = std::shared_ptr<Session>;

#endif //XPLANE_NANOMSG_SESSION_H
