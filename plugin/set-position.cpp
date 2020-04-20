#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <cstdio>
#include <ctime>

#include <string>

#include <nng/nng.h>
#include <fmt/core.h>
#include <nng/protocol/reqrep0/req.h>

#include "xplane.pb.h"

void nng_fatal(const char *func, int rv) {
  fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
  exit(1);
}

nng_socket sock;
int rv;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    fmt::print("USAGE: {} <latitude> <longitude> <elevation-in-meters>\n", argv[0]);
    return 2;
  }
  double lat = std::stod(argv[1]);
  double lon = std::stod(argv[2]);
  double elev = std::stod(argv[3]);

  if ((rv = nng_req0_open(&sock)) != 0) {
    nng_fatal("nng_req0_open", rv);
  }
  constexpr auto& url = "tcp://127.0.0.1:27471";
  if ((rv = nng_dial(sock, url, nullptr, 0)) != 0) {
    nng_fatal("nng_dial", rv);
  }

  auto req = std::make_unique<xplane::Request>();
  req->set_command(xplane::Request_Command_SetPosition);
  auto pos = req->mutable_position();
  pos->set_lat(lat);
  pos->set_lon(lon);
  pos->set_elev(elev);
  auto pb = req->SerializeAsString();
  req->PrintDebugString();

  fmt::print("Sending request\n");
  if ((rv = nng_send(sock, (void *) pb.c_str(), pb.length(), 0)) != 0) {
    nng_fatal("nng_send", rv);
  }
  fmt::print("Request sent\n");

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

  nng_free(buf, sz);
  nng_close(sock);
  return 0;
}
