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

#include "plotterController.h"

PlotterController::PlotterController(CondOut& condOut, PolarPlotter& plotter)
  : condOut(condOut),
    plotter(plotter) {
}

void PlotterController::performCycle() {
  if (!this->hasSteps) {
    if (!this->needsCommands()) this->commandIndex++;
    if (this->needsCommands()) return;

    String command = this->commands[this->commandIndex];
    String str = "";
    this->condOut.lcdPrint(this->drawing, str + (this->commandIndex + 1) + ": " + command);
    this->condOut.lcdSave();
    this->plotter.computeSteps(this->commands[this->commandIndex]);
    this->hasSteps = this->plotter.getStepCount() > 0;
  }

  if (this->hasSteps) {
    this->hasSteps = plotter.step();
  }
}

bool PlotterController::needsCommands() {
  return this->commandIndex >= this->commandCount;
}

void PlotterController::newDrawing(String& drawing) {
  this->commandIndex = 0;
  this->commandCount = 0;
  this->drawing = drawing;
  this->condOut.lcdPrint("WIPING", "");
  this->plotter.executeWipe();
}

void PlotterController::addCommand(String& command) {
  if (this->commandCount < MAX_COMMAND_COUNT) {
    this->commands[this->commandCount++] = command;
  }
}
