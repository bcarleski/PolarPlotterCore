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

#include "polarMotorCoordinator.h"

PolarMotorCoordinator::PolarMotorCoordinator(StepDirMotor *_radius, StepDirMotor *_azimuth,
                                             const int _interlocked, const int _minimumInterval, const int _maximumInterval,
                                             const double _slowSpeedIntervalMultiplier)
    : radius(_radius),
      azimuth(_azimuth),
      interlocked(_interlocked),
      minimumInterval(_minimumInterval),
      maximumInterval(_maximumInterval),
      currentInterval(_minimumInterval),
      slowSpeedIntervalMultiplier(_slowSpeedIntervalMultiplier)
{
}

void PolarMotorCoordinator::init()
{
    radius->init();
    azimuth->init();
}

void PolarMotorCoordinator::begin()
{
    radius->begin();
    azimuth->begin();
}

bool PolarMotorCoordinator::canAddSteps()
{
    return savingIndex != movingIndex;
}

void PolarMotorCoordinator::changeStepInterval(const unsigned long interval)
{
    unsigned long stepInterval = interval;

    if (stepInterval > maximumInterval) stepInterval = maximumInterval;
    else if (stepInterval < minimumInterval) stepInterval = minimumInterval;

    if (stepInterval != currentInterval)
    {
        currentInterval = stepInterval;
        recalculateMove();
    }
}

void PolarMotorCoordinator::addSteps(const long radiusStep, const long azimuthStep, const bool fastStep)
{
    if (!canAddSteps())
        return;

    steps[savingIndex].setStepsWithSpeed(radiusStep, azimuthStep, fastStep);
    savingIndex = getNextIndex(savingIndex);
}

void PolarMotorCoordinator::declareOrigin()
{
    pendingOriginIndex = savingIndex;
}

void PolarMotorCoordinator::move()
{
    if (paused)
        return;
    if (!radius->canMove() && !azimuth->canMove() && !prepareMove())
    {
        if (moving)
            moving = false;
        return;
    }

    if (!moving)
        moving = true;
    unsigned long currentMicros = micros();
    radius->move(currentMicros);
    azimuth->move(currentMicros);
}

void PolarMotorCoordinator::reset()
{
    stop();
    setCurrentStep(radius->getPosition() * -1, azimuth->getPosition() * -1, true);
}

void PolarMotorCoordinator::pause()
{
    paused = true;
}

void PolarMotorCoordinator::resume()
{
    paused = false;
    recalculateMove();
}

void PolarMotorCoordinator::stop()
{
    radius->setupMove(0, 0, 0);
    azimuth->setupMove(0, 0, 0);
    currentStep.setStepsWithSpeed(0, 0, false);
    moving = false;
    while (hasSteps()) movingIndex = getNextIndex(movingIndex);
}

bool PolarMotorCoordinator::isMoving()
{
    return moving || hasSteps();
}

int PolarMotorCoordinator::getNextIndex(int index)
{
    return (index + 1) % MAX_PENDING_STEPS;
}

bool PolarMotorCoordinator::hasSteps()
{
    return getNextIndex(movingIndex) != savingIndex;
}

bool PolarMotorCoordinator::prepareMove()
{
    int nextMovingIndex = getNextIndex(movingIndex);

    if (nextMovingIndex == pendingOriginIndex) {
        radius->declareOrigin();
        azimuth->declareOrigin();
        pendingOriginIndex = -1;
    }

    if (nextMovingIndex == savingIndex) {
        currentStep.setStepsWithSpeed(0, 0, false);
        return false;
    }

    movingIndex = getNextIndex(movingIndex);
    Step nextStep = steps[movingIndex];

    return setCurrentStep(nextStep.getRadiusStep(), nextStep.getAzimuthStep(), nextStep.isFast());
}

void PolarMotorCoordinator::recalculateMove()
{
    if (!moving)
        return;

    Step nextStep = steps[movingIndex];
    long nextRadiusSteps = nextStep.getRadiusStep() - radius->getCurrentStep();
    long nextAzimuthSteps = nextStep.getAzimuthStep() - azimuth->getCurrentStep();
    bool fastStep = nextStep.isFast();

    setCurrentStep(nextRadiusSteps, nextAzimuthSteps, fastStep);
}

bool PolarMotorCoordinator::setCurrentStep(const long nextRadiusSteps, const long nextAzimuthSteps, const bool fastStep)
{
    currentStep.setStepsWithSpeed(nextRadiusSteps, nextAzimuthSteps, fastStep);
    return setupMove(nextRadiusSteps, nextAzimuthSteps, fastStep);
}

bool PolarMotorCoordinator::setupMove(const long nextRadiusSteps, const long nextAzimuthSteps, const bool fastStep)
{
    long rSteps = abs(nextRadiusSteps);
    long aSteps = abs(nextAzimuthSteps);
    long maxSteps = rSteps > aSteps ? rSteps : aSteps;

    if (maxSteps == 0)
        return false;

    unsigned long moveTime = round(maxSteps * currentInterval * (fastStep ? 1 : slowSpeedIntervalMultiplier));
    unsigned long radiusStepTimeDelta = moveTime / rSteps;
    unsigned long azimuthStepTimeDelta = moveTime / aSteps;
    unsigned long currentMicros = micros();

    radius->setupMove(nextRadiusSteps, currentMicros, radiusStepTimeDelta);
    azimuth->setupMove(nextAzimuthSteps, currentMicros, azimuthStepTimeDelta);
}

unsigned long PolarMotorCoordinator::getStepInterval()
{
    return currentInterval;
}

Step PolarMotorCoordinator::getCurrentPosition()
{
    currentPosition.setStepsWithSpeed(radius->getPosition(), azimuth->getPosition(), true);
    return currentPosition;
}

Step PolarMotorCoordinator::getCurrentProgress()
{
    currentProgress.setStepsWithSpeed(radius->getCurrentStep(), azimuth->getCurrentStep(), false);
    return currentProgress;
}

Step PolarMotorCoordinator::getCurrentStep()
{
    return currentStep;
}
