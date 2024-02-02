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

#include "spiralStepper.h"

SpiralStepper::SpiralStepper(float radiusStepSize, float azimuthStepSize)
    : BaseStepper(radiusStepSize, azimuthStepSize)
{
}

bool SpiralStepper::parseArgumentsAndSetFinish(Point &currentPosition, String &arguments) {
    int comma = arguments.indexOf(',');
    if (comma <= 0) return false;

    float radiusOffset = arguments.substring(0, comma).toFloat();
    float degreeOffset = arguments.substring(comma + 1).toFloat();
    float azimuthOffset = (degreeOffset / 180) * PI;

    if ((currentPosition.getRadius() + radiusOffset) < 0) {
        radiusOffset = currentPosition.getRadius() * -1;
    }

    radiusStep = radiusOffset >= 0 ? 1 : -1;
    azimuthStep = azimuthOffset >= 0 ? 1 : -1;

    radiusSteps = abs(round(radiusOffset / radiusStepSize));
    azimuthSteps = abs(round(azimuthOffset / azimuthStepSize));
    maxSteps = radiusSteps > azimuthSteps ? radiusSteps : azimuthSteps;

    float totalSteps = maxSteps;
    radiusStepFrequency = radiusSteps / totalSteps;
    azimuthStepFrequency = azimuthSteps / totalSteps;
    radiusStepsTaken = 0;
    azimuthStepsTaken = 0;
    currentStep = 0;

    finish.repoint(currentPosition.getRadius() + radiusOffset, currentPosition.getAzimuth() + azimuthOffset);

    return true;
}

void SpiralStepper::computeNextStep()
{
    if (currentStep >= maxSteps) {
        nextStep.setSteps(0, 0);
        return;
    }

    currentStep++;
    int expectedRadiusSteps = radiusStepsTaken < radiusSteps ? round(currentStep * radiusStepFrequency) : radiusStepsTaken;
    int expectedAzimuthSteps = azimuthStepsTaken < azimuthSteps ? round(currentStep * azimuthStepFrequency) : azimuthStepsTaken;

    int rStep = 0, aStep = 0;
    if (expectedRadiusSteps > radiusStepsTaken) {
        rStep = radiusStep;
        radiusStepsTaken = expectedRadiusSteps;
    }
    if (expectedAzimuthSteps > azimuthStepsTaken) {
        aStep = azimuthStep;
        azimuthStepsTaken = expectedAzimuthSteps;
    }

    nextStep.setSteps(rStep, aStep);
}

float SpiralStepper::findDistanceFromPointOnLineToFinish(Point &point)
{
    // Not needed for this class
    return 0;
}

void SpiralStepper::setClosestPointOnLine(Point &point, Point &closestPoint)
{
    // Not needed for this class
}

float SpiralStepper::determineStartingAzimuthFromCenter()
{
    return start.getAzimuth();
}
