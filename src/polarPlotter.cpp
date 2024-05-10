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

PolarPlotter::PolarPlotter(Print &printer, StatusUpdate &statusUpdater, double maxRadius, int marbleSizeInRadiusSteps)
    : printer(ExtendedPrinter(printer)),
      statusUpdater(statusUpdater),
      maxRadius(maxRadius),
      marbleSizeInRadiusSteps(marbleSizeInRadiusSteps),
      lineStepper(LineStepper()),
      circleStepper(CircleStepper()),
      spiralStepper(SpiralStepper()),
      wipeStepper(WipeStepper(maxRadius)),
      currentStepper(NULL)
{
}

void PolarPlotter::onStep(void stepper(const int radiusSteps, const int azimuthSteps, const bool fastStep))
{
  this->stepper = stepper;
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
 
  if (debugLevel >= 1)
  {
    printer.print(" COMMAND - ");
    printer.println(command);
    statusUpdater.status("COMMAND", command);
  }
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

    String positionString = "";
    double radius = this->position.getRadius();
    double azimuth = this->position.getAzimuth();
    positionString = positionString + String(radius, 6) + "x" + String(azimuth, 6);
    const String positionArg = positionString;
    statusUpdater.setPosition(positionArg);
  }

  if (this->debugLevel >= 2)
  {
    this->printer.print(" HAS STEPPER: ");
    this->printer.println(currentStepper != NULL);
    this->printer.print(" HAS STEPS: ");
    this->printer.println(this->hasNextStep());
  }
}

bool PolarPlotter::hasNextStep()
{
  return currentStepper != NULL && currentStepper->hasStep();
}

void PolarPlotter::clearStepper()
{
  currentStepper = NULL;
}

void PolarPlotter::step()
{
  if (!this->hasNextStep()) {
    return;
  }

  currentStep++;
  statusUpdater.setCurrentStep(currentStep);

  Step &step = currentStepper->step();
  const bool fastStep = currentStepper->isFastStep();
  return this->executeStep(step.getRadiusStep(), step.getAzimuthStep(), fastStep);
}

void PolarPlotter::executeStep(const int radiusSteps, const int azimuthSteps, const bool fastStep)
{
  double oldRadius = position.getRadius();
  double oldAzimuth = position.getAzimuth();
  double newRadiusDelta = radiusSteps * this->radiusStepSize;
  double newAzimuthDelta = azimuthSteps * this->azimuthStepSize;
  double newRadius = oldRadius + newRadiusDelta;
  double newAzimuth = oldAzimuth + newAzimuthDelta;
  int radiusStep = radiusSteps;
  int azimuthStep = azimuthSteps;

  if (newRadius >= maxRadius)
  {
    radiusStep = 0;
  }
  if (newRadius <= (radiusStepSize * 0.5)) {
    newRadius = 0;
  }
  if (newAzimuth <= (azimuthStepSize * 0.5)) {
    newAzimuth = 0;
  }

  if (debugLevel >= 3)
  {
    String msg = "";
    msg = msg + radiusStep + "," + azimuthStep;
    printer.print("STEP: ");
    printer.print(radiusStep);
    printer.print(",");
    printer.println(azimuthStep);
    statusUpdater.status("STEP:", msg);
  }
  if ((radiusStep != 0 || azimuthStep != 0) && this->stepper)
  {
    this->stepper(radiusStep, azimuthStep, fastStep);
  }
  position.repoint(newRadius, newAzimuth);

  String positionString = "";
  double radius = position.getRadius();
  double azimuth = position.getAzimuth();
  positionString = positionString + radius + "x" + azimuth;
  const String positionArg = positionString;
  statusUpdater.setPosition(positionArg);
}

Point PolarPlotter::getPosition() const
{
  return this->position;
}

void PolarPlotter::setDebug(unsigned int level)
{
  this->debugLevel = level;
  if (level > 0)
  {
    this->printer.print(" Set debug level to ");
    this->printer.println(level);
  }
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
