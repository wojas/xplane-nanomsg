#ifndef XPLANE_NANOMSG_PUBLISHER_H
#define XPLANE_NANOMSG_PUBLISHER_H

#include <string>
#include <nng/nng.h>

#include "Statistics.h"
#include "Info.h"

class Publisher {
protected:
  nng_socket sock{};

public:
  std::string bindURL;
  int lastErrorCode = 0;
  std::string lastCall;
  std::shared_ptr<Statistics> stats;

  explicit Publisher(std::string url, std::shared_ptr<Statistics> & stats);
  virtual ~Publisher();

  void publish(const std::string& topic, const std::string& pb);
  void publishStats();
  void publishInfo(unique_ptr<Info> &info);
  bool open();
  bool close();
  [[nodiscard]] std::string lastError() const;

};

#endif //XPLANE_NANOMSG_PUBLISHER_H
