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

#include "lineStepCalculator.h"

LineStepCalculator::LineStepCalculator(Print &printer, float radiusStepSize, float azimuthStepSize)
    : printer(ExtendedPrinter(printer)),
      radiusStepSize(radiusStepSize),
      azimuthStepSize(azimuthStepSize)
{
}

void LineStepCalculator::addLineSteps(Point &start, Point &finish, StepBank &steps)
{
  computeLineProperties(start, finish);

  showLine(" Going", true, start, finish, radiusSteps, azimuthSteps, radiusStepOffset, azimuthStepOffset);

  handleDirectChanges(start, finish, steps);

  if (radiusSteps == 0 && azimuthSteps == 0)
  {
    return;
  }

  determineMiddle(start, finish);
  addCompoundStepsForLine(steps, start, this->middle, startA);
  addCompoundStepsForLine(steps, this->middle, finish, this->middle.getAzimuth());
}

void LineStepCalculator::computeLineProperties(Point &start, Point &finish) {
  orientEndPoint(start, finish);

  startA = start.getAzimuth();
  radiusStepOffset = finish.getRadius() > start.getRadius() ? this->radiusStepSize : (-1 * this->radiusStepSize);
  azimuthStepOffset = finish.getAzimuth() > startA ? this->azimuthStepSize : (-1 * this->azimuthStepSize);
  radiusSteps = round(abs(finish.getRadius() - start.getRadius()) / this->radiusStepSize);
  azimuthSteps = round(abs(finish.getAzimuth() - startA) / this->azimuthStepSize);
}

void LineStepCalculator::handleDirectChanges(Point &start, Point &finish, StepBank &steps) {
  // If we are at the origin, make sure we are pointed in the right direction before heading out
  if (azimuthSteps > 0 && abs(start.getRadius()) <= (this->radiusStepSize * 0.1))
  {
    addBulkSteps(steps, 0, azimuthStepOffset, azimuthSteps);
    startA = finish.getAzimuth();
    azimuthSteps = 0;
  }

  // If we are pointed in the right direction, just push in/out to where we need to go
  if (radiusSteps > 0 && azimuthSteps == 0)
  {
    addBulkSteps(steps, radiusStepOffset, 0, radiusSteps);
    radiusSteps = 0;
  }
}

void LineStepCalculator::determineMiddle(Point &start, Point &finish) {
  // Draw the line in two section, to better handle line drawing
  this->middle.cartesianRepoint((start.getX() + finish.getX()) / 2, (start.getY() + finish.getY()) / 2);
  orientEndPoint(start, this->middle);
}

void LineStepCalculator::setDebug(unsigned int level)
{
  debugLevel = level;
}

void LineStepCalculator::addBulkSteps(StepBank &steps, float radiusStepOffset, float azimuthStepOffset, int stepCount)
{
  int radiusStep = radiusStepOffset > 0.0000001 ? 1 : (radiusStepOffset < -0.0000001 ? -1 : 0);
  int azimuthStep = azimuthStepOffset > 0.0000001 ? 1 : (azimuthStepOffset < -0.0000001 ? -1 : 0);

  for (int i = 0; i < stepCount; i++)
  {
    steps.addStep(radiusStep, azimuthStep);
  }
}

