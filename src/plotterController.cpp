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

PlotterController::PlotterController(PolarPlotter& plotter, const String& deviceName, const String& shadowName)
  : plotter(plotter),
    getTopic("$aws/things/" + deviceName + "/shadow/name/" + shadowName + "/get"),
    getAcceptedTopic("$aws/things/" + deviceName + "/shadow/name/" + shadowName + "/get/accepted"),
    getRejectedTopic("$aws/things/" + deviceName + "/shadow/name/" + shadowName + "/get/rejected"),
    updateTopic("$aws/things/" + deviceName + "/shadow/name/" + shadowName + "/update"),
    toDeviceTopic(deviceName + "/toDevice"),
    fromDeviceTopic(deviceName + "/fromDevice"),
    currentDrawing("") {
  this->topics[0] = this->getAcceptedTopic;
  this->topics[1] = this->getRejectedTopic;
  this->topics[2] = this->toDeviceTopic;
}

void PlotterController::onMessage(void publishMessage(const String&, const JSONVar&)) {
  this->publishMessage = publishMessage;
}

void PlotterController::onMqttPoll(bool pollMqtt(String[])) {
  this->pollMqtt = pollMqtt;
}

void PlotterController::performCycle() {
  if (Serial && Serial.available()) {
    String command = Serial.readStringUntil('\n');

    if (command.startsWith("d") || command.startsWith("D")) {
      unsigned int debugLevel = (unsigned int)command.substring(1).toInt();
      this->plotter.setDebug(debugLevel);
    } else if (command.startsWith("h") || command.startsWith("H")) {
      Serial.println(PolarPlotter::getHelpMessage());
    } else if (command == "w" || command == "W") {
      this->plotter.executeWipe();
    } else if (command.startsWith("ra") || command.startsWith("RA")) {
      this->messageReceived(getAcceptedTopic, command.substring(2));
    } else if (command.startsWith("rr") || command.startsWith("RR")) {
      this->messageReceived(getRejectedTopic, command.substring(2));
    } else if (command.startsWith("rt") || command.startsWith("RT")) {
      this->messageReceived(toDeviceTopic, command.substring(2));
    } else {
      this->plotter.computeSteps(command);
      this->hasSteps = this->plotter.getStepCount() > 0;
    }
  }

  if (this->hasSteps) {
    this->hasSteps = plotter.step();
  }

  if (!this->pollMqtt(this->topics) || this->waitingForDeviceShadow || this->waitingForDrawing || this->waitingForLine) {
    return;
  }

  if (!this->initialized) {
    JSONVar empty;
    this->waitingForDeviceShadow = true;
    this->publishMessage(this->getTopic, empty);
    this->initialized = true;
    return;
  }

  if (this->hasSteps) {
    return;
  }

  if (this->currentDrawing == "" || this->currentLine >= (this->totalLines - 1)) {
    this->requestDrawing();
  } else {
    this->requestLine();
  }
}

void PlotterController::messageReceived(const String& topic, const String& message) {
  if (topic == this->getAcceptedTopic) {
    this->getAcceptedMessage(message);
  } else if (topic == this->getRejectedTopic) {
    this->getRejectedMessage(message);
  } else if (topic == this->toDeviceTopic) {
    this->toDeviceMessage(message);
  }
}

void PlotterController::requestDrawing() {
  this->waitingForDrawing = true;
  this->currentDrawing = "";
  this->currentLine = -1;

  JSONVar message;
  message["command"] = "drawing";

  this->publishMessage(this->fromDeviceTopic, message);
}

void PlotterController::requestLine() {
  this->waitingForLine = true;
  this->currentLine++;

  JSONVar message;
  message["command"] = "line";
  message["drawing"] = this->currentDrawing;
  message["line"] = this->currentLine;

  this->publishMessage(this->fromDeviceTopic, message);
}

