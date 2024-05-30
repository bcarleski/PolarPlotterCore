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

#ifdef __SHOW_STEP_DETAILS__
#include <iostream>
#include <iomanip>
#endif
#include "wipeStepper.h"
#define WIPE_LOOPS 15

WipeStepper::WipeStepper(double maxRadius) : currentStep(Step()), state(WIPE_INITIALIZING), maxRadius(maxRadius)
{
}

void WipeStepper::startNewLine(Point &currentPosition, String &arguments)
{
    state = MOVING_TO_EDGE;
    stepsToEdge = round((maxRadius - currentPosition.getRadius()) / radiusStepSize);
    fullCircleAzimuthSteps = round((2 * PI) / azimuthStepSize);
    spiralInAzimuthSteps = fullCircleAzimuthSteps * WIPE_LOOPS;

#ifdef __SHOW_STEP_DETAILS__
    std::cout << "  Wiping.  stepsToEdge=" << stepsToEdge << ", fullCircleAzimuthSteps=" << fullCircleAzimuthSteps << ", spiralInAzimuthSteps=" << spiralInAzimuthSteps << std::endl;
#endif
}

bool WipeStepper::hasStep()
{
    return state != WIPE_INITIALIZING && state != FINISHED;
}

Step &WipeStepper::step()
{
    currentStep.setSteps(0, 0);

    switch (state)
    {
    case MOVING_TO_EDGE:
        stepToEdge();
        break;
    case FULL_CIRCLE:
        fullCircle();
        break;
    case SPIRALING_IN:
        spiralIn();
        break;
    case WIPE_INITIALIZING:
    case FINISHED:
    default:
        break;
    }

    return currentStep;
}

void WipeStepper::calibrate(double radiusStepSize, double azimuthStepSize)
{
    this->radiusStepSize = radiusStepSize;
    this->azimuthStepSize = azimuthStepSize;
}

void WipeStepper::stepToEdge()
{
    currentStep.setSteps(stepsToEdge, 0);
    state = FULL_CIRCLE;
}

void WipeStepper::fullCircle()
{
    currentStep.setSteps(0, fullCircleAzimuthSteps);
    state = SPIRALING_IN;
}

void WipeStepper::spiralIn()
{
    currentStep.setSteps(-stepsToEdge, spiralInAzimuthSteps);
    state = FINISHED;
}
