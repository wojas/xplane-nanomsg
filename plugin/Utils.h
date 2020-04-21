#ifndef XPLANE_NANOMSG_UTILS_H
#define XPLANE_NANOMSG_UTILS_H

#include <fmt/format.h>

void logString(const std::string& s);
std::string replace(const std::string& in, const std::string& from, const std::string& to);

#define LOG_PREFIX "[nanomsg] "

// LOG is a wrapper around fmt for logging with compile-time format string checks.
// The "##" is to allow calls with a plain string.
// Call logString(s) instead if you preformatted your string.
#define LOG(fmtStr, ...) logString(fmt::format(FMT_STRING(fmtStr), ## __VA_ARGS__))

// Same as LOG, but only in debug mode
#ifdef DEBUG
#define DEBUGLOG(fmtStr, ...) logString("DEBUG: " + fmt::format(FMT_STRING(fmtStr), ## __VA_ARGS__))
#else
#define DEBUGLOG(fmtStr, ...)
#endif

#endif //XPLANE_NANOMSG_UTILS_H
