#ifndef XPLANE_NANOMSG_STATISTICS_H
#define XPLANE_NANOMSG_STATISTICS_H

#include <string>

#include <xplane.pb.h>

class Statistics {
public:
  xplane::Message msg;
  xplane::Stats *st;

  Statistics();
  void setPublishError(int lastErrorCode) const;
  void setCommandError(int lastErrorCode) const;
  std::string SerializeAsString();
};

using S_Statistics = std::shared_ptr<Statistics>;

#endif //XPLANE_NANOMSG_STATISTICS_H
