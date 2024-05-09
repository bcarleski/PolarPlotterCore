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

#include "wipeStepper.h"
#define WIPE_LOOPS 15

WipeStepper::WipeStepper(float maxRadius) : currentStep(Step()), state(WIPE_INITIALIZING), maxRadius(maxRadius)
{
}

void WipeStepper::startNewLine(Point &currentPosition, String &arguments)
{
    state = MOVING_TO_EDGE;
    stepsToEdge = round((maxRadius - currentPosition.getRadius()) / radiusStepSize);
    fullCircleAzimuthSteps = round((2 * PI) / azimuthStepSize);
    edgeToCenterRadiusSteps = round(maxRadius / radiusStepSize);
    spiralInAzimuthSteps = fullCircleAzimuthSteps * WIPE_LOOPS;
    float spiralInRadiusSteps = round(maxRadius / radiusStepSize);
    spiralInAzimuthStepsPerRadiusStep = spiralInAzimuthSteps / spiralInRadiusSteps;

    currentFullCircleStep = 0;
    currentSpiralInAzimuthStep = 0;
    currentSpiralInRadiusStep = 0;
    currentStepToEdge = 0;
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
    }

    return currentStep;
}

void WipeStepper::calibrate(float radiusStepSize, float azimuthStepSize)
{
    this->radiusStepSize = radiusStepSize;
    this->azimuthStepSize = azimuthStepSize;
}

void WipeStepper::stepToEdge()
{
    if (currentStepToEdge >= stepsToEdge)
    {
        state = FULL_CIRCLE;
        return;
    }

    currentStep.setSteps(1, 0);
    currentStepToEdge++;
}

void WipeStepper::fullCircle()
{
    if (currentFullCircleStep >= fullCircleAzimuthSteps)
    {
        state = SPIRALING_IN;
        return;
    }

    currentStep.setSteps(0, 1);
    currentFullCircleStep++;
}

void WipeStepper::spiralIn()
{
    if (currentSpiralInAzimuthStep >= spiralInAzimuthSteps)
    {
        if (currentSpiralInRadiusStep < edgeToCenterRadiusSteps)
        {
            currentStep.setSteps(1, 0);
            currentSpiralInRadiusStep++;
            return;
        }

        state = FINISHED;
        return;
    }

    int spiralInRadiusStep = round(currentSpiralInAzimuthStep * spiralInAzimuthStepsPerRadiusStep);
    if (spiralInRadiusStep > currentSpiralInRadiusStep)
    {
        currentStep.setSteps(1, 1);
        currentSpiralInRadiusStep++;
    } else {
        currentStep.setSteps(0, 1);
    }
    currentSpiralInAzimuthStep++;
}
