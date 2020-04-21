#ifndef XPLANE_NANOMSG_INFO_H
#define XPLANE_NANOMSG_INFO_H

#include <string>

#include <xplane.pb.h>

class Info {
public:
  xplane::Message msg;
  xplane::Info *info;

  Info();
  std::string SerializeAsString();
  void updateScreenInfo();

};


#endif //XPLANE_NANOMSG_INFO_H
