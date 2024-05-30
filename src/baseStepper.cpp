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

#include "baseStepper.h"
#ifdef __SHOW_STEP_DETAILS__
#include <iostream>
#include <iomanip>
#endif

void BaseStepper::calibrate(double radiusStepSize, double azimuthStepSize)
{
    this->radiusStepSize = radiusStepSize;
    this->azimuthStepSize = azimuthStepSize;
}

bool BaseStepper::hasStep() {
    if (needNextStep) {
        currentPosition.cloneFrom(nextPosition);

        this->computeNextStep();

        currentDistanceToFinish = nextDistanceToFinish;
        needNextStep = false;
    }

    return nextStep.hasStep();
}

Step &BaseStepper::step() {
#ifdef __SHOW_STEP_DETAILS__
    long r = nextStep.getRadiusStep();
    long a = nextStep.getAzimuthStep();

    std::cout << "    Next Step: (" << r << "," << a << ")" << std::endl;
#endif
    needNextStep = true;
    return nextStep;
}

void BaseStepper::startNewLine(Point &currentPosition, String &arguments) {
    this->snapPointToClosestPossiblePosition(currentPosition);
    this->start.cloneFrom(currentPosition);
    this->currentPosition.cloneFrom(currentPosition);
    this->nextPosition.cloneFrom(currentPosition);

    if (!this->parseArgumentsAndSetFinish(currentPosition, arguments)) {
        currentDistanceToFinish = 0;
        nextStep.setSteps(0, 0);
        nextDistanceToFinish = 0;
        return;
    }

    currentDistanceToFinish = this->findDistanceFromPointOnLineToFinish(currentPosition);

    // If we are at the center and need to rotate towards the end, we shouldn't use the default calculation to determine next position
    if (currentPosition.getRadius() < (radiusStepSize * 0.1)) {
        originExitAzimuth = this->determineStartingAzimuthFromCenter();
    } else {
        originExitAzimuth = currentPosition.getAzimuth();
    }

#ifdef __SHOW_STEP_DETAILS__
    double r = finish.getRadius();
    double a = finish.getAzimuth();
    double x = finish.getX();
    double y = finish.getY();

    std::cout << std::setprecision(8) << "    Finish: (" << r << "," << a << "," << x << "," << y << ") - " << std::setprecision(14) << currentDistanceToFinish << " - " << originExitAzimuth << std::endl;
#endif

    needNextStep = true;
}

void BaseStepper::computeNextStep() {
    this->determineNextPositionAndDistance();
    this->setupNextStepFromNextPosition();
}

void BaseStepper::determineNextPositionAndDistance() {
    // If we are at the origin and aren't pointed in the right direction, repoint
    if (currentPosition.getRadius() < (radiusStepSize * 0.1) && abs(originExitAzimuth - currentPosition.getAzimuth()) > (azimuthStepSize * 0.5)) {
        nextPosition.repoint(currentPosition.getRadius(), originExitAzimuth);
        nextDistanceToFinish = currentDistanceToFinish;
        return;
    }

    if (abs(currentPosition.getRadius() - finish.getRadius()) <= (radiusStepSize * 0.5) && abs(currentPosition.getAzimuth() - finish.getAzimuth()) <= (azimuthStepSize * 0.5)) {
        nextPosition.cloneFrom(currentPosition);
        nextStep.setSteps(0, 0);
        nextDistanceToFinish = 0;
        return;
    }

    this->setupNextPoints();
    this->findNextClosestPointsOnLine();
    this->findPointsCloserToFinish();

    if (pointsCloserToFinishCount <= 0) {
        nextDistanceToFinish = currentDistanceToFinish;
        nextPosition.cloneFrom(currentPosition);
        return;
    }

    Point closestPointToLine = pointsCloserToFinish[0];
    double closestPointDistanceToFinish = pointsCloserToFinishDistance[0];
    double closestDistanceToLine = this->findDistanceBetweenPoints(closestPointToLine, pointsCloserToFinishOnLine[0]);

    for (int i = 1; i < pointsCloserToFinishCount; i++) {
        double distanceToLine = this->findDistanceBetweenPoints(pointsCloserToFinish[i], pointsCloserToFinishOnLine[i]);

        if (distanceToLine < closestDistanceToLine) {
            closestPointToLine = pointsCloserToFinish[i];
            closestPointDistanceToFinish = pointsCloserToFinishDistance[i];
            closestDistanceToLine = distanceToLine;
        }
    }

    nextPosition.cloneFrom(closestPointToLine);
    nextDistanceToFinish = closestPointDistanceToFinish;
    originExitAzimuth = nextPosition.getAzimuth();

    if (nextPosition.getRadius() < (radiusStepSize * 0.1)) {
        double nextA = nextPosition.getAzimuth();
        nextPosition.repoint(0, nextA);
        originExitAzimuth = nextA + PI * (nextA > finish.getAzimuth() ? -1 : 1);
    }
}

