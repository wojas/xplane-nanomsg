
#ifndef XPLANE_NANOMSG_COMMANDS_H
#define XPLANE_NANOMSG_COMMANDS_H


#include <nng/nng.h>
#include "Statistics.h"
#include "Position.h"
#include "SessionManager.h"

class Commands {
protected:
  nng_socket sock{};
  S_Position position;
  S_Statistics stats;
  S_SessionManager sessionManager;

  std::optional<std::string> recv();
  void send(const std::string &pb);

  // Name for the last nanomsg call for debugging
  std::string lastCall;

public:
  // nanomsg bind socket url, like "tcp://127.0.0.1:12345"
  std::string bindURL;

  // Last nanomsg error code
  int lastErrorCode = 0;

  explicit Commands(std::string url, S_Statistics &stats,
                    S_Position &position, S_SessionManager &sm);
  virtual ~Commands();

  // Actually start listening on bindURL
  bool open();

  // Stop listening and close the socket. The destructor will also
  // close the socket if needed, but explicitly calling this
  // allows you to check for errors.
  bool close();

  // Last nanomsg error as a descriptive string
  [[nodiscard]] std::string lastError() const;

  // Handle any commands received through nanomsg.
  void handle();

  // Dispatches incoming commands to their appropriate handlers and
  // updates the response message to be sent to the client.
  void dispatch(const xplane::Request * req, xplane::Response * rep);
};


#endif //XPLANE_NANOMSG_COMMANDS_H
