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

#ifndef _POLARPLOTTERCORE_CONDOUT_H_
#define _POLARPLOTTERCORE_CONDOUT_H_

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "point.h"

class CondOut {
private:
  LiquidCrystal lcd;
  const bool useLcd;
  String lastLine1;
  String lastLine2;
  bool preserveLastLines;

public:
  CondOut(LiquidCrystal&, bool);
  void init();

  void print(String&);
  void print(const char[]);
  void print(int, int = DEC);
  void print(unsigned int, int = DEC);
  void print(float, int = DEC);
  void print(double, int = DEC);
  void print(long, int = DEC);
  void print(unsigned long, int = DEC);
  void print(String, Point&);
  void println(String&);
  void println(const char[]);
  void println(int, int = DEC);
  void println(unsigned int, int = DEC);
  void println(float, int = DEC);
  void println(double, int = DEC);
  void println(long, int = DEC);
  void println(unsigned long, int = DEC);
  void println(String, Point&);

  void clear();
  void setCursor(int, int);
  void lcdPrint(String&);
  void lcdPrint(const char[]);
  void lcdPrint(long, int = DEC);
  void lcdPrint(const char[], String&);
  void lcdPrint(const char[], const char[]);
  void lcdPrint(String&, const char[]);
  void lcdPrint(String&, String&);
  void lcdSave();
  void lcdPrintSaved();
};

#endif
