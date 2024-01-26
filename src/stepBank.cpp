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

#include "stepBank.h"

StepBank::StepBank(Print &printer)
    : printer(printer)
{
}

void StepBank::addStep(int radiusStep, int azimuthStep)
{
  if (this->stepCount >= MAX_STEPS)
  {
    this->stepCount = 0;
  }

  this->steps[this->stepCount++].setSteps(radiusStep, azimuthStep);
}

Step StepBank::getStep(unsigned int stepIndex)
{
  if (this->debugLevel >= 5)
  {
    printer.print("     Getting Step ");
    printer.print(stepIndex);
    printer.print(" of ");
    printer.println(this->stepCount);
  }
  if (stepIndex < 0 || stepIndex >= this->stepCount)
  {
    return this->blankStep;
  }

  return this->steps[stepIndex];
}

unsigned int StepBank::getStepCount() const
{
  return this->stepCount;
}

void StepBank::reset()
{
  this->stepCount = 0;
}

void StepBank::setDebug(unsigned int level)
{
  this->debugLevel = level;
}
