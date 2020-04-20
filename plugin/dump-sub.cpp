#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <cstdio>
#include <ctime>

#include <string>

#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>

#include "xplane.pb.h"

void nng_fatal(const char *func, int rv) {
  fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
  exit(1);
}

nng_socket sock;
int rv;

void receive() {
  for (;;) {
    char *buf = nullptr;
    size_t sz;
    if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
      nng_fatal("nng_recv", rv);
      return;
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
  // Connect SUB
  if ((rv = nng_sub0_open(&sock)) != 0) {
    nng_fatal("nng_sub0_open", rv);
  }
  // subscribe to everything (empty means all topics)
  if ((rv = nng_setopt(sock, NNG_OPT_SUB_SUBSCRIBE, "", 0)) != 0) {
    nng_fatal("nng_setopt", rv);
  }
  constexpr auto& url = "tcp://127.0.0.1:27472";
  if ((rv = nng_dial(sock, url, nullptr, 0)) != 0) {
    nng_fatal("nng_dial", rv);
  }
  receive();
}
