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

PolarPlotter::PolarPlotter(Print &printer, StatusUpdate &statusUpdater, float maxRadius, int marbleSizeInRadiusSteps)
    : printer(ExtendedPrinter(printer)),
      statusUpdater(statusUpdater),
      maxRadius(maxRadius),
      marbleSizeInRadiusSteps(marbleSizeInRadiusSteps),
      lineStepper(LineStepper()),
      circleStepper(CircleStepper()),
      spiralStepper(SpiralStepper()),
      currentStepper(NULL)
{
}

void PolarPlotter::onStep(void stepper(const int radiusSteps, const int azimuthSteps, const bool fastStep))
{
  this->stepper = stepper;
}

void PolarPlotter::calibrate(float startingRadius, float startingAzimuth, float radiusStepSize, float azimuthStepSize)
{
  this->radiusStepSize = radiusStepSize;
  this->azimuthStepSize = azimuthStepSize;
  this->position.repoint(startingRadius, startingAzimuth);
  this->lineStepper.calibrate(radiusStepSize, azimuthStepSize);
  this->circleStepper.calibrate(radiusStepSize, azimuthStepSize);
  this->spiralStepper.calibrate(radiusStepSize, azimuthStepSize);
}

void PolarPlotter::startCommand(String &command)
{
  currentStepper = NULL;
 
  if (this->debugLevel >= 1)
  {
    printer.print(" COMMAND - ");
    printer.println(command);
    statusUpdater.status("COMMAND", command);
  }

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
  }

  if (currentStepper != NULL) {
    String arguments = command.substring(1);
    currentStepper->startNewLine(position, arguments);
  }

  if (this->debugLevel >= 2)
  {
    this->printer.print(" HAS STEPPER: ");
    this->printer.println(currentStepper != NULL);
    this->printer.print(" HAS STEPS: ");
    this->printer.println(this->hasNextStep());
  }
}

void PolarPlotter::executeStepsToCenter()
{
  float radius = this->position.getRadius();
  float azimuth = this->position.getAzimuth();

  if (abs(radius - 0) < 0.0000001 && abs(azimuth - 0) < 0.0000001)
  {
    return;
  }

  int radiusOffset = radius > 0 ? -1 : 1;
  int azimuthOffset = azimuth > 0 ? -1 : 1;
  int radiusSteps = round(radius / this->radiusStepSize);
  int azimuthSteps = round(azimuth / this->azimuthStepSize);
  int max = radiusSteps > azimuthSteps ? radiusSteps : azimuthSteps;

  for (int i = 0; i < max; i++)
  {
    this->executeStep(radiusSteps > i ? radiusOffset : 0, azimuthSteps > i ? azimuthOffset : 0, true);
  }
}

void PolarPlotter::executeRadiusSteps(int radiusSteps)
{
  for (int i = 0; i < radiusSteps; i++)
  {
    this->executeStep(1, 0, true);
  }
}

void PolarPlotter::executeFullCircleSteps()
{
  int azimuthSteps = round(2 * PI / this->azimuthStepSize);

  for (int i = 0; i < azimuthSteps; i++)
  {
    this->executeStep(0, 1, true);
  }
}

void PolarPlotter::executeWipe()
{
  if (this->debugLevel >= 1)
  {
    this->printer.println(" WIPING");
    this->statusUpdater.status("WIPE");
  }

  float radiusSteps = this->maxRadius / this->radiusStepSize;
  this->executeStepsToCenter();
  this->executeRadiusSteps(ceil(radiusSteps));
  this->executeFullCircleSteps();

  int stepChunks = floor(radiusSteps / marbleSizeInRadiusSteps);
  int remainder = ceil(radiusSteps - (stepChunks * marbleSizeInRadiusSteps));

  for (int i = 0; i < stepChunks; i++)
  {
    for (int j = 0; j < marbleSizeInRadiusSteps; j++)
    {
      this->executeStep(-1, 0, true);
    }

    this->executeFullCircleSteps();
  }

  if (remainder > 0)
  {
    this->executeRadiusSteps(remainder);
  }

  this->position.repoint(0, 0);
}

bool PolarPlotter::hasNextStep()
{
  return currentStepper != NULL && currentStepper->hasStep();
}
void PolarPlotter::step()
{
  if (!this->hasNextStep()) {
    return;
  }

  Step &step = currentStepper->step();

  return this->executeStep(step.getRadiusStep(), step.getAzimuthStep(), false);
}

void PolarPlotter::executeStep(const int radiusSteps, const int azimuthSteps, const bool fastStep)
{
  Point pos = this->position;
  float oldRadius = pos.getRadius();
  float oldAzimuth = pos.getAzimuth();
  float newRadiusDelta = radiusSteps * this->radiusStepSize;
  float newAzimuthDelta = azimuthSteps * this->azimuthStepSize;
  float newRadius = oldRadius + newRadiusDelta;
  float newAzimuth = oldAzimuth + newAzimuthDelta;
  int radiusStep = radiusSteps;
  int azimuthStep = azimuthSteps;

  if (newRadius >= this->maxRadius)
  {
    radiusStep = 0;
  }

  if (this->debugLevel >= 3)
  {
    String msg = "";
    msg = msg + radiusStep + "," + azimuthStep;
    this->printer.print("STEP: ");
    this->printer.print(radiusStep);
    this->printer.print(",");
    this->printer.println(azimuthStep);
    this->statusUpdater.status("STEP:", msg);
  }
  if ((radiusStep != 0 || azimuthStep != 0) && this->stepper)
  {
    this->stepper(radiusStep, azimuthStep, fastStep);
  }
  this->position.repoint(newRadius, newAzimuth);

  if (this->debugLevel >= 4)
  {
    pos = this->position;
    this->printer.print(" Position", pos);
    this->printer.print(", from=(");
    this->printer.print(oldRadius, 4);
    this->printer.print(", ");
    this->printer.print(oldAzimuth, 4);
    this->printer.print("), delta=(");
    this->printer.print(newRadiusDelta, 4);
    this->printer.print(", ");
    this->printer.print(newAzimuthDelta, 4);
    this->printer.print("), to=(");
    this->printer.print(newRadius, 4);
    this->printer.print(", ");
    this->printer.print(newAzimuth, 4);
    this->printer.println(")");
  }
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
