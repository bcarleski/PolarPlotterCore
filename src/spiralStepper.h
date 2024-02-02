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

#ifndef _POLARPLOTTERCORE_SPIRALSTEPPER_H_
#define _POLARPLOTTERCORE_SPIRALSTEPPER_H_

#include "baseStepper.h"

class SpiralStepper : public BaseStepper
{
private:
    float radiusStepFrequency;
    float azimuthStepFrequency;
    int radiusStepsTaken;
    int azimuthStepsTaken;
    int radiusStep;
    int azimuthStep;
    int radiusSteps;
    int azimuthSteps;
    int maxSteps;
    int currentStep;

protected:
    bool parseArgumentsAndSetFinish(Point &currentPosition, String &arguments);
    float findDistanceFromPointOnLineToFinish(Point &point);
    void setClosestPointOnLine(Point &point, Point &closestPoint);
    float determineStartingAzimuthFromCenter();
    void computeNextStep();

public:
  SpiralStepper(float radiusStepSize, float azimuthStepSize);
};

#endif