void LineStepCalculator::addCompoundStepsForLine(StepBank &steps, Point &start, Point &finish, float startA)
{
  Point kpMinIncMajP = this->keepMinorIncrementMajor;
  Point incMinIncMajP = this->incrementMinorIncrementMajor;
  Point incMinKpMajP = this->incrementMinorKeepMajor;
  Point decMinIncMajP = this->decrementMinorIncrementMajor;
  Point decMinKpMajP = this->decrementMinorKeepMajor;

  float xDelta = start.getX() - finish.getX();
  float yDelta = finish.getY() - start.getY();
  float distanceDenominator = sqrt(xDelta * xDelta + yDelta * yDelta);
  float radiusStepOffset = finish.getRadius() > start.getRadius() ? this->radiusStepSize : (-1 * this->radiusStepSize);
  float azimuthStepOffset = finish.getAzimuth() > startA ? this->azimuthStepSize : (-1 * this->azimuthStepSize);
  int radiusSteps = round(abs(finish.getRadius() - start.getRadius()) / this->radiusStepSize);
  int azimuthSteps = round(abs(finish.getAzimuth() - startA) / this->azimuthStepSize);
  bool moreRadiusSteps = radiusSteps > azimuthSteps;
  int majorSteps = moreRadiusSteps ? radiusSteps : azimuthSteps;
  int minorStep = 0;

  showLine("  Compound steps", false, start, finish, radiusSteps, azimuthSteps, radiusStepOffset, azimuthStepOffset);
  showDeltas(xDelta, yDelta, distanceDenominator);

  float lastDist = sqrt(xDelta * xDelta + yDelta * yDelta);
  for (int majorStep = 0; majorStep <= majorSteps;)
  {
    float radius = start.getRadius() + (moreRadiusSteps ? majorStep : minorStep) * radiusStepOffset;
    float azimuth = startA + (moreRadiusSteps ? minorStep : majorStep) * azimuthStepOffset;

    if (moreRadiusSteps)
    {
      kpMinIncMajP.repoint(radius + radiusStepOffset, azimuth);
      incMinIncMajP.repoint(radius + radiusStepOffset, azimuth + azimuthStepOffset);
      incMinKpMajP.repoint(radius, azimuth + azimuthStepOffset);
      decMinIncMajP.repoint(radius + radiusStepOffset, azimuth - azimuthStepOffset);
      decMinKpMajP.repoint(radius, azimuth - azimuthStepOffset);
    }
    else
    {
      kpMinIncMajP.repoint(radius, azimuth + azimuthStepOffset);
      incMinIncMajP.repoint(radius + radiusStepOffset, azimuth + azimuthStepOffset);
      incMinKpMajP.repoint(radius + radiusStepOffset, azimuth);
      decMinIncMajP.repoint(radius - radiusStepOffset, azimuth + azimuthStepOffset);
      decMinKpMajP.repoint(radius - radiusStepOffset, azimuth);
    }
    showPoints(kpMinIncMajP, incMinIncMajP, incMinKpMajP, decMinIncMajP, decMinKpMajP);

    float kpMinIncMajDistToFinish = findDistanceToFinish(finish, kpMinIncMajP);
    float incMinIncMajDistToFinish = findDistanceToFinish(finish, incMinIncMajP);
    float incMinKpMajDistToFinish = findDistanceToFinish(finish, incMinKpMajP);
    float decMinIncMajDistToFinish = findDistanceToFinish(finish, decMinIncMajP);
    float decMinKpMajDistToFinish = findDistanceToFinish(finish, decMinKpMajP);
    float kpMinIncMajDistToLine = findDistanceFromLine(start, kpMinIncMajP, xDelta, yDelta, distanceDenominator);
    float incMinIncMajDistToLine = findDistanceFromLine(start, incMinIncMajP, xDelta, yDelta, distanceDenominator);
    float incMinKpMajDistToLine = findDistanceFromLine(start, incMinKpMajP, xDelta, yDelta, distanceDenominator);
    float decMinIncMajDistToLine = findDistanceFromLine(start, decMinIncMajP, xDelta, yDelta, distanceDenominator);
    float decMinKpMajDistToLine = findDistanceFromLine(start, decMinKpMajP, xDelta, yDelta, distanceDenominator);

    bool validkpMinIncMaj = lastDist > kpMinIncMajDistToFinish;
    bool validIncMinIncMaj = lastDist > incMinIncMajDistToFinish;
    bool validIncMinKpMaj = lastDist > incMinKpMajDistToFinish;
    bool validDecMinIncMaj = lastDist > decMinIncMajDistToFinish;
    bool validDecMinKpMaj = lastDist > decMinKpMajDistToFinish;
    if (!validkpMinIncMaj && !validIncMinIncMaj && !validIncMinKpMaj && !validDecMinIncMaj && !validDecMinKpMaj)
    {
      break;
    }
    if (validIncMinIncMaj && (!validIncMinKpMaj || incMinIncMajDistToLine <= incMinKpMajDistToLine) && (!validkpMinIncMaj || incMinIncMajDistToLine <= kpMinIncMajDistToLine) && (!validDecMinIncMaj || incMinIncMajDistToLine <= decMinIncMajDistToLine) && (!validDecMinKpMaj || incMinIncMajDistToLine <= decMinKpMajDistToLine))
    {
      lastDist = incMinIncMajDistToFinish;
      minorStep++;
      majorStep++;
      addStep(steps, radiusStepOffset, 1, azimuthStepOffset, 1);
    }
    else if (validIncMinKpMaj && (!validIncMinIncMaj || incMinKpMajDistToLine < incMinIncMajDistToLine) && (!validkpMinIncMaj || incMinKpMajDistToLine < kpMinIncMajDistToLine) && (!validDecMinIncMaj || incMinIncMajDistToLine <= decMinIncMajDistToLine) && (!validDecMinKpMaj || incMinIncMajDistToLine <= decMinKpMajDistToLine))
    {
      lastDist = incMinKpMajDistToFinish;
      minorStep++;
      addStep(steps, radiusStepOffset, moreRadiusSteps ? 0 : 1, azimuthStepOffset, moreRadiusSteps ? 1 : 0);
    }
    else if (validDecMinIncMaj && (!validIncMinKpMaj || decMinIncMajDistToLine <= incMinKpMajDistToLine) && (!validkpMinIncMaj || decMinIncMajDistToLine <= kpMinIncMajDistToLine) && (!validIncMinIncMaj || decMinIncMajDistToLine <= incMinIncMajDistToLine) && (!validDecMinKpMaj || decMinIncMajDistToLine <= decMinKpMajDistToLine))
    {
      lastDist = decMinIncMajDistToFinish;
      minorStep--;
      majorStep++;
      addStep(steps, radiusStepOffset, moreRadiusSteps ? 1 : -1, azimuthStepOffset, moreRadiusSteps ? -1 : 1);
    }
    else if (validDecMinKpMaj && (!validIncMinKpMaj || decMinKpMajDistToLine <= incMinKpMajDistToLine) && (!validkpMinIncMaj || decMinKpMajDistToLine <= kpMinIncMajDistToLine) && (!validDecMinIncMaj || decMinKpMajDistToLine <= decMinIncMajDistToLine) && (!validIncMinIncMaj || decMinKpMajDistToLine <= incMinIncMajDistToLine))
    {
      lastDist = decMinKpMajDistToFinish;
      minorStep--;
      addStep(steps, radiusStepOffset, moreRadiusSteps ? 0 : -1, azimuthStepOffset, moreRadiusSteps ? -1 : 0);
    }
    else
    {
      lastDist = kpMinIncMajDistToFinish;
      majorStep++;
      addStep(steps, radiusStepOffset, moreRadiusSteps ? 1 : 0, azimuthStepOffset, moreRadiusSteps ? 0 : 1);
    }

    showDistances("    ", "Line", kpMinIncMajDistToLine, incMinIncMajDistToLine, incMinKpMajDistToLine, decMinIncMajDistToLine, decMinKpMajDistToLine);
    showDistances(", ", "Finish", kpMinIncMajDistToFinish, incMinIncMajDistToFinish, incMinKpMajDistToFinish, decMinIncMajDistToFinish, decMinKpMajDistToFinish);
    showLastDistanceAndSteps(lastDist, minorStep, majorStep);
  }
}

