#include "Statistics.h"

Statistics::Statistics() {
  msg.set_msg_type(xplane::Message_Type_Stats);
  st = msg.mutable_stats();
}

string Statistics::SerializeAsString() {
  return msg.SerializeAsString();
}

void Statistics::setPublishError(int lastErrorCode) {
  st->set_last_publish_error_code(lastErrorCode);
  st->set_total_publish_errors(st->total_publish_errors() + 1);
}
