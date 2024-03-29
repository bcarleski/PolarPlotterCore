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

#define RADIUS_STEP_POSITIVE 0x8
#define RADIUS_STEP_NEGATIVE 0x4
#define AZIMUTH_STEP_POSITIVE 0x2
#define AZIMUTH_STEP_NEGATIVE 0x1
#include "step.h"

Step::Step()
{
  state = 0;
}

void Step::setSteps(Step &other)
{
  state = other.state;
}

void Step::setSteps(int radiusStep, int azimuthStep)
{
  unsigned char state = 0;

  if (radiusStep > 0)
  {
    state |= RADIUS_STEP_POSITIVE;
  }
  else if (radiusStep < 0)
  {
    state |= RADIUS_STEP_NEGATIVE;
  }

  if (azimuthStep > 0)
  {
    state |= AZIMUTH_STEP_POSITIVE;
  }
  else if (azimuthStep < 0)
  {
    state |= AZIMUTH_STEP_NEGATIVE;
  }

  this->state = state;
}

int Step::getRadiusStep() const
{
  if (state & RADIUS_STEP_POSITIVE)
  {
    return 1;
  }

  if (state & RADIUS_STEP_NEGATIVE)
  {
    return -1;
  }

  return 0;
}

int Step::getAzimuthStep() const
{
  if (state & AZIMUTH_STEP_POSITIVE)
  {
    return 1;
  }

  if (state & AZIMUTH_STEP_NEGATIVE)
  {
    return -1;
  }

  return 0;
}

bool Step::hasStep() const
{
  return state != 0;
}
