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

PolarPlotter::PolarPlotter(Print &printer, StatusUpdate &statusUpdater, float maxRadius, float radiusStepSize, float azimuthStepSize, int marbleSizeInRadiusSteps)
    : printer(ExtendedPrinter(printer)),
      statusUpdater(statusUpdater),
      maxRadius(maxRadius),
      radiusStepSize(radiusStepSize),
      azimuthStepSize(azimuthStepSize),
      marbleSizeInRadiusSteps(marbleSizeInRadiusSteps),
      steps(StepBank(printer)),
      lineStepCalculator(LineStepCalculator(printer, radiusStepSize, azimuthStepSize))
{
}

void PolarPlotter::onStep(void stepper(const int radiusSteps, const int azimuthSteps, const bool fastStep))
{
  this->stepper = stepper;
}

void PolarPlotter::init(float startingRadius, float startingAzimuth)
{
  this->finish.repoint(startingRadius, startingAzimuth);
  this->position.cloneFrom(this->finish);
}

void PolarPlotter::computeSteps(String &command)
{
  unsigned long startMillis = millis();
  this->stepIndex = 0;
  this->steps.reset();

  if (this->debugLevel >= 1)
  {
    this->printer.print(" COMMAND - ");
    this->printer.println(command);
    this->statusUpdater.status("COMMAND", command);
  }

  switch (command.charAt(0))
  {
  case 'l':
  case 'L':
    setFinishPoint(command);
    this->lineStepCalculator.addLineSteps(this->position, this->finish, this->steps);
    break;
  }

  if (this->debugLevel >= 2)
  {
    unsigned long endMillis = millis();
    long duration = endMillis - startMillis;
    this->statusUpdater.status("COMPUTE DURATION", duration);
    this->statusUpdater.status("COMPUTE STEPS", this->getStepCount());

    this->printer.print(" STEPS - ");
    this->printer.println(this->getStepCount());
    this->printer.print(" DURATION - ");
    this->printer.println(duration);
  }
}

void PolarPlotter::setFinishPoint(String &command)
{
  int commaIndex = command.indexOf(',');
  float rawX = command.substring(1, commaIndex).toFloat();
  float rawY = command.substring(commaIndex + 1).toFloat();
  float rawR = sqrt(rawX * rawX + rawY * rawY);
  float rawA = atan2(rawY, rawX);
  float radius = round(rawR / this->radiusStepSize) * this->radiusStepSize;
  float azimuth = round(rawA / this->azimuthStepSize) * this->azimuthStepSize;

  this->finish.repoint(radius, azimuth);
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
    this->steps.addStep(radiusSteps > i ? radiusOffset : 0, azimuthSteps > i ? azimuthOffset : 0);
  }

  while (this->executeStep(true))
    ;

  this->stepIndex = 0;
  this->steps.reset();
}

void PolarPlotter::executeRadiusSteps(int radiusSteps)
{
  for (int i = 0; i < radiusSteps; i++)
  {
    this->steps.addStep(1, 0);
  }

  while (this->executeStep(true))
    ;

  this->stepIndex = 0;
  this->steps.reset();
}

void PolarPlotter::executeFullCircleSteps()
{
  int azimuthSteps = round(2 * PI / this->azimuthStepSize);

  for (int i = 0; i < azimuthSteps; i++)
  {
    this->steps.addStep(0, 1);
  }

  while (this->executeStep(true))
    ;

  this->stepIndex = 0;
  this->steps.reset();
}

void PolarPlotter::executeWipe()
{
  this->stepIndex = 0;
  this->steps.reset();

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
      this->steps.addStep(-1, 0);
    }

    this->executeFullCircleSteps();
  }

  if (remainder > 0)
  {
    this->executeRadiusSteps(remainder);
  }

  this->position.repoint(0, 0);
  this->finish.repoint(0, 0);
}

bool PolarPlotter::step()
{
  return this->executeStep(false);
}

bool PolarPlotter::executeStep(const bool fastStep)
{
  if (this->stepIndex < 0 || this->stepIndex >= this->steps.getStepCount())
  {
    if (this->stepStartMillis > 0)
    {
      if (this->debugLevel >= 2)
      {
        unsigned long endMillis = millis();
        long duration = endMillis - this->stepStartMillis;

        this->statusUpdater.status("STEP DURATION", duration);
        this->printer.println(String(" STEPPING COMPLETE, Step=") + this->getStepCount() + ", Duration=" + duration);
      }

      this->stepStartMillis = 0;
      this->position.cloneFrom(this->finish);
    }

    return false;
  }

  if (this->stepStartMillis == 0)
  {
    this->stepStartMillis = millis();
  }

  Step step = steps.getStep(this->stepIndex);
  Point pos = this->position;
  float oldRadius = pos.getRadius();
  float oldAzimuth = pos.getAzimuth();
  float newRadiusDelta = step.getRadiusStep() * this->radiusStepSize;
  float newAzimuthDelta = step.getAzimuthStep() * this->azimuthStepSize;
  float newRadius = oldRadius + newRadiusDelta;
  float newAzimuth = oldAzimuth + newAzimuthDelta;
  int radiusStep = step.getRadiusStep();
  int azimuthStep = step.getAzimuthStep();

  if (newRadius >= this->maxRadius)
  {
    radiusStep = 0;
  }

  if (this->debugLevel >= 3)
  {
    String head = "STEP ";
    head = head + this->stepIndex + ":";
    String msg = "";
    msg = msg + radiusStep + "," + azimuthStep;
    this->printer.print("STEP: ");
    this->printer.print(radiusStep);
    this->printer.print(",");
    this->printer.println(azimuthStep);
    this->statusUpdater.status(head, msg);
  }
  if ((radiusStep != 0 || azimuthStep != 0) && this->stepper)
  {
    this->stepper(radiusStep, azimuthStep, fastStep);
  }
  this->position.repoint(newRadius, newAzimuth);
  this->stepIndex++;

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

  return true;
}

unsigned int PolarPlotter::getStepCount() const
{
  return this->steps.getStepCount();
}

Point PolarPlotter::getPosition() const
{
  return this->position;
}

void PolarPlotter::setDebug(unsigned int level)
{
  this->debugLevel = level;
  this->lineStepCalculator.setDebug(level);
  this->steps.setDebug(level);
  if (level > 0)
  {
    this->printer.print(" Set debug level to ");
    this->printer.println(level);
  }
}

String PolarPlotter::getHelpMessage()
{
  return "help       This help message\n"
         "W          Wipe any existing drawing\n"
         "L{X},{Y}   Draw a line from the origin to the cartesian point (X,Y)\n"
         "D{#}       Set the debug level between 0-9 (0-Off, 9-Most Verbose)\n"
         "RA{JSON}   Simulate having received the given JSON message on the get/accepted topic\n"
         "RR{JSON}   Simulate having received the given JSON message on the get/rejected topic\n"
         "RT{JSON}   Simulate having received the given JSON message on the toDevice topic";
}
