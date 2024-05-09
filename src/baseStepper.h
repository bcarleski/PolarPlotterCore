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

#ifndef _POLARPLOTTERCORE_BASESTEPPER_H_
#define _POLARPLOTTERCORE_BASESTEPPER_H_

#include "abstractStepper.h"
#define NEXT_POINT_COUNT 8

class BaseStepper : public AbstractStepper
{
protected:
    double radiusStepSize;
    double azimuthStepSize;

    Point start;
    double originExitAzimuth;
    Point finish;

    Point currentPosition;
    Step currentStep;
    double currentDistanceToFinish;

    Point nextPosition;
    Step nextStep;
    double nextDistanceToFinish;

    Point nextPoints[NEXT_POINT_COUNT];
    Point nextClosestPointsOnLine[NEXT_POINT_COUNT];
    Point pointsCloserToFinish[NEXT_POINT_COUNT];
    Point pointsCloserToFinishOnLine[NEXT_POINT_COUNT];
    double pointsCloserToFinishDistance[NEXT_POINT_COUNT];
    int pointsCloserToFinishCount;

    virtual void computeNextStep();
    virtual void determineNextPositionAndDistance();
    virtual void setupNextStepFromNextPosition();
    virtual void setupNextPoints();
    virtual void findNextClosestPointsOnLine();
    virtual void findPointsCloserToFinish();
    virtual void orientPoint(Point &referencePoint, Point &pointToOrient);
    virtual void snapPointToClosestPossiblePosition(Point &point);
    virtual double findDistanceBetweenPoints(Point &first, Point &second);

    virtual bool parseArgumentsAndSetFinish(Point &currentPosition, String &arguments) = 0;
    virtual double findDistanceFromPointOnLineToFinish(Point &point) = 0;
    virtual void setClosestPointOnLine(Point &point, Point &closestPoint) = 0;
    virtual double determineStartingAzimuthFromCenter() = 0;

public:
    virtual void calibrate(double radiusStepSize, double azimuthStepSize);
    virtual void startNewLine(Point &currentPosition, String &arguments);
    virtual bool hasStep();
    virtual Step& step();
    using AbstractStepper::isFastStep;
};

#endif
