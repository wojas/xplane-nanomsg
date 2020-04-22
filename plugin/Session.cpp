
#include "Session.h"

Session::Session(const std::string & id, const xplane::SessionOptions & opt) : id(id), opt(opt) {
  lastSeen = std::time(nullptr);
  ttl = opt.ttl_seconds();
  if (ttl == 0) {
    ttl = DEFAULT_TTL;
  }
  uniqueAppId = opt.unique_app_id();
}

