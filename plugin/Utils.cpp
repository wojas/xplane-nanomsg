#include <string>
#include <XPLMUtilities.h>

#include "Utils.h"

void logString(const std::string &s) {
  //std::cout << LOG_PREFIX << s << std::endl;
  // TODO: Check if the extra \n is needed
  std::string line = LOG_PREFIX + s + '\n';
  XPLMDebugString(line.c_str());
}

// Based on https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
std::string replace(const std::string& in, const std::string& from, const std::string& to) {
  std::string str = in;
  size_t start_pos = str.find(from);
  if(start_pos != std::string::npos) {
    str.replace(start_pos, from.length(), to);
  }
  return str;
}

