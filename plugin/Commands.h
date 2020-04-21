
#ifndef XPLANE_NANOMSG_COMMANDS_H
#define XPLANE_NANOMSG_COMMANDS_H


#include <nng/nng.h>
#include "Statistics.h"
#include "Position.h"

class Commands {
protected:
  nng_socket sock{};
  std::shared_ptr<Position> position;
  std::shared_ptr<Statistics> stats;

  std::optional<std::string> recv();
  void send(const std::string &pb);

public:
  std::string bindURL;
  int lastErrorCode = 0;
  std::string lastCall;

  explicit Commands(std::string url,
                    std::shared_ptr<Statistics> & stats,
                    std::shared_ptr<Position> & position);
  virtual ~Commands();

  bool open();
  bool close(); // FIXME: call in destructor
  [[nodiscard]] std::string lastError() const;

  // Handle any received commands
  void handle();

  void dispatch(const std::unique_ptr<xplane::Request> &req,
                std::unique_ptr<xplane::Response> &rep);
};


#endif //XPLANE_NANOMSG_COMMANDS_H
