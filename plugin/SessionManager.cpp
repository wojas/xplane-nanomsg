
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
    std::string oldId;
    for (auto const& [sid, ses]  : sessions) {
      if (ses->uniqueAppId == unique) {
        // TODO: end old session
        oldId = ses->id;
        break;
      }
    }
    if (!oldId.empty()) {
      remove(oldId);
    }
  }

  // End any previous session with the same sessionId
  {
    auto old = get(sessionId);
    if (old) {
      // Perhaps unsafe to announce, because the new client will already be listening...
      remove(old->id);
    }
  }

  // Create new session
  auto ses = std::make_shared<Session>(sessionId, opt);
  sessions[sessionId] = ses;
  return ses;
}

void SessionManager::remove(const std::string &sessionId) {
  sessions.erase(sessionId);
}

