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

#include "circleStepper.h"

bool CircleStepper::parseArgumentsAndSetFinish(Point &currentPosition, String &arguments) {
    int firstComma = arguments.indexOf(',');
    int secondComma = arguments.indexOf(',', firstComma + 1);
    if (secondComma <= 2) return false;

    double centerX = arguments.substring(0, firstComma).toFloat();
    double centerY = arguments.substring(firstComma + 1, secondComma).toFloat();
    double thetaDegrees = arguments.substring(secondComma + 1).toFloat();
    double theta = thetaDegrees / 180 * PI;
    center.cartesianRepoint(centerX, centerY);
    this->orientPoint(currentPosition, center);

    radius = this->findDistanceBetweenPoints(currentPosition, center);

    if (radius < (radiusStepSize * 2) || radius < (azimuthStepSize * 2) || abs(theta) < (azimuthStepSize * 0.1)) return false;

    if (abs(theta) > PI) {
        theta = PI * (theta > 0 ? 1 : -1);
    }

    double cosTheta = cos(theta);
    double sinTheta = sin(theta);
    double cosHalfTheta = cos(theta * 0.5);
    double sinHalfTheta = sin(theta * 0.5);
    double curX = currentPosition.getX();
    double curY = currentPosition.getY();
    double cenX = center.getX();
    double cenY = center.getY();
    double centeredX = curX - cenX;
    double centeredY = curY - cenY;
    double middleX = centeredX * cosHalfTheta + centeredY * sinHalfTheta + cenX;
    double middleY = centeredX * sinHalfTheta + centeredY * cosHalfTheta + cenY;
    double finishX = centeredX * cosTheta + centeredY * sinTheta + cenX;
    double finishY = centeredX * sinTheta + centeredY * cosTheta + cenY;
    slope = (finishY - curY) / (finishX - curX);
    intercept = finishY - slope * finishX;
    double midY = slope * middleX + intercept;
    middleAboveLine = middleY > midY;

    finish.cartesianRepoint(finishX, finishY);
    this->orientPoint(currentPosition, finish);
    this->snapPointToClosestPossiblePosition(finish);
    slope = (finish.getY() - curY) / (finish.getX() - curX);
    intercept = finish.getY() - slope * finish.getX();

    return true;
}

double CircleStepper::findDistanceFromPointOnLineToFinish(Point &point)
{
    bool aboveLine = point.getY() > (slope * point.getX() + intercept);
    double distanceToFinish = this->findDistanceBetweenPoints(point, finish);
    double distanceOnLine = distanceToFinish;
    
    if (aboveLine != middleAboveLine) {
        double distanceToStart = this->findDistanceBetweenPoints(point, start);
        distanceOnLine += distanceToStart < distanceToFinish ? distanceToStart : distanceToFinish;
    }

    return distanceOnLine;
}

void CircleStepper::setClosestPointOnLine(Point &point, Point &closestPoint)
{
    double deltaX = point.getX() - center.getX();
    double deltaY = point.getY() - center.getY();
    double magnitude = sqrt(deltaX * deltaX + deltaY * deltaY);
    double closestX = center.getX() + deltaX / magnitude * radius;
    double closestY = center.getY() + deltaY / magnitude * radius;

    closestPoint.cartesianRepoint(closestX, closestY);
}

double CircleStepper::determineStartingAzimuthFromCenter()
{
    return start.getAzimuth() + (PI / 2);
}
