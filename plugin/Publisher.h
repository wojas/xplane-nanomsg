#ifndef XPLANE_NANOMSG_PUBLISHER_H
#define XPLANE_NANOMSG_PUBLISHER_H

#include <string>
#include <nng/nng.h>

#include "Statistics.h"

using namespace std;

class Publisher {
protected:
  nng_socket sock{};

public:
  string bindURL;
  int lastErrorCode = 0;
  string lastCall;
  Statistics *stats;

  explicit Publisher(string url, Statistics *stats);

  void publish(const std::string& topic, const std::string& pb);
  bool open();
  bool close();
  string lastError() const;
};

#endif //XPLANE_NANOMSG_PUBLISHER_H
