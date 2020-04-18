#include <iostream>
#include <string>
#include <XPLMUtilities.h>

#include "Utils.h"

void logString(const std::string &s) {
  //std::cout << LOG_PREFIX << s << std::endl;
  // TODO: Check if the extra \n is needed
  std::string line = LOG_PREFIX + s + '\n';
  XPLMDebugString(line.c_str());
}

