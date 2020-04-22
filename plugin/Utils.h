#ifndef XPLANE_NANOMSG_UTILS_H
#define XPLANE_NANOMSG_UTILS_H

#include <fmt/format.h>

// Log a debug string to X-Plane's Log.txt
void logString(const std::string& s);

// Replace a substring in given string and return a new string.
std::string replace(const std::string& in, const std::string& from, const std::string& to);

// Prefix used for log messages in Log.txt
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
