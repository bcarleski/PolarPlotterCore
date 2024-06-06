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

#include "polarPlotter.h"
#if defined(__SHOW_STEP_DETAILS__) || defined(__SHOW_STEP__)
#include <iostream>
#include <iomanip>
#endif

PolarPlotter::PolarPlotter(Print &printer, StatusUpdate &statusUpdater, double maxRadius, int marbleSizeInRadiusSteps, PolarMotorCoordinator* coordinator)
    : printer(ExtendedPrinter(printer)),
      statusUpdater(statusUpdater),
      maxRadius(maxRadius),
      marbleSizeInRadiusSteps(marbleSizeInRadiusSteps),
      lineStepper(LineStepper()),
      circleStepper(CircleStepper()),
      spiralStepper(SpiralStepper()),
      wipeStepper(WipeStepper(maxRadius)),
      currentStepper(NULL),
      coordinator(coordinator)
{
}

void PolarPlotter::calibrate(double startingRadius, double startingAzimuth, double radiusStepSize, double azimuthStepSize)
{
  this->radiusStepSize = radiusStepSize;
  this->azimuthStepSize = azimuthStepSize;
  this->position.repoint(startingRadius, startingAzimuth);
  this->lineStepper.calibrate(radiusStepSize, azimuthStepSize);
  this->circleStepper.calibrate(radiusStepSize, azimuthStepSize);
  this->spiralStepper.calibrate(radiusStepSize, azimuthStepSize);
  this->wipeStepper.calibrate(radiusStepSize, azimuthStepSize);
  this->statusUpdater.setRadiusStepSize(radiusStepSize);
  this->statusUpdater.setAzimuthStepSize(azimuthStepSize);
}

void PolarPlotter::startCommand(String &command)
{
  currentStepper = NULL;
  currentStep = 0;
  statusUpdater.setCurrentStep(currentStep);
 
  const String cmd = command;
  statusUpdater.setCurrentCommand(cmd);

  switch (command.charAt(0))
  {
  case 'l':
  case 'L':
    currentStepper = &lineStepper;
    break;
  case 'c':
  case 'C':
    currentStepper = &circleStepper;
    break;
  case 's':
  case 'S':
    currentStepper = &spiralStepper;
    break;
  case 'w':
  case 'W':
    currentStepper = &wipeStepper;
    break;
  }

  if (currentStepper != NULL) {
    String arguments = command.substring(1);
    currentStepper->startNewLine(position, arguments);

    const double radius = position.getRadius();
    const double azimuth = position.getAzimuth();
    statusUpdater.setPosition(radius, azimuth);
  }
}

bool PolarPlotter::hasNextStep()
{
  return pendingStep.hasStep() || (currentStepper != NULL && currentStepper->hasStep());
}

void PolarPlotter::clearStepper()
{
  currentStepper = NULL;
}

void PolarPlotter::step()
{
  if (pendingStep.hasStep()) {
    if (!coordinator->canAddSteps()) return;
    coordinator->addSteps(pendingStep.getRadiusStep(), pendingStep.getAzimuthStep(), pendingStep.isFast());
    pendingStep.setSteps(0, 0);
  }
  if (!hasNextStep()) {
    return;
  }

  Step &step = currentStepper->step();
  const bool fastStep = currentStepper->isFastStep();
  return moveTo(step.getRadiusStep(), step.getAzimuthStep(), fastStep);
}

void PolarPlotter::moveTo(const long radiusSteps, const long azimuthSteps, const bool fastStep)
{
  double oldRadius = position.getRadius();
  double oldAzimuth = position.getAzimuth();
  double newRadiusDelta = radiusSteps * radiusStepSize;
  double newAzimuthDelta = azimuthSteps * azimuthStepSize;
  double newRadius = oldRadius + newRadiusDelta;
  double newAzimuth = oldAzimuth + newAzimuthDelta;
  long radiusStep = radiusSteps;
  long azimuthStep = azimuthSteps;

  currentStep++;
  statusUpdater.setCurrentStep(currentStep);

  if (newRadius >= maxRadius) { radiusStep = round((maxRadius - oldRadius) / radiusStepSize); newRadius = maxRadius; }
  if (newRadius < (radiusStepSize * 0.5)) { radiusStep = -round(oldRadius / radiusStepSize); newRadius = 0; }
  if (abs(newAzimuth) < (azimuthStepSize * 0.5)) newAzimuth = 0;


#ifdef __SHOW_STEP_DETAILS__
    std::cout << "  Stepping.  Original: (" << radiusSteps << "," << azimuthSteps << "), Adjusted: (" << radiusStep << ", " << azimuthStep << "), Fast: " << fastStep << std::endl;
#endif
#ifdef __SHOW_STEP__
    std::cout << "STEP: " << radiusStep << "," << azimuthStep << std::endl;
#endif

  if ((radiusStep != 0 || azimuthStep != 0) && coordinator)
  {
    if (coordinator->canAddSteps()) {
      coordinator->addSteps(radiusStep, azimuthStep, fastStep);
    } else {
      pendingStep.setStepsWithSpeed(radiusStep, azimuthStep, fastStep);
    }
  }

  updatePosition(newRadius, newAzimuth, position, &statusUpdater);
}

Point PolarPlotter::getPosition() const
{
  return position;
}

String PolarPlotter::getHelpMessage()
{
  return "help          This help message\n"
         "W             Wipe any existing drawing\n"
         "L{X},{Y}      Draw a line to the cartesian point (X,Y)\n"
         "C{X},{Y},{D}  Draw a circular arc with center at the cartesian point (X,Y) having an angle of the given degress (-180 to 180)\n"
         "S{R},{D}      Draw a spiral using R units of radius change and D degrees around\n"
         "D{#}          Set the debug level between 0-9 (0-Off, 9-Most Verbose)";
}

void PolarPlotter::printStep(const long radiusStep, const long azimuthStep, const bool fastStep, StatusUpdate* statusUpdater, ExtendedPrinter printer)
{
    printer.print("STEP: ");
    printer.print(radiusStep);
    printer.print(",");
    printer.print(azimuthStep);
    printer.print(" / ");
    printer.println(fastStep ? "fast" : "slow");

    String msg = "";
    msg = msg + radiusStep + "," + azimuthStep + " / " + (fastStep ? "fast" : "slow");
    statusUpdater->status("STEP:", msg);
}

void PolarPlotter::updatePosition(const double newRadius, const double newAzimuth, Point &position, StatusUpdate* statusUpdater)
{
  position.repoint(newRadius, newAzimuth);
  statusUpdater->setPosition(newRadius, newAzimuth);
}