void BaseStepper::setupNextStepFromNextPosition() {
    double currentRadiusPlusMargin = currentPosition.getRadius() + radiusStepSize * 0.1;
    double currentRadiusMinusMargin = currentPosition.getRadius() - radiusStepSize * 0.1;
    double currentAzimuthPlusMargin = currentPosition.getAzimuth() + azimuthStepSize * 0.1;
    double currentAzimuthMinusMargin = currentPosition.getAzimuth() - azimuthStepSize * 0.1;
    long radiusStep = 0, azimuthStep = 0;

    if (nextPosition.getRadius() > currentRadiusPlusMargin || nextPosition.getRadius() < currentRadiusMinusMargin) {
        radiusStep = (long)round((nextPosition.getRadius() - currentPosition.getRadius()) / radiusStepSize);
    }
    if (nextPosition.getAzimuth() > currentAzimuthPlusMargin || nextPosition.getAzimuth() < currentAzimuthMinusMargin) {
        azimuthStep = (long)round((nextPosition.getAzimuth() - currentPosition.getAzimuth()) / azimuthStepSize);
    }

    nextStep.setSteps(radiusStep, azimuthStep);
    nextPosition.repoint(currentPosition.getRadius() + radiusStep * radiusStepSize,
                         currentPosition.getAzimuth() + azimuthStep * azimuthStepSize);
}

void BaseStepper::setupNextPoints() {
    double curR = currentPosition.getRadius();
    double curA = currentPosition.getAzimuth();
#ifdef __SHOW_STEP_DETAILS__
    double x = currentPosition.getX();
    double y = currentPosition.getY();

    std::cout << std::setprecision(8) << "    Current: (" << curR << "," << curA << "," << x << "," << y << ") - " << std::setprecision(14) << currentDistanceToFinish << std::endl;
#endif

    nextPoints[0].repoint(curR - radiusStepSize, curA - azimuthStepSize);
    nextPoints[1].repoint(curR - radiusStepSize, curA);
    nextPoints[2].repoint(curR - radiusStepSize, curA + azimuthStepSize);
    nextPoints[3].repoint(curR, curA - azimuthStepSize);
    nextPoints[4].repoint(curR, curA + azimuthStepSize);
    nextPoints[5].repoint(curR + radiusStepSize, curA - azimuthStepSize);
    nextPoints[6].repoint(curR + radiusStepSize, curA);
    nextPoints[7].repoint(curR + radiusStepSize, curA + azimuthStepSize);
}

void BaseStepper::findNextClosestPointsOnLine() {
    for (int i = 0; i < NEXT_POINT_COUNT; i++) {
        this->setClosestPointOnLine(nextPoints[i], nextClosestPointsOnLine[i]);
    }
}

void BaseStepper::findPointsCloserToFinish() {
    pointsCloserToFinishCount = 0;

    for (int i = 0; i < NEXT_POINT_COUNT; i++) {
        double distanceToFinish = this->findDistanceFromPointOnLineToFinish(nextClosestPointsOnLine[i]);
#ifdef __SHOW_STEP_DETAILS__
    double rC = nextClosestPointsOnLine[i].getRadius();
    double aC = nextClosestPointsOnLine[i].getAzimuth();
    double xC = nextClosestPointsOnLine[i].getX();
    double yC = nextClosestPointsOnLine[i].getY();
    double rN = nextPoints[i].getRadius();
    double aN = nextPoints[i].getAzimuth();
    double xN = nextPoints[i].getX();
    double yN = nextPoints[i].getY();
    bool closer = distanceToFinish < currentDistanceToFinish;

    std::cout << std::setprecision(8) << "    Distance to (" << rN << "," << aN << "," << xN << "," << yN << ") / (" << rC << "," << aC << "," << xC << "," << yC << ") - " << std::setprecision(14) << distanceToFinish << " / " << closer << std::endl;
#endif
        if (distanceToFinish < currentDistanceToFinish) {
            pointsCloserToFinish[pointsCloserToFinishCount] = nextPoints[i];
            pointsCloserToFinishOnLine[pointsCloserToFinishCount] = nextClosestPointsOnLine[i];
            pointsCloserToFinishDistance[pointsCloserToFinishCount] = distanceToFinish;
            pointsCloserToFinishCount++;
        }
    }
}

void BaseStepper::orientPoint(Point &referencePoint, Point &pointToOrient) {
  double referenceA = referencePoint.getAzimuth();
  double fullCircle = PI * 2;
  double pointA = pointToOrient.getAzimuth();

  // Ensure we go the way that has the shortest sweep
  while (abs(referenceA - pointA) > abs(referenceA - (pointA + fullCircle)))
  {
    pointA += fullCircle;
  }
  while (abs(referenceA - pointA) > abs(referenceA - (pointA - fullCircle)))
  {
    pointA -= fullCircle;
  }

  pointToOrient.repoint(pointToOrient.getRadius(), pointA);
}

void BaseStepper::snapPointToClosestPossiblePosition(Point &point) {
  double radius = round(point.getRadius() / this->radiusStepSize) * this->radiusStepSize;
  double azimuth = round(point.getAzimuth() / this->azimuthStepSize) * this->azimuthStepSize;
  point.repoint(radius, azimuth);
}

double BaseStepper::findDistanceBetweenPoints(Point &first, Point &second) {
    double deltaX = first.getX() - second.getX();
    double deltaY = first.getY() - second.getY();

    return sqrt(deltaX * deltaX + deltaY * deltaY);
}