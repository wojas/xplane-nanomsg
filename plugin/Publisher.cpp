#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>

#include <utility>

#include "Publisher.h"
#include "Utils.h"

Publisher::Publisher(std::string url, S_Statistics & stats)
    : bindURL(std::move(url)), stats(stats), lastCall("") {}

void Publisher::publish(const std::string &topic, const std::string &pb) {
  if (pb.empty()) {
    LOG("WARNING: empty protobuf");
  }
  std::string buf = topic;
  buf += '=';
  buf += pb;
  //std::cout << "@@@ " << pb.size() << " " << buf << std::endl;
  lastCall = "nng_send #" + topic;
  if ((lastErrorCode = nng_send(sock, (void *) buf.c_str(), buf.length(), NNG_FLAG_NONBLOCK)) != 0) {
    stats->setPublishError(lastErrorCode);
  }
}

bool Publisher::open() {
  // Open PUB socket
  lastCall = "nng_pub0_open";
  if ((lastErrorCode = nng_pub0_open(&sock)) != 0) {
    return false;
  }
  lastCall = "nng_setopt_int NNG_OPT_SENDBUF";
  if ((lastErrorCode = nng_setopt_int(sock, NNG_OPT_SENDBUF, 1024)) < 0) {
    return false;
  }
  lastCall = "nng_listen";
  lastErrorCode = nng_listen(sock, bindURL.c_str(), nullptr, 0);
  return lastErrorCode >= 0;
}

std::string Publisher::lastError() const {
 return lastCall + ": " + std::string(nng_strerror(lastErrorCode));
}

bool Publisher::close() {
  lastCall = "nng_close";
  return (lastErrorCode = nng_close(sock)) == 0;
}

void Publisher::publishStats() {
  publish("stats", stats->SerializeAsString());
}

void Publisher::publishInfo(std::unique_ptr<Info> & info) {
  publish("info", info->SerializeAsString());
}

Publisher::~Publisher() {
  DEBUGLOG("~Publisher");
  nng_close(sock); // ignore NNG_EBADF if already closed
}

