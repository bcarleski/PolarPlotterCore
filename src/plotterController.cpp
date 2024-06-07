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

PlotterController::PlotterController(Print &printer, StatusUpdate &statusUpdater, double maxRadius, int marbleSizeInRadiusSteps, PolarMotorCoordinator* coordinator)
    : printer(printer),
      statusUpdater(statusUpdater),
      plotter(PolarPlotter(printer, statusUpdater, maxRadius, marbleSizeInRadiusSteps, coordinator)),
      state(INITIALIZING),
      lastState(INITIALIZING),
      lastTextState("Initializing"),
      coordinator(coordinator)
{
}

void PlotterController::calibrate(double radiusStepSize, double azimuthStepSize)
{
  state = lastState;
  this->radiusStepSize = radiusStepSize;
  printer.print("Resuming after calibration, radiusStepSize=");
  printer.print(radiusStepSize, 8);
  printer.print(", azimuthStepSize=");
  printer.println(azimuthStepSize, 8);

  isCalibrated = true;
  statusUpdater.setState(lastTextState);
  plotter.calibrate(0.0, 0.0, radiusStepSize, azimuthStepSize);
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
      String defaultCommand = ".";

      if (calibrating) {
        this->handleCalibrationCommand(defaultCommand);
      } else if (manual) {
        this->handleManualCommand(defaultCommand);
      } else if (coordinator && coordinator->isMoving()) {
        state = FINISHING_MOVING;
      } else if (state != RETRIEVING) {
        printer.println("Controller needs commands");
        lastTextState = "Retrieving Commands";
        statusUpdater.setState(lastTextState);
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
      lastTextState = "Drawing";
      statusUpdater.setState(lastTextState);
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
  String msg = "";

  switch (chr2) {
    case 'C': case 'c':
      printer.println("Starting calibration");
      statusUpdater.setState("Calibrating Center");

      commandIndex = 0;
      commandCount = 0;
      lastState = state;
      state = CALIBRATING_ORIGIN;
      break;
    case 'M': case 'm':
      printer.println("Starting manual commands in radius mode");
      statusUpdater.setState("Manual Radius");
      lastState = state;
      state = MANUAL_RADIUS;
      break;
    case 'P': case 'p':
      if (coordinator) coordinator->pause();
      if (state == DRAWING || state == RETRIEVING) {
        printer.println("Pausing");
        statusUpdater.setState("Paused");
        lastState = state;
        state = PAUSED;
      }
      break;
    case 'R': case 'r':
      if (coordinator) coordinator->resume();
      if (state == PAUSED) {
        printer.println("Resuming");
        statusUpdater.setState(lastTextState);
        state = lastState;
      }
      break;
    case 'S': case 's':
      msg = "JSON={";
      if (coordinator) {
        msg += "\"Coordinator\":{\"Mov\":"; msg += coordinator->isMoving();
        msg += ",\"Int\":"; msg += coordinator->getStepInterval();

        Step stp = coordinator->getCurrentPosition();
        msg += ",\"Pos\":{\"R\":"; msg += stp.getRadiusStep(); msg += ",\"A\":"; msg += stp.getAzimuthStep(); msg += ",\"F\":"; msg += stp.isFast(); msg += "}";

        stp = coordinator->getCurrentProgress();
        msg += ",\"Prg\":{\"R\":"; msg += stp.getRadiusStep(); msg += ",\"A\":"; msg += stp.getAzimuthStep(); msg += ",\"F\":"; msg += stp.isFast(); msg += "}";

        stp = coordinator->getCurrentStep();
        msg += ",\"Stp\":{\"R\":"; msg += stp.getRadiusStep(); msg += ",\"A\":"; msg += stp.getAzimuthStep(); msg += ",\"F\":"; msg += stp.isFast(); msg += "}";
      }

      msg += ",\"State\":"; msg += state;
      msg += ",\"LastState\":\""; msg += lastTextState;
      msg += "\",\"Drawing\":\""; msg += drawing;
      msg += "\",\"CommandCount\":"; msg += commandCount;
      msg += ",\"CommandIndex\":"; msg += commandIndex;
      msg += ",\"CalibrationRadiusSteps\":"; msg += calibrationRadiusSteps;
      msg += ",\"CalibrationAzimuthSteps\":"; msg += calibrationAzimuthSteps;
      msg += "}";
      statusUpdater.status(msg);
      printer.println(msg);
      break;
    case 'H': case 'h':
      printer.println(PolarPlotter::getHelpMessage());
      break;
    case 'W': case 'w':
      commandIndex = 0;
      commandCount = 0;
      String cmd = "W";
      plotter.startCommand(cmd);
      break;
  }
}

void PlotterController::handleCalibrationCommand(String& command) {
  const char chr = command.charAt(0);
  long radiusSteps = 0;
  long azimuthSteps = 0;

  if (chr != '.') {
    printer.print("Got calibration command:");
    printer.println(command);
  }

  switch (state) {
    case CALIBRATING_ORIGIN:
    case CALIBRATING_RADIUS:
      switch (chr)
      {
        case 'E':
        case 'e':
          if (state == CALIBRATING_ORIGIN) {
            String manualCalibration = command.substring(1);
            int commaPosition = manualCalibration.indexOf(',');
            calibrationRadiusSteps = manualCalibration.substring(0, commaPosition).toInt();
            calibrationAzimuthSteps = manualCalibration.substring(commaPosition + 1).toInt();
            printer.print("Using explicit calibration, calibrationRadiusSteps=");
            printer.print(calibrationRadiusSteps);
            printer.print(", calibrationAzimuthSteps=");
            printer.println(calibrationAzimuthSteps);
            printer.println("Waiting for calibration moves to finish");
            state = FINISHING_CALIBRATION;
          }
          break;
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
        default: azimuthSteps = state == CALIBRATING_ORIGIN ? 1 : 0; break;
      }

      if (state == CALIBRATING_RADIUS) calibrationRadiusSteps += radiusSteps;
    break;
    case CALIBRATING_AZIMUTH:
      switch (chr)
      {
        case 'A': // Accept
        case 'a':
          printer.println("Waiting for calibration moves to finish");
          state = FINISHING_CALIBRATION;
          radiusSteps = -1 * calibrationRadiusSteps;
          break;
        case 'O': // Offset
        case 'o':
          azimuthSteps = command.substring(1).toInt();
          break;
      }
      calibrationAzimuthSteps += azimuthSteps;
    break;
    case FINISHING_CALIBRATION:
      if (coordinator && !coordinator->isMoving()) {
        statusUpdater.setState(lastTextState);
        state = lastState;
        printer.print("Finished calibration, radiusSteps=");
        printer.print(calibrationRadiusSteps);
        printer.print(", azimuthSteps=");
        printer.println(calibrationAzimuthSteps);

        coordinator->declareOrigin();
        if (recalibrater) {
          recalibrater(calibrationRadiusSteps, calibrationAzimuthSteps);
        }
      }
    return;
  }

  manualStep(radiusSteps, azimuthSteps, chr != '.');
}

bool PlotterController::isCalibrating() {
  return state == CALIBRATING_ORIGIN || state == CALIBRATING_RADIUS || state == CALIBRATING_AZIMUTH || state == FINISHING_CALIBRATION;
}

void PlotterController::handleManualCommand(String& command) {
  if (!isManual()) return;

  const char chr = command.charAt(0);
  if (chr != '.') {
    printer.print("Got manual command:");
    printer.println(command);
  }

  long radiusSteps = 0;
  long azimuthSteps = 0;

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
    case 'C': // Manual Center
    case 'c':
      if (isCalibrated) {
        radiusSteps = -1 * round(plotter.getPosition().getRadius() / radiusStepSize);
        printer.print("Manualling moving to center, steps=");
        printer.println(radiusSteps);
      }
      break;
    case 'X': // Exit
    case 'x':
      if (isCalibrated) {
        printer.println("Resuming drawing from manual mode");
        statusUpdater.setState(lastTextState);
        state = lastState;
      } else {
        printer.println("Resuming calibration from manual mode");
        statusUpdater.setState("Calibrating Center");
        state = CALIBRATING_ORIGIN;
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

  manualStep(radiusSteps, azimuthSteps, chr != '.');
}

void PlotterController::manualStep(const long radiusSteps, const long azimuthSteps, const bool printStep) {
  if (radiusSteps == 0 && azimuthSteps == 0) return;

  if (printStep) {
    printer.print("Manual stepping, radiusSteps=");
    printer.print(radiusSteps);
    printer.print(", azimuthSteps=");
    printer.println(azimuthSteps);
  }

  plotter.moveTo(radiusSteps, azimuthSteps, true);
}

bool PlotterController::isManual() {
  return state == MANUAL_RADIUS || state == MANUAL_AZIMUTH;
}

bool PlotterController::canCycle()
{
  return state == PAUSED || state == FINISHING_MOVING || isCalibrating() || isManual() || plotter.hasNextStep() || !this->needsCommands();
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
