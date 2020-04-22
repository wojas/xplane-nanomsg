#ifndef XPLANE_NANOMSG_PUBLISHER_H
#define XPLANE_NANOMSG_PUBLISHER_H

#include <string>
#include <nng/nng.h>

#include "Statistics.h"
#include "Info.h"

class Publisher {
protected:
  nng_socket sock{};

  // This class updates the global statistics
  S_Statistics stats;

  // Name for the last nanomsg call for debugging
  std::string lastCall;

public:
  // nanomsg bind socket url, like "tcp://127.0.0.1:12345"
  std::string bindURL;

  // Last nanomsg error code
  int lastErrorCode = 0;

  explicit Publisher(std::string url, S_Statistics & stats);
  virtual ~Publisher();

  // Publish some protobuf data on given topic
  void publish(const std::string& topic, const std::string& pb);

  // Publish the current statistics
  void publishStats();

  // Publish global info
  void publishInfo(std::unique_ptr<Info> &info);

  // Actually start listening on bindURL
  bool open();

  // Stop listening and close the socket. The destructor will also
  // close the socket if needed, but explicitly calling this
  // allows you to check for errors.
  bool close();

  // Last nanomsg error as a descriptive string
  [[nodiscard]] std::string lastError() const;

};

#endif //XPLANE_NANOMSG_PUBLISHER_H
