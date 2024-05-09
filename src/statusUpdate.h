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

#ifndef _POLARPLOTTERCORE_STATUSPRINT_H_
#define _POLARPLOTTERCORE_STATUSPRINT_H_

#ifndef __IN_TEST__
#include <Arduino.h>
#else
#include "mockArduino.h"
#endif

class StatusUpdate
{
private:
  const String empty;
  String lastLine1;
  String lastLine2;
  bool preserveLastLines;

public:
  void status(const String &status) {
    this->writeStatus(status, empty);
  }
  void status(const char status[]) {
    String v(status);
    this->writeStatus(v, empty);
  }
  void status(const String &key, long value, int base = DEC) {
    String v(value);
    this->writeStatus(key, v);
  }
  void status(const char key[], long value, int base = DEC) {
    String k(key);
    String v(value);
    this->writeStatus(k, v);
  }
  void status(const String &key, const String &value) {
    this->writeStatus(key, value);
  }
  void status(const char key[], const String &value) {
    String k(key);
    this->writeStatus(k, value);
  }
  void status(const String &key, const char value[]) {
    String v(value);
    this->writeStatus(key, v);
  }
  void status(const char key[], const char value[]) {
    String k(key);
    String v(value);
    this->writeStatus(k, v);
  }

  virtual void setRadiusStepSize(const float value) = 0;
  virtual void setAzimuthStepSize(const float value) = 0;
  virtual void setCurrentDrawing(const String &value) = 0;
  virtual void setCurrentStep(const String &value) = 0;
  virtual void setPosition(const String &value) = 0;
  virtual void setState(const String &value) = 0;

protected:
  virtual void writeStatus(const String &key, const String &value) = 0;
};

#endif
