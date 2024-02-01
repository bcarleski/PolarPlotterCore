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

BaseStepper::BaseStepper(float radiusStepSize, float azimuthStepSize)
  : radiusStepSize(radiusStepSize),
    azimuthStepSize(azimuthStepSize)
{
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

void BaseStepper::startNewLine(Point &currentPosition, Point &finish) {
    this->orientEndPoint(currentPosition, finish);
    this->start.cloneFrom(currentPosition);
    this->finish.cloneFrom(finish);
    this->currentPosition.cloneFrom(currentPosition);
    currentDistanceToFinish = this->findDistanceFromPointOnLineToFinish(currentPosition);

    this->computeNextStep();
}

void BaseStepper::computeNextStep() {
    this->determineNextPositionAndDistance();
    this->setupNextStepFromNextPosition();
}

void BaseStepper::determineNextPositionAndDistance() {
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
        if (nextPoints[i].getRadius() >= 0) {
            float distanceToFinish = this->findDistanceFromPointOnLineToFinish(nextClosestPointsOnLine[i]);
            if (distanceToFinish < currentDistanceToFinish) {
                pointsCloserToFinish[pointsCloserToFinishCount] = nextPoints[i];
                pointsCloserToFinishOnLine[pointsCloserToFinishCount] = nextClosestPointsOnLine[i];
                pointsCloserToFinishDistance[pointsCloserToFinishCount] = distanceToFinish;
                pointsCloserToFinishCount++;
            }
        }
    }
}

void BaseStepper::orientEndPoint(Point &start, Point &end) {
  float startA = start.getAzimuth();
  float fullCircle = PI * 2;
  float endA = end.getAzimuth();

  // Ensure we go the way that has the shortest sweep
  while (abs(startA - endA) > abs(startA - (endA + fullCircle)))
  {
    endA += fullCircle;
  }
  while (abs(startA - endA) > abs(startA - (endA - fullCircle)))
  {
    endA -= fullCircle;
  }

  end.repoint(end.getRadius(), endA);
}

float BaseStepper::findDistanceBetweenPoints(Point &first, Point &second) {
    float deltaX = first.getX() - second.getX();
    float deltaY = first.getY() - second.getY();

    return sqrt(deltaX * deltaX + deltaY * deltaY);
}
