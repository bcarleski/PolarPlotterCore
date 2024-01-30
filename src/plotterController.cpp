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

PlotterController::PlotterController(Print &printer, StatusUpdate &statusUpdater, PolarPlotter &plotter)
    : printer(printer),
      statusUpdater(statusUpdater),
      plotter(plotter)
{
}

void PlotterController::performCycle()
{
  if (!hasSteps)
  {
    if (this->needsCommands()) return;

    String command = commands[commandIndex++];
    String str = "";
    statusUpdater.status(drawing, str + (commandIndex) + ": " + command);

    this->executeCommand(command);
  }

  if (hasSteps)
  {
    hasSteps = plotter.step();
  }
}

void PlotterController::executeCommand(String& command) {
  const char chr = command.charAt(0);

  switch (chr) {
    case 'D':
    case 'd':
      setDebug(command);
      break;
    case 'H':
    case 'h':
      printer.println(PolarPlotter::getHelpMessage());
      break;
    case 'W':
    case 'w':
      printer.println("Executing wipe");
      plotter.executeWipe();
      break;
    default:
      printer.print("Computing steps for command ");
      printer.print(commandIndex);
      printer.print(": ");
      printer.println(command);
      plotter.computeSteps(command);
      printer.print("    Computed step count: ");
      printer.println(plotter.getStepCount());
      hasSteps = plotter.getStepCount() > 0;
      printer.print("    Has Steps: ");
      printer.println(hasSteps);
      break;
  }
}

void PlotterController::setDebug(String& command) {
  int debug = command.substring(1).toInt();
  printer.print("Setting debug to ");
  printer.println(debug);
  plotter.setDebug(debug);
}

bool PlotterController::canCycle()
{
  return hasSteps || !this->needsCommands();
}

bool PlotterController::needsCommands()
{
  return commandIndex >= commandCount;
}

void PlotterController::newDrawing(String &drawing)
{
  commandIndex = 0;
  commandCount = 0;
  this->drawing = drawing;
#ifndef __IN_TEST__
  statusUpdater.status("WIPING");
  printer.println("Executing wipe");
  plotter.executeWipe();
#endif
}

void PlotterController::addCommand(String &command)
{
  if (commandCount < MAX_COMMAND_COUNT)
  {
    commands[commandCount++] = command;
  }
}
