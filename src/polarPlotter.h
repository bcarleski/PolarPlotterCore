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

#ifndef _POLARPLOTTERCORE_POLARPLOTTER_H_
#define _POLARPLOTTERCORE_POLARPLOTTER_H_

#include "lineStepper.h"
#include "circleStepper.h"
#include "spiralStepper.h"
#include "extendedPrinter.h"
#include "statusUpdate.h"

class PolarPlotter
{
private:
  ExtendedPrinter printer;
  StatusUpdate &statusUpdater;
  LineStepper lineStepper;
  CircleStepper circleStepper;
  SpiralStepper spiralStepper;
  BaseStepper *currentStepper;
  Step emptyStep;

  void (*stepper)(const int radiusSteps, const int azimuthSteps, const bool fastStep);
  unsigned int debugLevel;
  Point position;
  float maxRadius;
  float radiusStepSize;
  float azimuthStepSize;
  int marbleSizeInRadiusSteps;

  void executeStepsToCenter();
  void executeRadiusSteps(int radiusSteps);
  void executeFullCircleSteps();
  void executeStep(const int radiusSteps, const int azimuthSteps, const bool fastStep);

public:
  PolarPlotter(Print &printer, StatusUpdate &statusUpdater, float maxRadius, int marbleSizeInRadiusSteps);
  void onStep(void stepper(const int radiusSteps, const int azimuthSteps, const bool fastStep));
  void init(float initialRadius, float initialAzimuth, float radiusStepSize, float azimuthStepSize);
  void startCommand(String &command);
  void executeWipe();
  bool hasNextStep();
  void step();
  Point getPosition() const;
  void setDebug(unsigned int level);
  static String getHelpMessage();
};

#endif
