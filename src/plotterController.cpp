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

PlotterController::PlotterController(Print &printer, StatusUpdate &statusUpdater, double maxRadius, int marbleSizeInRadiusSteps)
    : printer(printer),
      statusUpdater(statusUpdater),
      plotter(PolarPlotter(printer, statusUpdater, maxRadius, marbleSizeInRadiusSteps)),
      state(INITIALIZING)
{
}

void PlotterController::calibrate(double radiusStepSize, double azimuthStepSize)
{
  state = RESUMING;
  printer.println("Resuming after calibration");
  isCalibrated = true;
  statusUpdater.setState("Resuming");
  plotter.calibrate(0.0, 0.0, radiusStepSize, azimuthStepSize);
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
  if (state == PAUSED) {
    return;
  }

  const bool calibrating = isCalibrating();
  const bool manual = isManual();
  if (calibrating || manual || !plotter.hasNextStep())
  {
    if (this->needsCommands()) {
      if (calibrating) {
        String defaultCommand = "C";

        this->handleCalibrationCommand(defaultCommand);
      } else if (!manual && state != RETRIEVING) {
        printer.println("Controller needs commands");
        statusUpdater.setState("Retrieving Commands");
        state = RETRIEVING;
      }

      return;
    } else {
      String command = commands[commandIndex++];
      String str = "";
      statusUpdater.status(drawing, str + (commandIndex) + ": " + command);

      this->executeCommand(command);
    }
  }

  if (!calibrating && !manual && plotter.hasNextStep()) {
    if (state != DRAWING) {
      printer.println("Starting drawing");
      state = DRAWING;
    }

    plotter.step();
  }
}

void PlotterController::executeCommand(String& command) {
  if (isManual()) {
    handleManualCommand(command);
    return;
  }
  if (isCalibrating()) {
    handleCalibrationCommand(command);
    return;
  }

  printer.print("Starting command ");
  printer.print(commandIndex);
  printer.print(": ");
  printer.println(command);
  plotter.startCommand(command);
  printer.print("    Has Steps: ");
  printer.println(plotter.hasNextStep());
}

