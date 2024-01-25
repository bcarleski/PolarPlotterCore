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

#ifndef _POLARPLOTTERCORE_PLOTTERCONTROLLER_H_
#define _POLARPLOTTERCORE_PLOTTERCONTROLLER_H_

#include "polarPlotter.h"
#include <Arduino_JSON.h>

#define TOPIC_SUBSCRIPTION_COUNT 3

class PlotterController {
  private:
    CondOut& condOut;
    PolarPlotter& plotter;
    const String getTopic;
    const String getAcceptedTopic;
    const String getRejectedTopic;
    const String updateTopic;
    const String toDeviceTopic;
    const String fromDeviceTopic;
    String topics[TOPIC_SUBSCRIPTION_COUNT];
    void (*publishMessage)(const String&, const JSONVar&);
    bool (*pollMqtt)(String[]);
    bool initialized;
    bool waitingForDeviceShadow;
    bool waitingForDrawing;
    bool waitingForLine;
    bool hasSteps;
    String currentDrawing;
    int currentLine;
    int totalLines;

    void requestDrawing();
    void requestLine();
    void getAcceptedMessage(const String&);
    void getRejectedMessage(const String&);
    void toDeviceMessage(const String&);
    void reportState();
    void reportError(const String&, const String&);

  public:
    PlotterController(CondOut& condOut, PolarPlotter&, const String&, const String&);
    void onMessage(void publishMessage(const String&, const JSONVar&));
    void onMqttPoll(bool pollMqtt(String[]));
    void performCycle();
    void messageReceived(const String&, const String&);
};

#endif
