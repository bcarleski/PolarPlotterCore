/*
    This file is part of the PolarPlotterCore library.
    Copyright (c) 2024 Benjamin Carleski

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef _POLARPLOTTERCORE_PLOTTERCONTROLLER_H_
#define _POLARPLOTTERCORE_PLOTTERCONTROLLER_H_

#define MAX_COMMAND_COUNT 1024
#include "polarPlotter.h"

#define TOPIC_SUBSCRIPTION_COUNT 3

enum PlotterState {
  INITIALIZING,
  DRAWING,
  RETRIEVING,
  PAUSED,
  MANUAL_RADIUS,
  MANUAL_AZIMUTH,
  CALIBRATING_ORIGIN,
  CALIBRATING_RADIUS,
  CALIBRATING_AZIMUTH,
  FINISHING_CALIBRATION,
  FINISHING_MOVING
};

class PlotterController
{
private:
  Print &printer;
  StatusUpdate &statusUpdater;
  PolarMotorCoordinator* coordinator;
  PolarPlotter plotter;
  void (*recalibrater)(const int maxRadiusSteps, const int fullCircleAzimuthSteps);
  String drawing;
  String commands[MAX_COMMAND_COUNT];
  int commandCount;
  int commandIndex;
  int calibrationRadiusSteps;
  int calibrationAzimuthSteps;
  double radiusStepSize;
  bool isCalibrated;
  PlotterState state;
  PlotterState lastState;
  char* lastTextState;

  bool needsCommands();
  bool isCalibrating();
  bool isManual();
  void manualStep(const long radiusSteps, const long azimuthSteps, const bool printStep);
  void executeCommand(String& command);
  void handleControlCommand(String& command);
  void handleCalibrationCommand(String& command);
  void handleManualCommand(String& command);

public:
  PlotterController(Print &printer, StatusUpdate &statusUpdater, double maxRadius, int marbleSizeInRadiusSteps, PolarMotorCoordinator* coordinator);
  void calibrate(double radiusStepSize, double azimuthStepSize);
  void onRecalibrate(void recalibrater(const int maxRadiusSteps, const int fullCircleAzimuthSteps));
  void performCycle();
  bool canCycle();
  void newDrawing(String &drawing);
  void addCommand(String &command);
  bool isPaused() const { return state == PAUSED; }
};

#endif