void PlotterController::getAcceptedMessage(const String& message) {
  this->waitingForDeviceShadow = false;

  JSONVar json = JSON.parse(message);
  if (!json.hasOwnProperty("state")) {
    reportError("Missing getAccepted.state", message);
    return;
  }

  JSONVar state = json["state"];
  if (!state.hasOwnProperty("reported")) {
    reportError("Missing getAccepted.state.reported", message);
    return;
  }

  JSONVar reported = state["reported"];
  if (!reported.hasOwnProperty("drawing") || !reported.hasOwnProperty("totalLines")) {
    // This isn't an error, there just isn't any state we can load
    return;
  }

  String drawing = reported["drawing"];
  int totalLines = reported["totalLines"];

  this->currentDrawing = drawing;
  this->totalLines = totalLines;

  if (reported.hasOwnProperty("line")) {
    int line = reported["line"];
    this->currentLine = line;
  }

  if (reported.hasOwnProperty("radius") && reported.hasOwnProperty("azimuth")) {
    double radius = reported["radius"];
    double azimuth = reported["azimuth"];

    this->plotter.init(radius, azimuth);
  }
}

void PlotterController::getRejectedMessage(const String& message) {
  this->waitingForDeviceShadow = false;
  this->plotter.executeWipe();
}

void PlotterController::toDeviceMessage(const String& message) {
  JSONVar json = JSON.parse(message);
  bool handled = false;

  if (json.hasOwnProperty("type")) {
    String type = json["type"];

    if ((this->waitingForDrawing && type == "drawing" && json.hasOwnProperty("drawing") && json.hasOwnProperty("totalLines"))
        || (this->waitingForLine && type == "line" && json.hasOwnProperty("command"))) {
      this->waitingForDrawing = false;
      this->waitingForLine = false;

      if (json.hasOwnProperty("drawing") && json.hasOwnProperty("totalLines")) {
        String drawing = json["drawing"];
        int totalLines = json["totalLines"];

        this->currentDrawing = drawing;
        this->totalLines = totalLines;
        this->plotter.executeWipe();
        this->currentLine = 0;
      }

      if (json.hasOwnProperty("drawing") && json.hasOwnProperty("line")) {
        String drawing = json["drawing"];
        int line = json["line"];

        if (drawing != this->currentDrawing || line != this->currentLine) {
          String msg = "Drawing=" + this->currentDrawing + ", Line=" + this->currentLine + ", Message=" + message;
          this->reportError("Incorrect drawing or line", msg);
        }
      }

      if (json.hasOwnProperty("command")) {
        String command = json["command"];

        this->plotter.computeSteps(command);
        this->hasSteps = this->plotter.getStepCount() > 0;
      }

      this->reportState();
      handled = true;
    } else if (type == "drawing") {
      if (!this->waitingForDrawing) {
        reportError("Unexpected drawing message", message); handled = true;
      } else if (!json.hasOwnProperty("drawing")) {
        reportError("Missing drawing element in drawing message", message); handled = true;
      } else if (!json.hasOwnProperty("totalLines")) {
        reportError("Missing totalLines element in drawing message", message); handled = true;
      }
    } else if (type == "line") {
      if (!this->waitingForLine) {
        reportError("Unexpected line message", message); handled = true;
      } else if (!json.hasOwnProperty("command")) {
        reportError("Missing command element in line message", message); handled = true;
      }
    } else {
      reportError("Missing type", message); handled = true;
    }
  } else {
    reportError("Unknown type", message); handled = true;
  }

  if (!handled) {
    reportError("Unknown or invalid toDevice", message);
  }
}

void PlotterController::reportState() {
  Point position = this->plotter.getPosition();

  JSONVar reported;
  reported["drawing"] = this->currentDrawing;
  reported["totalLines"] = this->totalLines;
  reported["line"] = this->currentLine;
  reported["radius"] = position.getRadius();
  reported["azimuth"] = position.getAzimuth();

  JSONVar state;
  state["reported"] = reported;

  JSONVar msg;
  msg["state"] = state;

  this->publishMessage(this->updateTopic, msg);
}

void PlotterController::reportError(const String& type, const String& message) {
  JSONVar msg;
  msg["command"] = "error";
  msg["type"] = type;
  msg["message"] = message;
  msg["drawing"] = this->currentDrawing;
  msg["totalLines"] = this->totalLines;
  msg["line"] = this->currentLine;

  this->publishMessage(this->fromDeviceTopic, msg);
}

void PlotterController::simulateMessageReceived(const char topic[], const String& message) {
  const String tp = String(topic);
  this->messageReceived(topic, message);
}