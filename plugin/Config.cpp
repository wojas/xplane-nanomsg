
#include "Config.h"
#include "Utils.h"


std::string Config::rpcClientUrl() const {
  return replace(m_rpcUrl, "0.0.0.0", "127.0.0.1");
}

std::string Config::pubClientUrl() const {
  return replace(m_pubUrl, "0.0.0.0", "127.0.0.1");
}

std::string Config::rpcUrl() const {
  return m_rpcUrl;
}

std::string Config::pubUrl() const {
  return m_pubUrl;
}

[[maybe_unused]] void Config::setRpcUrl(const std::string & s) {
  m_rpcUrl = s;
}

[[maybe_unused]] void Config::setPubUrl(const std::string & s) {
  m_pubUrl = s;
}
