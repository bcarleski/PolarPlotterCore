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

void BaseStepper::calibrate(float radiusStepSize, float azimuthStepSize)
{
    this->radiusStepSize = radiusStepSize;
    this->azimuthStepSize = azimuthStepSize;
}

bool BaseStepper::hasStep() {
    return nextStep.hasStep();
}

Step &BaseStepper::step() {
    currentStep.setSteps(nextStep);
    currentPosition.cloneFrom(nextPosition);
    currentDistanceToFinish = nextDistanceToFinish;

    this->computeNextStep();

    return currentStep;
}

void BaseStepper::startNewLine(Point &currentPosition, String &arguments) {
    this->snapPointToClosestPossiblePosition(currentPosition);
    this->start.cloneFrom(currentPosition);
    this->currentPosition.cloneFrom(currentPosition);

    if (!this->parseArgumentsAndSetFinish(currentPosition, arguments)) {
        currentDistanceToFinish = 0;
        currentStep.setSteps(0, 0);
        nextStep.setSteps(0, 0);
        nextDistanceToFinish = 0;
        nextPosition.cloneFrom(currentPosition);
        return;
    }

    currentDistanceToFinish = this->findDistanceFromPointOnLineToFinish(currentPosition);

    // If we are at the center and need to rotate towards the end, we shouldn't use the default calculation to determine next position
    if (currentPosition.getRadius() < (radiusStepSize * 0.1)) {
        originExitAzimuth = this->determineStartingAzimuthFromCenter();
    } else {
        originExitAzimuth = currentPosition.getAzimuth();
    }

    currentStep.setSteps(1, 1);
    this->computeNextStep();
}

void BaseStepper::computeNextStep() {
    if (currentStep.hasStep()) {
        this->determineNextPositionAndDistance();
        this->setupNextStepFromNextPosition();
    }
}

void BaseStepper::determineNextPositionAndDistance() {
    // If we are at the origin and aren't pointed in the right direction, repoint
    if (currentPosition.getRadius() < (radiusStepSize * 0.1) && abs(originExitAzimuth - currentPosition.getAzimuth()) > (azimuthStepSize * 0.5)) {
        nextPosition.repoint(currentPosition.getRadius(), currentPosition.getAzimuth() + (azimuthStepSize * (originExitAzimuth > currentPosition.getAzimuth() ? 1 : -1)));
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
    float closestPointDistanceToFinish = pointsCloserToFinishDistance[0];
    float closestDistanceToLine = this->findDistanceBetweenPoints(closestPointToLine, pointsCloserToFinishOnLine[0]);

    for (int i = 1; i < pointsCloserToFinishCount; i++) {
        float distanceToLine = this->findDistanceBetweenPoints(pointsCloserToFinish[i], pointsCloserToFinishOnLine[i]);

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
        float nextA = nextPosition.getAzimuth();
        nextPosition.repoint(0, nextA);
        originExitAzimuth = nextA + PI * (nextA > finish.getAzimuth() ? -1 : 1);
    }
}

void BaseStepper::setupNextStepFromNextPosition() {
    float currentRadiusPlusMargin = currentPosition.getRadius() + radiusStepSize * 0.1;
    float currentRadiusMinusMargin = currentPosition.getRadius() - radiusStepSize * 0.1;
    float currentAzimuthPlusMargin = currentPosition.getAzimuth() + azimuthStepSize * 0.1;
    float currentAzimuthMinusMargin = currentPosition.getAzimuth() - azimuthStepSize * 0.1;
    int radiusStep = 0, azimuthStep = 0;

    if (nextPosition.getRadius() > currentRadiusPlusMargin) {
        radiusStep = 1;
    } else if (nextPosition.getRadius() < currentRadiusMinusMargin) {
        radiusStep = -1;
    }
    if (nextPosition.getAzimuth() > currentAzimuthPlusMargin) {
        azimuthStep = 1;
    } else if (nextPosition.getAzimuth() < currentAzimuthMinusMargin) {
        azimuthStep = -1;
    }

    nextStep.setSteps(radiusStep, azimuthStep);
}

void BaseStepper::setupNextPoints() {
    float curR = currentPosition.getRadius();
    float curA = currentPosition.getAzimuth();

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
        float distanceToFinish = this->findDistanceFromPointOnLineToFinish(nextClosestPointsOnLine[i]);
        if (distanceToFinish < currentDistanceToFinish) {
            pointsCloserToFinish[pointsCloserToFinishCount] = nextPoints[i];
            pointsCloserToFinishOnLine[pointsCloserToFinishCount] = nextClosestPointsOnLine[i];
            pointsCloserToFinishDistance[pointsCloserToFinishCount] = distanceToFinish;
            pointsCloserToFinishCount++;
        }
    }
}

void BaseStepper::orientPoint(Point &referencePoint, Point &pointToOrient) {
  float referenceA = referencePoint.getAzimuth();
  float fullCircle = PI * 2;
  float pointA = pointToOrient.getAzimuth();

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
  float radius = round(point.getRadius() / this->radiusStepSize) * this->radiusStepSize;
  float azimuth = round(point.getAzimuth() / this->azimuthStepSize) * this->azimuthStepSize;
  point.repoint(radius, azimuth);
}

float BaseStepper::findDistanceBetweenPoints(Point &first, Point &second) {
    float deltaX = first.getX() - second.getX();
    float deltaY = first.getY() - second.getY();

    return sqrt(deltaX * deltaX + deltaY * deltaY);
}