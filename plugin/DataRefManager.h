
#ifndef XPLANE_NANOMSG_DATAREFMANAGER_H
#define XPLANE_NANOMSG_DATAREFMANAGER_H

#include <string>
#include <map>

#include <XPLMDataAccess.h>

#include <xplane.pb.h>

class DataRefInfo {
public:
  std::string name; // TODO: use const char*?
  XPLMDataRef ref;
  bool writable;
  XPLMDataTypeID types; // bitmap

  xplane::DataRef * asProtobufData() const;
  int getInt() const;
  float getFloat() const;
  double getDouble() const;
  std::string getString() const;
};

class DataRefManager {
protected:
  std::unordered_map<std::string, DataRefInfo*> map = {};

public:
  DataRefManager();
  DataRefInfo * getInfo(const std::string& name);
};

#endif //XPLANE_NANOMSG_DATAREFMANAGER_H
