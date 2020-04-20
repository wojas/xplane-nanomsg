#include "Statistics.h"

Statistics::Statistics() {
  msg.set_msg_type(xplane::Message_Type_Stats);
  st = msg.mutable_stats();
}

std::string Statistics::SerializeAsString() {
  return msg.SerializeAsString();
}

void Statistics::setPublishError(int lastErrorCode) const {
  st->set_last_publish_error_code(lastErrorCode);
  st->set_total_publish_errors(st->total_publish_errors() + 1);
}

void Statistics::setCommandError(int lastErrorCode) const {
  st->set_last_command_error_code(lastErrorCode);
  st->set_total_command_errors(st->total_command_errors() + 1);
}
