#ifndef XPLANE_NANOMSG_INFO_H
#define XPLANE_NANOMSG_INFO_H

#include <string>

using namespace std;

#include <xplane.pb.h>

class Info {
public:
  xplane::Message msg;
  xplane::Info *info;

  Info();
  string SerializeAsString();
  void updateScreenInfo();

};


#endif //XPLANE_NANOMSG_INFO_H
