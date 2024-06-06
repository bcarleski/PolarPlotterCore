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

#ifndef _POLARPLOTTERCORE_POLARMOTORCOORDINATOR_H_
#define _POLARPLOTTERCORE_POLARMOTORCOORDINATOR_H_

#include "stepDirMotor.h"
#include "step.h"

#define MAX_PENDING_STEPS 100

class PolarMotorCoordinator
{
private:
    StepDirMotor *radius;
    StepDirMotor *azimuth;

    // Interlocked != 0 means that azimuth and radius are physically connected, such that turning the azimuth results in a change in radius
    // A positive value indicates that one step of the azimuth results in one or more steps of the radius in the same direction (value of interlocked is the # of steps)
    // A negative value indicates that one step of the azimuth results in one or more steps of the radius in the opposite direction
    int interlocked;
    unsigned long minimumInterval;
    unsigned long maximumInterval;
    unsigned long currentInterval;
    double slowSpeedIntervalMultiplier;
    Step steps[MAX_PENDING_STEPS];
    Step currentStep;
    Step currentPosition;
    Step currentProgress;
    int paused = false;
    int moving = false;
    int movingIndex = 0;
    int savingIndex = 1;
    int pendingOriginIndex = -1;

protected:
    int getNextIndex(int index);
    bool prepareMove();
    void recalculateMove();
    bool hasSteps();
    bool setCurrentStep(const long nextRadiusSteps, const long nextAzimuthSteps, const bool fastStep);
    bool setupMove(const long nextRadiusSteps, const long nextAzimuthSteps, const bool fastStep);

public:
    /**
     * Constructs a new polar motor coordinator.
     *
     * @param _radius the motor controlling the radial amount, the in/out of the coordinate
     * @param _azimuth the motor controlling the azimuth amount, the "how far around the circle" of the coordinate
     * @param _interlocked indicates how the radius and azimuth are physically connected.  A value of 0 means they are completely independent.  A positive value indicates that one azimuth step results in that many radial steps in the same direction.  A negative value indicates radial steps in the opposite direction.
     * @param _minimumInterval the lowest step interval that is allowed.  Setting the speed to a value below this will result in the speed being set to this value.
     * @param _maximumInterval the highest step interval that is allowed.  Setting the speed to a value above this will result in the speed being set to this value.
     * @param _slowSpeedIntervalMultiplier how many times slower is a "slow" step versus a "fast" step.  The fast step waits the current step interval between steps, while a slow step waits the current step interval times this multiplier.
     * @param _getMicros a function pointer to the method to get the current system microseconds, so we can run in both foreground and background threads
     * @param _delayMicros a function pointer to the method to wait for the given number of microseconds, so we can run in both foreground and background threads
     */
    PolarMotorCoordinator(StepDirMotor *_radius, StepDirMotor *_azimuth,
                          const int _interlocked, const int _minimumInterval, const int _maximumInterval,
                          const double _slowSpeedIntervalMultiplier);

    /**
     * Performs any initial setup of ports.  This method should be called before the begin method.
     */
    void init();

    /**
     * Puts the coordinator and its dependencies in a state to be ready to be used.  This method should be called after the init method.
     */
    void begin();

    /** Returns whether we can accept new steps at the current time. */
    virtual bool canAddSteps();

    /** Changes the interval between steps. This is effective immediately, and does not wait for pending moves to complete. */
    virtual void changeStepInterval(const unsigned long interval);

    /** Adds steps to the current pending queue of steps to run.  Fast steps run using the current step interval.  Slow steps run at the current interval multiplied by the slow speed interval multiplier. */
    virtual void addSteps(const long radiusStep, const long azimuthStep, const bool fastStep);

    /** Declares the current point (after any pending moves complete) as the (0, 0) origin. */
    virtual void declareOrigin();

    /** Moves a single step if enough time has passed, or returns if not enough time has passed, or there are no pending steps. */
    virtual void move();

    /** Moves back to the origin, after any other pending moves complete. */
    virtual void reset();

    /** Pauses all moves, resulting in the move() method returning immediately when called. */
    virtual void pause();

    /** Resumes after pausing. */
    virtual void resume();

    /** Cancels all pending moves, including the current move if we are in the process of moving. */
    virtual void stop();

    /** Returns true if there are still steps to move. */
    virtual bool isMoving();

    /** Returns the current step interval. */
    virtual unsigned long getStepInterval();

    /** Returns the current position in terms of radius steps and azimuth steps from the origin. */
    virtual Step getCurrentPosition();

    /** Returns the step each motor is on, so that it can be compared with the current step to see how far along they are. */
    virtual Step getCurrentProgress();

    /** Returns the currently processing step, or the empty step if nothing is currently processing. */
    virtual Step getCurrentStep();
};
#endif