void LineStepCalculator::showLine(String prefix, bool fullLine, Point &start, Point &finish, int radiusSteps, int azimuthSteps, float radiusStepOffset, float azimuthStepOffset)
{
  if (debugLevel < 2)
  {
    return;
  }

  printer.print(prefix);
  printer.print(" from", start);
  printer.print(" to", finish);
  printer.print(", radiusSteps=");
  printer.print(radiusSteps);
  printer.print(", azimuthSteps=");
  printer.print(azimuthSteps);
  printer.print(", radiusStepOffset=");
  printer.print(radiusStepOffset, 4);
  printer.print(", azimuthStepOffset=");
  printer.print(azimuthStepOffset, 4);
  if (fullLine) {
    printer.println();
  }
}

void LineStepCalculator::showDeltas(float xDelta, float yDelta, float distanceDenominator)
{
  if (debugLevel < 2)
  {
    return;
  }

  printer.print(", xDelta=");
  printer.print(xDelta, 4);
  printer.print(", yDelta=");
  printer.print(yDelta, 4);
  printer.print(", distanceDenominator=");
  printer.println(distanceDenominator, 4);
}

void LineStepCalculator::showPoints(Point &kpMinIncMajP, Point &incMinIncMajP, Point &incMinKpMajP, Point &decMinIncMajP, Point &decMinKpMajP)
{
  if (debugLevel < 3)
  {
    return;
  }

  printer.print("   ");
  printer.print("KeepMinorIncrementMajorPoint", kpMinIncMajP);
  printer.print("; ");
  printer.print("IncrementMinorIncrementMajorPoint", incMinIncMajP);
  printer.print("; ");
  printer.print("IncrementMinorKeepMajorPoint", incMinKpMajP);
  printer.print("; ");
  printer.print("DecrementMinorIncrementMajorPoint", decMinIncMajP);
  printer.print("; ");
  printer.print("DecrementMinorKeepMajorPoint", decMinKpMajP);
  printer.println("");
}

