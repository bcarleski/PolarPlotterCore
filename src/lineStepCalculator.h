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

#ifndef _POLARPLOTTERCORE_LINESTEPCALCULATOR_H_
#define _POLARPLOTTERCORE_LINESTEPCALCULATOR_H_

#include "stepBank.h"
#include "extendedPrinter.h"

class LineStepCalculator
{
private:
  ExtendedPrinter printer;
  unsigned int debugLevel;
  Point middle;
  Point keepMinorIncrementMajor;
  Point incrementMinorIncrementMajor;
  Point incrementMinorKeepMajor;
  Point decrementMinorIncrementMajor;
  Point decrementMinorKeepMajor;
  float radiusStepSize;
  float azimuthStepSize;
  float startA;
  float radiusStepOffset;
  float azimuthStepOffset;
  int radiusSteps;
  int azimuthSteps;

protected:
  virtual void computeLineProperties(Point &start, Point &finish);
  virtual void handleDirectChanges(Point &start, Point &finish, StepBank &steps);
  virtual void determineMiddle(Point &start, Point &finish);
  void addBulkSteps(StepBank &steps, float radiusStepOffset, float azimuthStepOffset, int stepCount);
  void addCompoundStepsForLine(StepBank &steps, Point &start, Point &finish, float startA);
  void showLine(String prefix, bool fullLine, Point &start, Point &finish, int radiusSteps, int azimuthSteps, float radiusStepOffset, float azimuthStepOffset);
  void showDeltas(float xDelta, float yDelta, float distanceDenominator);
  void showPoints(Point &kpMinIncMajP, Point &incMinIncMajP, Point &incMinKpMajP, Point &decMinIncMajP, Point &decMinKpMajP);
  void showDistances(String prefix, String distanceToWhere, float kpMinIncMajDist, float incMinIncMajDist, float incMinKpMajDist, float decMinIncMajDist, float decMinKpMajDist);
  void showLastDistanceAndSteps(float lastDist, int minorStep, int majorStep);
  void addStep(StepBank &steps, float radiusStepOffset, int radiusStepIncrement, float azimuthStepOffset, int azimuthStepIncrement);
  int getStepValue(float offset, int increment);
  virtual float findDistanceToFinish(Point &finish, Point &point);
  virtual float findDistanceFromLine(Point &start, Point &point, float xDelta, float yDelta, float distanceDenominator);
  void orientEndPoint(Point &start, Point &end);

public:
  LineStepCalculator(Print &printer, float radiusStepSize, float azimuthStepSize);
  void addLineSteps(Point &start, Point &finish, StepBank &steps);
  void setDebug(unsigned int level);
};

#endif
