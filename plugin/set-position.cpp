#include <cstdio>
#include <ctime>
#include <string>

#include <fmt/core.h>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>

#include "xplane.pb.h"

void nng_fatal(const char *func, int rv) {
  fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
  exit(1);
}


std::unique_ptr<xplane::Response>
do_rpc(nng_socket sock, const std::unique_ptr<xplane::Request> & req) {
  // Always return a Response, also for local and connection errors
  auto rep = std::make_unique<xplane::Response>();
  auto pb = req->SerializeAsString();
  int rv;

  fmt::print("Sending request\n");
  if ((rv = nng_send(sock, (void *) pb.c_str(), pb.length(), 0)) != 0) {
    rep->set_error(nng_strerror(rv));
    return rep;
  }
  fmt::print("Request sent\n");

  fmt::print("Reading response\n");
  size_t sz;
  char * buf = nullptr;
  if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
    rep->set_error(nng_strerror(rv));
    return rep;
  }
  fmt::print("Read response\n");
  rep->ParseFromArray(buf, sz);
  nng_free(buf, sz);
  return rep;
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    fmt::print("USAGE: {} <latitude> <longitude> <elevation-in-meters>\n", argv[0]);
    return 2;
  }
  double lat = std::stod(argv[1]);
  double lon = std::stod(argv[2]);
  double elev = std::stod(argv[3]);

  // Create socket and connect
  nng_socket sock;
  int rv;
  if ((rv = nng_req0_open(&sock)) != 0) {
    nng_fatal("nng_req0_open", rv);
  }
  if ((rv = nng_setopt_ms(sock, NNG_OPT_SENDTIMEO, 1000)) != 0) {
    nng_fatal("nng_setopt_ms NNG_OPT_SENDTIMEO", rv);
  }
  if ((rv = nng_setopt_ms(sock, NNG_OPT_RECVTIMEO, 1000)) != 0) {
    nng_fatal("nng_setopt_ms NNG_OPT_RECVTIMEO", rv);
  }
  constexpr auto& url = "tcp://127.0.0.1:27471";
  if ((rv = nng_dial(sock, url, nullptr, 0)) != 0) {
    nng_fatal("nng_dial", rv);
  }

  // Build request
  auto req = std::make_unique<xplane::Request>();
  req->set_command(xplane::Request_Command_SetPosition);
  auto pos = req->mutable_position();
  pos->set_lat(lat);
  pos->set_lon(lon);
  pos->set_elev(elev);
  req->PrintDebugString();

  // Do call
  auto rep = do_rpc(sock, req);
  rep->PrintDebugString();

  nng_close(sock);
  return 0;
}