void LineStepCalculator::showDistances(String prefix, String distanceToWhere, float kpMinIncMajDist, float incMinIncMajDist, float incMinKpMajDist, float decMinIncMajDist, float decMinKpMajDist)
{
  if (debugLevel < 4)
  {
    return;
  }

  printer.print(prefix);
  printer.print("KeepMinorIncrementMajorDistanceTo");
  printer.print(distanceToWhere);
  printer.print("=");
  printer.print(kpMinIncMajDist, 4);
  printer.print(", IncrementMinorIncrementMajorDistanceTo");
  printer.print(distanceToWhere);
  printer.print("=");
  printer.print(incMinIncMajDist, 4);
  printer.print(", IncrementMinorKeepMajorDistanceTo");
  printer.print(distanceToWhere);
  printer.print("=");
  printer.print(incMinKpMajDist, 4);
  printer.print(", DecrementMinorIncrementMajorDistanceTo");
  printer.print(distanceToWhere);
  printer.print("=");
  printer.print(decMinIncMajDist, 4);
  printer.print(", DecrementMinorKeepMajorDistanceTo");
  printer.print(distanceToWhere);
  printer.print("=");
  printer.print(decMinKpMajDist, 4);
}

void LineStepCalculator::showLastDistanceAndSteps(float lastDist, int minorStep, int majorStep)
{
  if (debugLevel < 4)
  {
    return;
  }

  printer.print(", NextLastDistanceToFinish=");
  printer.print(lastDist, 4);
  printer.print(", MinorStep=");
  printer.print(minorStep);
  printer.print(", MajorStep=");
  printer.println(majorStep);
}

void LineStepCalculator::addStep(StepBank &steps, float radiusStepOffset, int radiusStepIncrement, float azimuthStepOffset, int azimuthStepIncrement)
{
  int radiusStep = getStepValue(radiusStepOffset, radiusStepIncrement);
  int azimuthStep = getStepValue(azimuthStepOffset, azimuthStepIncrement);

  if (debugLevel >= 4)
  {
    printer.print("    RadiusStepIncrement=");
    printer.print(radiusStepIncrement, 4);
    printer.print(", RadiusStep=");
    printer.print(radiusStep, 4);
    printer.print(", AzimuthStepIncrement=");
    printer.print(azimuthStepIncrement, 4);
    printer.print(", AzimuthStep=");
    printer.println(azimuthStep, 4);
  }

  steps.addStep(radiusStep, azimuthStep);
}

int LineStepCalculator::getStepValue(float offset, int increment)
{
  if ((offset > 0.0000001 && increment > 0) || (offset < -0.0000001 && increment < 0))
  {
    return 1;
  }
  if ((offset > 0.0000001 && increment < 0) || (offset < -0.0000001 && increment > 0))
  {
    return -1;
  }

  return 0;
}

float LineStepCalculator::findDistanceToFinish(Point &finish, Point &point)
{
  float xDelta = finish.getX() - point.getX();
  float yDelta = finish.getY() - point.getY();
  return sqrt(xDelta * xDelta + yDelta * yDelta);
}

float LineStepCalculator::findDistanceFromLine(Point &start, Point &point, float xDelta, float yDelta, float distanceDenominator)
{
  return abs(xDelta * (point.getY() - start.getY()) - (start.getX() - point.getX()) * yDelta) / distanceDenominator;
}

void LineStepCalculator::orientEndPoint(Point &start, Point &end)
{
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
