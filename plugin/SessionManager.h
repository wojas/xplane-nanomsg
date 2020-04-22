
#ifndef XPLANE_NANOMSG_SESSIONMANAGER_H
#define XPLANE_NANOMSG_SESSIONMANAGER_H

#include <memory>
#include <string>
#include <map>

#include "Session.h"

#include <xplane.pb.h>


class SessionManager {
private:
  // Map of all active sessions by sessionId
  std::map<std::string,S_Session> sessions = {};

public:
  // Get a session. Returns a nullptr if the session does not exist.
  [[nodiscard]] S_Session get(const std::string & sessionId) const;

  // Create a new session. If the same sessionId exists, or a uniqueAppId
  // is set and already is in use by a session, the old session will be
  // removed first.
  S_Session create(const std::string & sessionId, xplane::SessionOptions & opt);

};

using S_SessionManager = std::shared_ptr<SessionManager>;

#endif //XPLANE_NANOMSG_SESSIONMANAGER_H
