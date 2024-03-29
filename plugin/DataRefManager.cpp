#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

#include <bitset>
#include <memory>

#include "DataRefManager.h"
#include "Utils.h"

DataRefManager::DataRefManager() {
}

S_DataRefInfo DataRefManager::get(const std::string &name) {
  if (map.count(name) > 0) {
    return map[name];
  }
  XPLMDataRef ref = XPLMFindDataRef(name.c_str());
  if (ref == nullptr) {
    LOG("Invalid dataref: {}", name);
    auto dri = std::make_shared<DataRefInfo>(DataRefInfo{
        .name = name,
        .valid = false,
    });
    return dri;
  }
  XPLMDataTypeID t = XPLMGetDataRefTypes(ref);
  auto dri = std::make_shared<DataRefInfo>(DataRefInfo{
      .name = name,
      .ref = ref,
      .writable = static_cast<bool>(XPLMCanWriteDataRef(ref)),
      .types = t,
      .valid = true,
  });
  map[name] = dri;
  return dri;
}

// Based on biggest ones found in DataRefs.txt
const int maxFloats = 2920;
static float floatBuf[maxFloats];
const int maxInts = 3200;
static int intBuf[maxInts];
const int maxBytes = 65536;
static char bytesBuf[maxBytes];

std::unique_ptr<xplane::DataRef> DataRefInfo::asProtobufData() const {
  auto d = std::make_unique<xplane::DataRef>();
  XPLMDataTypeID t = this->types;
  // If a dataref supports multiple types, we return all variants
  d->set_all_types(t);
  if (t & xplmType_Int) {
    d->set_int_val(XPLMGetDatai(ref));
  }
  if (t & xplmType_Float) {
    d->set_float_val(XPLMGetDataf(ref));
  }
  if (t & xplmType_Double) {
    d->set_double_val(XPLMGetDatad(ref));
  }
  if (t & xplmType_FloatArray) {
    int n = XPLMGetDatavf(ref, floatBuf, 0, maxFloats);
    for (int i = 0; i < n; i++) {
      d->add_float_array(floatBuf[i]);
    }
  }
  if (t & xplmType_IntArray) {
    int n = XPLMGetDatavi(ref, intBuf, 0, maxInts);
    for (int i = 0; i < n; i++) {
      d->add_int_array(intBuf[i]);
    }
  }
  if (t & xplmType_Data) {
    d->set_data_val(getString());
  }
  return d;
}

int DataRefInfo::getInt() const {
  if (valid && types & xplmType_Int) {
    return XPLMGetDatai(ref);
  }
  return 0;
}

float DataRefInfo::getFloat() const {
  if (valid && types & xplmType_Float) {
    return XPLMGetDataf(ref);
  }
  return 0;
}

double DataRefInfo::getDouble() const {
  if (valid && types & xplmType_Double) {
    return XPLMGetDatad(ref);
  }
  return 0;
}

std::string DataRefInfo::getString() const {
  if (valid && types & xplmType_Data) {
    int n = XPLMGetDatab(ref, bytesBuf, 0, maxBytes);
    return std::string(bytesBuf, n);
  }
  return std::string();
}

#pragma clang diagnostic pop