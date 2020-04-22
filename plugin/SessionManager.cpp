
#include "SessionManager.h"

S_Session SessionManager::get(const std::string & sessionId) const {
  try {
    return sessions.at(sessionId);
  } catch (const std::out_of_range& oor) {
    return nullptr;
  }
}

S_Session SessionManager::create(const std::string & sessionId, xplane::SessionOptions & opt) {
  // End any previous session with the same unique_app_id
  auto unique = opt.unique_app_id();
  if (!unique.empty()) {
    for (auto const& [sid, ses]  : sessions) {
      if (ses->uniqueAppId == unique) {
        // TODO: end old session
        sessions.erase(ses->id); // Should be safe, because of break
        break; // Do not remove
      }
    }
  }

  // End any previous session with the same sessionId
  {
    auto old = get(sessionId);
    if (old) {
      // TODO: end old session
      // Perhaps unsafe to announce, because the new client will already be listening...
      sessions.erase(old->id);
    }
  }

  // Create new session
  auto ses = std::make_shared<Session>(sessionId, opt);
  sessions[sessionId] = ses;
  return ses;
}

