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

#include "lineStepper.h"

bool LineStepper::parseArgumentsAndSetFinish(Point &currentPosition, String &arguments) {
    int comma = arguments.indexOf(',');
    if (comma <= 0) return false;

    double finishX = arguments.substring(0, comma).toFloat();
    double finishY = arguments.substring(comma + 1).toFloat();
    deltaX = finishX - currentPosition.getX();
    deltaY = finishY - currentPosition.getY();

    finish.cartesianRepoint(finishX, finishY);
    this->orientPoint(currentPosition, finish);
    this->snapPointToClosestPossiblePosition(finish);

    return true;
}

double LineStepper::findDistanceFromPointOnLineToFinish(Point &point)
{
    return this->findDistanceBetweenPoints(point, this->finish);
}

void LineStepper::setClosestPointOnLine(Point &point, Point &closestPoint)
{
    double startToFinishSquared = deltaX * deltaX + deltaY * deltaY;
    double startToPointDotStartToFinish = (point.getX() - start.getX()) * deltaX + (point.getY() - start.getY()) * deltaY;
    double normalizedDistanceFromStart = startToPointDotStartToFinish / startToFinishSquared;

    closestPoint.cartesianRepoint(start.getX() + deltaX * normalizedDistanceFromStart, start.getY() + deltaY * normalizedDistanceFromStart);
}

double LineStepper::determineStartingAzimuthFromCenter()
{
    return finish.getAzimuth();
}
