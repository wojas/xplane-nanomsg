
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

  [[nodiscard]] xplane::DataRef * asProtobufData() const;
  [[nodiscard]] int getInt() const;
  [[nodiscard]] float getFloat() const;
  [[nodiscard]] double getDouble() const;
  [[nodiscard]] std::string getString() const;
};

using S_DataRefInfo = std::shared_ptr<DataRefInfo>;

class DataRefManager {
protected:
  std::unordered_map<std::string, S_DataRefInfo> map = {};

public:
  DataRefManager();
  S_DataRefInfo get(const std::string& name);
};

#endif //XPLANE_NANOMSG_DATAREFMANAGER_H
