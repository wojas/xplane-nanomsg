
#ifndef XPLANE_NANOMSG_CONFIG_H
#define XPLANE_NANOMSG_CONFIG_H

#include <string>

class Config {
private:
  std::string m_rpcUrl = "tcp://0.0.0.0:27471";
  std::string m_pubUrl = "tcp://0.0.0.0:27472";

public:
  [[nodiscard]] std::string rpcUrl() const;
  [[nodiscard]] std::string pubUrl() const;
  [[nodiscard]] std::string rpcClientUrl() const;
  [[nodiscard]] std::string pubClientUrl() const;

  [[maybe_unused]] void setRpcUrl(const std::string& s);
  [[maybe_unused]] void setPubUrl(const std::string& s);
};

inline Config config;

#endif //XPLANE_NANOMSG_CONFIG_H
