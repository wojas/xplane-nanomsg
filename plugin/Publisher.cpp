#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>

#include <utility>

#include "Publisher.h"
#include "Utils.h"

Publisher::Publisher(string url, Statistics *stats) : bindURL(std::move(url)), stats(stats) {
  lastCall = "";
}

void Publisher::publish(const string &topic, const string &pb) {
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

string Publisher::lastError() const {
 return lastCall + ": " + string(nng_strerror(lastErrorCode));
}

bool Publisher::close() {
  lastCall = "nng_close";
  return (lastErrorCode = nng_close(sock)) == 0;
}

void Publisher::publishStats() {
  publish("stats", stats->SerializeAsString());
}

void Publisher::publishInfo(Info *info) {
  publish("info", info->SerializeAsString());
}

