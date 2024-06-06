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

#include "step.h"

Step::Step()
{
  radiusSteps = 0;
  azimuthSteps = 0;
}

void Step::setSteps(Step &other)
{
  radiusSteps = other.radiusSteps;
  azimuthSteps = other.azimuthSteps;
  fast = other.fast;
}

void Step::setSteps(long radiusStep, long azimuthStep)
{
  this->radiusSteps = radiusStep;
  this->azimuthSteps = azimuthStep;
  this->fast = false;
}

void Step::setStepsWithSpeed(long radiusStep, long azimuthStep, bool fast)
{
  this->radiusSteps = radiusStep;
  this->azimuthSteps = azimuthStep;
  this->fast = fast;
}

long Step::getRadiusStep() const
{
  return radiusSteps;
}

long Step::getAzimuthStep() const
{
  return azimuthSteps;
}

bool Step::isFast() const
{
  return fast;
}

bool Step::hasStep() const
{
  return radiusSteps != 0 || azimuthSteps != 0;
}
