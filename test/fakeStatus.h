#ifndef _FAKE_STATUS_
#define _FAKE_STATUS_
#include "statusUpdate.h"
#include <iomanip>
#include <iostream>

class StatusUpdater : public StatusUpdate {
public:
  using StatusUpdate::status;
  void setRadiusStepSize(const double value) {}
  void setAzimuthStepSize(const double value) {}
  void setCurrentDrawing(const String &value) {}
  void setCurrentCommand(const String &value) {}
  void setCurrentStep(const int value) {}
  void setPosition(const double radius, const double azimuth) {
    std::cout << std::setprecision(8) << "POSITION: " << radius << "," << azimuth << std::endl;
  }
  void setState(const String &value) {}

protected:
  void writeStatus(const String &key, const String &value) {    
  }
};
#endif
