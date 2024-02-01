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
#ifdef __IN_TEST__
#include <iostream>
#endif

CircleStepper::CircleStepper(float radiusStepSize, float azimuthStepSize)
    : BaseStepper(radiusStepSize, azimuthStepSize)
{
}

bool CircleStepper::parseArguments(Point &currentPosition, String &arguments) {
    int firstComma = arguments.indexOf(',');
    int secondComma = arguments.indexOf(',', firstComma + 1);
#ifdef __IN_TEST__
    std::cout << "FirstComma=" << firstComma << ", SecondComma=" << secondComma << "\n";
#endif
    if (secondComma <= 2) return false;

    float centerX = arguments.substring(0, firstComma).toFloat();
    float centerY = arguments.substring(firstComma + 1, secondComma).toFloat();
    float thetaDegrees = arguments.substring(secondComma + 1).toFloat();
    float theta = thetaDegrees / 180 * PI;
    center.cartesianRepoint(centerX, centerY);
    this->orientPoint(currentPosition, center);

    radius = this->findDistanceBetweenPoints(currentPosition, center);

#ifdef __IN_TEST__
    std::cout << "Radius=" << radius << ", Theta=" << theta << "\n";
#endif
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
    float middleCenteredX = centeredX * cosHalfTheta + centeredY * sinHalfTheta;
    float middleCenteredY = centeredX * sinHalfTheta * -1 + centeredY * cosHalfTheta;
    float finishCenteredX = centeredX * cosTheta + centeredY * sinTheta;
    float finishCenteredY = centeredX * sinTheta * -1 + centeredY * cosTheta;

    middle.cartesianRepoint(middleCenteredX + cenX, middleCenteredY + cenY);
    this->orientPoint(currentPosition, middle);
    finish.cartesianRepoint(finishCenteredX + cenX, finishCenteredY + cenY);
    this->orientPoint(currentPosition, finish);
    middleToFinish = this->findDistanceBetweenPoints(middle, finish);
    this->snapPointToClosestPossiblePosition(finish);
#ifdef __IN_TEST__
    std::cout << "Finish=(" << finish.getX() << ", " << finish.getY() << ", " << finish.getRadius() << ", " << finish.getAzimuth() << ")\n";
#endif

    return true;
}

float CircleStepper::findDistanceFromPointOnLineToFinish(Point &point)
{
    float distanceToFinish = this->findDistanceBetweenPoints(point, finish);
    float distanceToMiddle = this->findDistanceBetweenPoints(point, middle);
    float totalDistance = distanceToFinish + (distanceToMiddle > middleToFinish ? distanceToMiddle - middleToFinish : 0);

#ifdef __IN_TEST__
    std::cout << "Point=(" << point.getX() << ", " << point.getY() << ", " << point.getRadius() << ", " << point.getAzimuth() << "), DistanceToMiddle=" << distanceToMiddle << ", DistanceToFinish=" << distanceToFinish << ", TotalDistance=" << totalDistance << "\n";
#endif

//     return theta * radius;
    return totalDistance;
}

void CircleStepper::setClosestPointOnLine(Point &point, Point &closestPoint)
{
    float deltaX = point.getX() - center.getX();
    float deltaY = point.getY() - center.getY();
    float magnitude = sqrt(deltaX * deltaX + deltaY * deltaY);
    float closestX = center.getX() + deltaX / magnitude * radius;
    float closestY = center.getY() + deltaY / magnitude * radius;

    closestPoint.cartesianRepoint(closestX, closestY);
#ifdef __IN_TEST__
    std::cout << "    Point=(" << point.getX() << ", " << point.getY() << ", " << point.getRadius() << ", " << point.getAzimuth() <<
        "), Closest=(" << closestPoint.getX() << ", " << closestPoint.getY() << ", " << closestPoint.getRadius() << ", " << closestPoint.getAzimuth() << ")\n";
#endif
}

float CircleStepper::determineStartingAzimuthFromCenter()
{
#ifdef __IN_TEST__
    std::cout << "Determining Starting Azimuth From Center\n";
#endif
    return start.getAzimuth() + (PI / 2);
}
