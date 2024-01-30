#ifndef _FAKE_STATUS_
#define _FAKE_STATUS_
#include "statusUpdate.h"

class StatusUpdater : public StatusUpdate {
public:
  using StatusUpdate::status;

protected:
  void writeStatus(const String &key, const String &value) {    
  }
};
#endif
