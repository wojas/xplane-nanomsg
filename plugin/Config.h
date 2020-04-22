
#ifndef XPLANE_NANOMSG_CONFIG_H
#define XPLANE_NANOMSG_CONFIG_H

#include <string>

class Config {
private:
  // Bind url for the nanomsg req-rep remote procedure calls.
  // This is where you will send commands.
  std::string m_rpcUrl = "tcp://0.0.0.0:27471";
  // Bind url for the nanomsg pub-sub announcements.
  // This is where the location and other info is broadcast, and
  // where you will listen to updates to your subscriptions.
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
