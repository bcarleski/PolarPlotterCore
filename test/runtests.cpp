#ifndef __IN_TEST__
#define __IN_TEST__
#endif
#include "plotterController.h"
#include "fakeStatus.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>

#define MAX_RADIUS 1000
#define MARBLE_SIZE_IN_RADIUS_STEPS 650
#define MAX_RADIUS_STEPS 10500
#define FULL_CIRCLE_AZIMUTH_STEPS 4810
//#define __SHOW_STEP_DETAILS__

const double maxRadius = MAX_RADIUS;
const double radiusStepSize = maxRadius / MAX_RADIUS_STEPS;
const double azimuthStepSize = (2 * PI) / FULL_CIRCLE_AZIMUTH_STEPS;

using namespace std;

int main(int argc, char **argv) {
    initialize_mock_arduino();

    Print print;
    StatusUpdater status;
    PlotterController plotter(print, status, MAX_RADIUS, MARBLE_SIZE_IN_RADIUS_STEPS);
    String drawing("TestDrawing");

    cout << "Initializing MAX_RADIUS: " << MAX_RADIUS << "\n";
    cout << "Initializing RADIUS_STEP_SIZE: " << setprecision(12) << radiusStepSize << "\n";
    cout << "Initializing AZIMUTH_STEP_SIZE: " << setprecision(12) << azimuthStepSize << "\n";
    cout << "Initializing MARBLE_SIZE_IN_RADIUS_STEPS: " << MARBLE_SIZE_IN_RADIUS_STEPS << "\n";
    cout << "Setting drawing: " << drawing.c_str() << "\n";
    plotter.calibrate(radiusStepSize, azimuthStepSize);
    plotter.newDrawing(drawing);
    for (int i = 1; i < argc; i++) {
        String command(argv[i]);
        cout << "Adding command: " << command.c_str() << "\n";
        plotter.addCommand(command);
    }

    cout << "Executing commands\n";
    while (plotter.canCycle()) {
        plotter.performCycle();
    }
}