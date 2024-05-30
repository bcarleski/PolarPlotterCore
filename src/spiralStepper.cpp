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
#ifdef __SHOW_STEP_DETAILS__
#include <iostream>
#include <iomanip>
#endif

bool SpiralStepper::parseArgumentsAndSetFinish(Point &currentPosition, String &arguments) {
    int comma = arguments.indexOf(',');
    if (comma <= 0) return false;

    double radiusOffset = arguments.substring(0, comma).toDouble();
    double degreeOffset = arguments.substring(comma + 1).toDouble();
    double azimuthOffset = (degreeOffset / 180) * PI;

    if ((currentPosition.getRadius() + radiusOffset) < 0) {
        radiusOffset = currentPosition.getRadius() * -1;
    }

    radiusSteps = (long)round(radiusOffset / radiusStepSize);
    azimuthSteps = (long)round(azimuthOffset / azimuthStepSize);
    stepped = false;
#ifdef __SHOW_STEP_DETAILS__
    std::cout << "    Spiral Step: (" << radiusSteps << "," << azimuthSteps << ")" << std::endl;
#endif

    finish.repoint(currentPosition.getRadius() + radiusOffset, currentPosition.getAzimuth() + azimuthOffset);

    return true;
}

void SpiralStepper::computeNextStep()
{
    if (stepped) {
        nextStep.setSteps(0, 0);
        return;
    }

    nextStep.setSteps(radiusSteps, azimuthSteps);
    stepped = true;
}

double SpiralStepper::findDistanceFromPointOnLineToFinish(Point &point)
{
    // Not needed for this class
    return 0;
}

void SpiralStepper::setClosestPointOnLine(Point &point, Point &closestPoint)
{
    // Not needed for this class
}

double SpiralStepper::determineStartingAzimuthFromCenter()
{
    return start.getAzimuth();
}
