
#ifndef XPLANE_NANOMSG_COMMANDS_H
#define XPLANE_NANOMSG_COMMANDS_H


#include <nng/nng.h>
#include "Statistics.h"
#include "Position.h"

class Commands {
protected:
  nng_socket sock{};
  Position *position;

  std::optional<std::string> recv();
  void send(const std::string &pb);

public:
  std::string bindURL;
  int lastErrorCode = 0;
  std::string lastCall;
  Statistics *stats;

  explicit Commands(std::string url, Statistics *stats, Position *position);

  bool open();
  bool close();
  [[nodiscard]] std::string lastError() const;

  // Handle any received commands
  void handle();

  void dispatch(const std::unique_ptr<xplane::Request> &req, std::unique_ptr<xplane::Response> &rep);
};


#endif //XPLANE_NANOMSG_COMMANDS_H
