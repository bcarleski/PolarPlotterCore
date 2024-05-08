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

PlotterController::PlotterController(Print &printer, StatusUpdate &statusUpdater, float maxRadius, int marbleSizeInRadiusSteps)
    : printer(printer),
      statusUpdater(statusUpdater),
      plotter(PolarPlotter(printer, statusUpdater, maxRadius, marbleSizeInRadiusSteps)),
      calibrationMode(OFF)
{
}

void PlotterController::calibrate(float radiusStepSize, float azimuthStepSize)
{
  this->calibrationMode = OFF;
  this->plotter.calibrate(0.0, 0.0, radiusStepSize, azimuthStepSize);
}

void PlotterController::onStep(void stepper(const int radiusSteps, const int azimuthSteps, const bool fastStep))
{
  plotter.onStep(stepper);
  this->stepper = stepper;
}

void PlotterController::onRecalibrate(void recalibrater(const int maxRadiusSteps, const int fullCircleAzimuthSteps))
{
  this->recalibrater = recalibrater;
}

void PlotterController::performCycle()
{
  if (calibrationMode != OFF || !plotter.hasNextStep())
  {
    if (this->needsCommands()) {
      if (calibrationMode != OFF) {
        String defaultCommand = "C";

        this->handleCalibrationCommand(defaultCommand);
      }

      return;
    } else {
      String command = commands[commandIndex++];
      String str = "";
      statusUpdater.status(drawing, str + (commandIndex) + ": " + command);

      this->executeCommand(command);
    }
  }

  if (calibrationMode == OFF && plotter.hasNextStep()) plotter.step();
}

void PlotterController::executeCommand(String& command) {
  if (calibrationMode != OFF) {
    handleCalibrationCommand(command);
    return;
  }

  const char chr = command.charAt(0);
  switch (chr) {
    case '.':
      handleInternalCommand(command);
      break;
    default:
      printer.print("Starting command ");
      printer.print(commandIndex);
      printer.print(": ");
      printer.println(command);
      plotter.startCommand(command);
      printer.print("    Has Steps: ");
      printer.println(plotter.hasNextStep());
      break;
  }
}

void PlotterController::handleInternalCommand(String& command) {
  const char chr2 = command.charAt(1);
  switch (chr2) {
    case 'C':
    case 'c':
      printer.println("Starting calibration");
      calibrationMode = ORIGIN;
      break;
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
  }
}

void PlotterController::handleCalibrationCommand(String& command) {
  const char chr = command.charAt(0);
  int azimuthSteps = 0;
  int radiusSteps = 0;

  if (chr != 'C') {
    printer.print("Got calibration command:");
    printer.println(command);
  }

  switch (calibrationMode) {
    case ORIGIN:
    case RADIUS:
      switch (chr)
      {
        case 'A': // Accept
        case 'a':
          if (calibrationMode == ORIGIN) {
            printer.println("Finished origin calibration, switching to radius calibration");
            calibrationMode = RADIUS;
            calibrationRadiusSteps = 0;
          } else {
            printer.println("Finished radius calibration, switching to azimuth calibration");
            calibrationMode = AZIMUTH;
            calibrationAzimuthSteps = 0;
          }
          break;
        case 'O': // Offset
        case 'o':
          radiusSteps = command.substring(1).toInt();
          break;
        case '+': radiusSteps = 1; break;
        case '-': radiusSteps = -1; break;
        default: azimuthSteps = calibrationMode == ORIGIN ? 1 : 0; break;
      }

      if (calibrationMode == RADIUS) calibrationRadiusSteps += radiusSteps;
    break;
    case AZIMUTH:
      switch (chr)
      {
        case 'A': // Accept
        case 'a':
          printer.println("Finished azimuth calibration, ending calibration");
          calibrationMode = OFF;
          radiusSteps = -1 * calibrationRadiusSteps;
          break;
        case 'O': // Offset
        case 'o':
          azimuthSteps = command.substring(1).toInt();
          break;
        case '+': azimuthSteps = 1; break;
        case '-': azimuthSteps = -1; break;
      }
      calibrationAzimuthSteps += azimuthSteps;
    break;
  }

  if (stepper) {
    const int aStep = azimuthSteps > 0 ? 1 : (azimuthSteps < 0 ? -1 : 0);
    const int rStep = radiusSteps > 0 ? 1 : (radiusSteps < 0 ? -1 : 0);
    const int maxSteps = abs(azimuthSteps) > abs(radiusSteps) ? abs(azimuthSteps) : abs(radiusSteps);
    if (chr != 'C') {
      printer.print("Moving, radiusSteps=");
      printer.print(radiusSteps);
      printer.print(", azimuthSteps=");
      printer.println(azimuthSteps);
    }

    for (int i = 0; i < maxSteps; i++) {
      stepper(rStep, aStep, true);
    }
  }

  if (calibrationMode == OFF) {
    printer.print("Finished calibration, radiusSteps=");
    printer.print(calibrationRadiusSteps);
    printer.print(", azimuthSteps=");
    printer.println(calibrationAzimuthSteps);

    if (recalibrater) {
      recalibrater(calibrationRadiusSteps, calibrationAzimuthSteps);
    }
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
  return calibrationMode != OFF || plotter.hasNextStep() || !this->needsCommands();
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
