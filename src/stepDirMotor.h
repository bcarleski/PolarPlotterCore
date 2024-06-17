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

#ifndef _POLARPLOTTERCORE_STEPDIRMOTOR_H_
#define _POLARPLOTTERCORE_STEPDIRMOTOR_H_

#ifndef __IN_TEST__
#include <Arduino.h>
#include "pico/time.h"
#else
#include "mockArduino.h"
#endif

class StepDirMotor
{
protected:
    int stepPin;
    int dirPin;

    int position = 0;
    int currentStep = 0;
    int maxSteps = 0;
    bool reversed = false;

    unsigned long nextStepTime;
    unsigned long nextStepTimeDelta;

    virtual void initDriver() { }
    virtual void beginDriver() { }

public:
    StepDirMotor(const int _stepPin, const int _dirPin)
        : stepPin(_stepPin),
          dirPin(_dirPin) { }

    virtual void init()
    {
        pinMode(stepPin, OUTPUT);
        pinMode(dirPin, OUTPUT);
        this->initDriver();
    }

    virtual void begin()
    {
        digitalWrite(stepPin, LOW);
        digitalWrite(dirPin, LOW);
        this->beginDriver();
    }

    virtual void setupMove(const int steps, const unsigned long currentMicros, const unsigned long stepTimeDelta)
    {
        currentStep = 0;
        maxSteps = abs(steps);
        nextStepTimeDelta = stepTimeDelta;
        nextStepTime = currentMicros + stepTimeDelta;

        if ((steps < 0) != reversed)
        {
            reversed = steps < 0;
            digitalWrite(dirPin, reversed ? HIGH : LOW);
            sleep_us(20);
        }
    }

    virtual bool canMove()
    {
        return currentStep < maxSteps;
    }

    virtual void move(const unsigned long currentMicros)
    {
        if (!canMove() || currentMicros < nextStepTime)
            return;

        nextStepTime += nextStepTimeDelta;
        currentStep++;
        position += (reversed ? -1 : 1);

        digitalWrite(stepPin, HIGH);
        sleep_us(20);
        digitalWrite(stepPin, LOW);
    }

    virtual void declareOrigin()
    {
        position = 0;
    }

    virtual int getCurrentStep()
    {
        return currentStep * (reversed ? -1 : 1);
    }

    virtual int getPosition()
    {
        return position;
    }
};
#endif
