#ifndef XPLANE_NANOMSG_STATISTICS_H
#define XPLANE_NANOMSG_STATISTICS_H

#include <string>

#include <xplane.pb.h>

class Statistics {
public:
  // Message to publish when SerializeAsString is called.
  // This message contains the statistics data.
  xplane::Message msg;

  // Mutable reference to the actual statistics data in the
  // message. You can directly manipulate the contained
  // statistics.
  xplane::Stats *st;

  Statistics();

  // Convenience method to record a Publisher nanomsg error
  void setPublishError(int lastErrorCode) const;

  // Convenience method to record a Commands nanomsg error
  void setCommandError(int lastErrorCode) const;

  // Returns a raw protobuf string with all the stats
  std::string SerializeAsString();
};

using S_Statistics = std::shared_ptr<Statistics>;

#endif //XPLANE_NANOMSG_STATISTICS_H
