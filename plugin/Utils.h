#ifndef XPLANE_NANOMSG_UTILS_H
#define XPLANE_NANOMSG_UTILS_H

#include <fmt/format.h>

void logString(const std::string& s);

#define LOG_PREFIX "[nanomsg] "

// LOG is a wrapper around fmt for logging with compile-time format string checks.
// The "##" is to allow calls with a plain string.
// Call logString(s) instead if you preformatted your string.
#define LOG(fmtStr, ...) logString(fmt::format(FMT_STRING(fmtStr), ## __VA_ARGS__))

#endif //XPLANE_NANOMSG_UTILS_H