void PlotterController::handleControlCommand(String& command) {
  const char chr2 = command.charAt(1);
  switch (chr2) {
    case 'C':
    case 'c':
      printer.println("Starting calibration");
      statusUpdater.setState("Calibrating Center");

      commandIndex = 0;
      commandCount = 0;
      state = CALIBRATING_ORIGIN;
      break;
    case 'M':
    case 'm':
      printer.println("Starting manual commands in radius mode");
      statusUpdater.setState("Manual Radius");
      state = MANUAL_RADIUS;
      break;
    case 'P':
    case 'p':
      if (state == DRAWING || state == RETRIEVING) {
        printer.println("Pausing");
        statusUpdater.setState("Paused");
        state = PAUSED;
      }
      break;
    case 'R':
    case 'r':
      if (state == PAUSED) {
        printer.println("Resuming");
        statusUpdater.setState("Resuming");
        state = RESUMING;
      }
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
      commandIndex = 0;
      commandCount = 0;
      String cmd = "W";
      addCommand(cmd);
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

  switch (state) {
    case CALIBRATING_ORIGIN:
    case CALIBRATING_RADIUS:
      switch (chr)
      {
        case 'A': // Accept
        case 'a':
          if (state == CALIBRATING_ORIGIN) {
            printer.println("Finished origin calibration, switching to radius calibration");
            statusUpdater.setState("Calibrating Edge");
            state = CALIBRATING_RADIUS;
            calibrationRadiusSteps = 0;
          } else {
            printer.println("Finished radius calibration, switching to azimuth calibration");
            statusUpdater.setState("Calibrating Circle");
            state = CALIBRATING_AZIMUTH;
            calibrationAzimuthSteps = 0;
          }
          break;
        case 'O': // Offset
        case 'o':
          radiusSteps = command.substring(1).toInt();
          break;
        case '+': radiusSteps = 1; break;
        case '-': radiusSteps = -1; break;
        default: azimuthSteps = state == CALIBRATING_ORIGIN ? 1 : 0; break;
      }

      if (state == CALIBRATING_RADIUS) calibrationRadiusSteps += radiusSteps;
    break;
    case CALIBRATING_AZIMUTH:
      switch (chr)
      {
        case 'A': // Accept
        case 'a':
          printer.println("Finished azimuth calibration, ending calibration");
          statusUpdater.setState("Resuming");
          state = RESUMING;
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

  if (state == RESUMING) {
    printer.print("Finished calibration, radiusSteps=");
    printer.print(calibrationRadiusSteps);
    printer.print(", azimuthSteps=");
    printer.println(calibrationAzimuthSteps);

    if (recalibrater) {
      recalibrater(calibrationRadiusSteps, calibrationAzimuthSteps);
    }
  }
}

bool PlotterController::isCalibrating() {
  return state == CALIBRATING_ORIGIN || state == CALIBRATING_RADIUS || state == CALIBRATING_AZIMUTH;
}

void PlotterController::handleManualCommand(String& command) {
  const char chr = command.charAt(0);
  int azimuthSteps = 0;
  int radiusSteps = 0;

  switch (chr)
  {
    case 'A': // Manual Azimuth
    case 'a':
      if (state != MANUAL_AZIMUTH) {
        printer.println("Switching to manual azimuth");
        statusUpdater.setState("Manual Azimuth");
        state = MANUAL_AZIMUTH;
      }
      break;
    case 'R': // Manual Radius
    case 'r':
      if (state != MANUAL_RADIUS) {
        printer.println("Switching to manual radius");
        statusUpdater.setState("Manual Radius");
        state = MANUAL_RADIUS;
      }
      break;
    case 'X': // Exit
    case 'x':
      if (state == MANUAL_AZIMUTH || state == MANUAL_RADIUS) {
        if (isCalibrated) {
          printer.println("Resuming drawing from manual mode");
          statusUpdater.setState("Resuming");
          state = RESUMING;
        } else {
          printer.println("Resuming calibration from manual mode");
          statusUpdater.setState("Calibrating Center");
          state = CALIBRATING_ORIGIN;
        }
      }
      break;
    case 'O': // Offset
    case 'o':
      if (state == MANUAL_AZIMUTH) {
        azimuthSteps = command.substring(1).toInt();
      } else if (state == MANUAL_RADIUS) {
        radiusSteps = command.substring(1).toInt();
      }
      break;
    default: break;
  }

  if (stepper) {
    const int aStep = azimuthSteps > 0 ? 1 : (azimuthSteps < 0 ? -1 : 0);
    const int rStep = radiusSteps > 0 ? 1 : (radiusSteps < 0 ? -1 : 0);
    const int maxSteps = abs(azimuthSteps) > abs(radiusSteps) ? abs(azimuthSteps) : abs(radiusSteps);

    for (int i = 0; i < maxSteps; i++) {
      stepper(rStep, aStep, true);
    }
  }
}

bool PlotterController::isManual() {
  return state == MANUAL_RADIUS || state == MANUAL_AZIMUTH;
}

void PlotterController::setDebug(String& command) {
  int debug = command.substring(1).toInt();
  printer.print("Setting debug to ");
  printer.println(debug);
  plotter.setDebug(debug);
}

bool PlotterController::canCycle()
{
  return state == PAUSED || isCalibrating() || isManual() || plotter.hasNextStep() || !this->needsCommands();
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
  const String drawingArg = drawing;
  statusUpdater.setCurrentDrawing(drawingArg);
}

void PlotterController::addCommand(String &command)
{
  if (command.charAt(0) == '.') {
    handleControlCommand(command);
    return;
  }

  if (commandCount < MAX_COMMAND_COUNT)
  {
    commands[commandCount++] = command;
  }
}
