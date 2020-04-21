#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>

#include <utility>

#include "Commands.h"
#include "Utils.h"


Commands::Commands(std::string url, S_Statistics & stats, S_Position & position)
    : bindURL(std::move(url)),
      stats(stats),
      position(position) {
  lastCall = "";
}

void Commands::send(const std::string &pb) {
  if (pb.empty()) {
    LOG("WARNING: empty protobuf");
  }
  lastCall = "nng_send";
  // FIXME: NNG_FLAG_NONBLOCK did not work, prevent any possible hang here
  if ((lastErrorCode = nng_send(sock, (void *) pb.c_str(), pb.length(), 0)) != 0) {
    stats->setCommandError(lastErrorCode);
    DEBUGLOG("Send response error: {} {}", lastErrorCode, lastError());
  }
}

std::optional<std::string> Commands::recv() {
  lastCall = "nng_recv";
  char *buf = nullptr;
  size_t sz;
  if ((lastErrorCode = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC|NNG_FLAG_NONBLOCK)) != 0) {
    if (lastErrorCode != NNG_EAGAIN) {
      stats->setCommandError(lastErrorCode);
      DEBUGLOG("Receive request error: {} {}", lastErrorCode, lastError());
    }
    return std::nullopt;
  }
  auto pb = std::string(buf, sz);
  nng_free(buf, sz);
  return {pb};
}

bool Commands::open() {
  lastCall = "nng_rep0_open";
  if ((lastErrorCode = nng_rep0_open(&sock)) != 0) {
    return false;
  }
  lastCall = "nng_setopt_int NNG_OPT_RECVBUF";
  if ((lastErrorCode = nng_setopt_int(sock, NNG_OPT_RECVBUF, 1024)) < 0) {
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

std::string Commands::lastError() const {
  return lastCall + ": " + std::string(nng_strerror(lastErrorCode));
}

bool Commands::close() {
  lastCall = "nng_close";
  return (lastErrorCode = nng_close(sock)) == 0;
}

void Commands::handle() {
  for (;;) {
    DEBUGLOG("@@@ Checking for requests");
    auto pb = recv();
    if (!pb) {
      return; // done
    }
    DEBUGLOG("Request received");
    stats->st->set_command_receives(stats->st->command_receives()+1);

    auto req = std::make_unique<xplane::Request>();
    auto rep = std::make_unique<xplane::Response>();
    if (!req->ParseFromString(*pb)) {
      stats->st->set_command_parse_errors(stats->st->command_parse_errors()+1);
      // Send error response
      DEBUGLOG("Request protobuf parse error");
      rep->set_error("protobuf parse error");
      send(rep->SerializeAsString());
      continue;
    }
    DEBUGLOG("Request:\n{}\n", req->DebugString());
    rep->mutable_header()->CopyFrom(req->header());
    dispatch(req, rep);
    DEBUGLOG("Response:\n{}\n", rep->DebugString());
    send(rep->SerializeAsString());
  }
}

void Commands::dispatch(const std::unique_ptr<xplane::Request> & req,
                        std::unique_ptr<xplane::Response> & rep) {
  switch (req->command()) {
    case xplane::Request_Command_SetPosition:
      if (!req->has_position()) {
        rep->set_error("SetPosition requires a .position");
        return;
      }
      {
        auto p = req->position();
        if (p.alt() != 0) {
          position->setPositionFeet(p.lat(), p.lon(), p.alt());
        } else {
          position->setPositionMetric(p.lat(), p.lon(), p.elev());
        }
      }
      rep->set_success(true);
      break;

    case xplane::Request_Command_GetStats:
      {
        auto rs = rep->mutable_stats();
        rs->CopyFrom(*(stats->st));
      }
      rep->set_success(true);
      break;

    case xplane::Request_Command_UnknownCommand:
      rep->set_error("No command set in request");
      break;

    default:
      rep->set_error("Command not implemented");
  }
}

Commands::~Commands() {
  DEBUGLOG("~Commands");
  nng_close(sock); // ignore NNG_EBADF if already closed
}


