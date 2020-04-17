#ifndef XPLANE_NANOMSG_STATISTICS_H
#define XPLANE_NANOMSG_STATISTICS_H

#include <string>

using namespace std;

#include <xplane.pb.h>

class Statistics {
public:
  xplane::Message msg;
  xplane::Stats *st;

  Statistics();
  void setPublishError(int lastErrorCode);
  string SerializeAsString();
};


#endif //XPLANE_NANOMSG_STATISTICS_H
