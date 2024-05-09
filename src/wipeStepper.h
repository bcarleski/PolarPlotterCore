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

#ifndef _POLARPLOTTERCORE_WIPESTEPPER_H_
#define _POLARPLOTTERCORE_WIPESTEPPER_H_

#include "abstractStepper.h"

enum WipeState
{
    WIPE_INITIALIZING,
    MOVING_TO_EDGE,
    FULL_CIRCLE,
    SPIRALING_IN,
    FINISHED
};

class WipeStepper : public AbstractStepper
{
private:
    double radiusStepSize;
    double azimuthStepSize;
    double maxRadius;
    WipeState state;

    int fullCircleAzimuthSteps;
    int stepsToEdge;
    int edgeToCenterRadiusSteps;
    int spiralInAzimuthSteps;
    double spiralInAzimuthStepsPerRadiusStep;

    int currentStepToEdge;
    int currentFullCircleStep;
    int currentSpiralInAzimuthStep;
    int currentSpiralInRadiusStep;

    Step currentStep;

    void stepToEdge();
    void fullCircle();
    void spiralIn();

public:
    WipeStepper(double maxRadius);
    void calibrate(double radiusStepSize, double azimuthStepSize);
    void startNewLine(Point &currentPosition, String &arguments);
    bool hasStep();
    Step& step();
    bool isFastStep() { return false; }
};

#endif
