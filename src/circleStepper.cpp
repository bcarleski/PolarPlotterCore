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

CircleStepper::CircleStepper(float radiusStepSize, float azimuthStepSize)
    : BaseStepper(radiusStepSize, azimuthStepSize)
{
}

bool CircleStepper::parseArgumentsAndSetFinish(Point &currentPosition, String &arguments) {
    int firstComma = arguments.indexOf(',');
    int secondComma = arguments.indexOf(',', firstComma + 1);
    if (secondComma <= 2) return false;

    float centerX = arguments.substring(0, firstComma).toFloat();
    float centerY = arguments.substring(firstComma + 1, secondComma).toFloat();
    float thetaDegrees = arguments.substring(secondComma + 1).toFloat();
    float theta = thetaDegrees / 180 * PI;
    center.cartesianRepoint(centerX, centerY);
    this->orientPoint(currentPosition, center);

    radius = this->findDistanceBetweenPoints(currentPosition, center);

    if (radius < (radiusStepSize * 2) || radius < (azimuthStepSize * 2) || abs(theta) < (azimuthStepSize * 0.1)) return false;

    if (abs(theta) > PI) {
        theta = PI * (theta > 0 ? 1 : -1);
    }

    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    float cosHalfTheta = cos(theta * 0.5);
    float sinHalfTheta = sin(theta * 0.5);
    float curX = currentPosition.getX();
    float curY = currentPosition.getY();
    float cenX = center.getX();
    float cenY = center.getY();
    float centeredX = curX - cenX;
    float centeredY = curY - cenY;
    float middleX = centeredX * cosHalfTheta + centeredY * sinHalfTheta + cenX;
    float middleY = centeredX * sinHalfTheta + centeredY * cosHalfTheta + cenY;
    float finishX = centeredX * cosTheta + centeredY * sinTheta + cenX;
    float finishY = centeredX * sinTheta + centeredY * cosTheta + cenY;
    slope = (finishY - curY) / (finishX - curX);
    intercept = finishY - slope * finishX;
    float midY = slope * middleX + intercept;
    middleAboveLine = middleY > midY;

    finish.cartesianRepoint(finishX, finishY);
    this->orientPoint(currentPosition, finish);
    this->snapPointToClosestPossiblePosition(finish);
    slope = (finish.getY() - curY) / (finish.getX() - curX);
    intercept = finish.getY() - slope * finish.getX();

    return true;
}

float CircleStepper::findDistanceFromPointOnLineToFinish(Point &point)
{
    bool aboveLine = point.getY() > (slope * point.getX() + intercept);
    float distanceToFinish = this->findDistanceBetweenPoints(point, finish);
    float distanceOnLine = distanceToFinish;
    
    if (aboveLine != middleAboveLine) {
        float distanceToStart = this->findDistanceBetweenPoints(point, start);
        distanceOnLine += distanceToStart < distanceToFinish ? distanceToStart : distanceToFinish;
    }

    return distanceOnLine;
}

void CircleStepper::setClosestPointOnLine(Point &point, Point &closestPoint)
{
    float deltaX = point.getX() - center.getX();
    float deltaY = point.getY() - center.getY();
    float magnitude = sqrt(deltaX * deltaX + deltaY * deltaY);
    float closestX = center.getX() + deltaX / magnitude * radius;
    float closestY = center.getY() + deltaY / magnitude * radius;

    closestPoint.cartesianRepoint(closestX, closestY);
}

float CircleStepper::determineStartingAzimuthFromCenter()
{
    return start.getAzimuth() + (PI / 2);
}
