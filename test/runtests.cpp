#ifndef __IN_TEST__
#define __IN_TEST__
#endif
#include "plotterController.h"
#include "fakeStatus.h"
#include <iostream>
#include <stdlib.h>

#define RADIUS_STEPPER_STEPS_PER_ROTATION 1600
#define RADIUS_GEAR_RATIO 1.0                  // # of drive gear teeth / # of motor gear teeth, i.e. if the drive shaft gear has 20 teeth, and the motor has 10, this should be 2.0
#define RADIUS_ROTATIONS_TO_MAX_RADIUS 1.0     // the number of complete turns of the drive gear that it takes to go from the center to the maximum radius

#define AZIMUTH_STEPPER_STEPS_PER_ROTATION 1600
#define AZIMUTH_GEAR_RATIO 1.0                 // # of drive gear teeth / # of motor gear teeth, i.e. if the drive shaft gear has 20 teeth, and the motor has 10, this should be 2.0

#define MAX_RADIUS 600
#define RADIUS_STEP_SIZE MAX_RADIUS / RADIUS_ROTATIONS_TO_MAX_RADIUS / RADIUS_STEPPER_STEPS_PER_ROTATION / RADIUS_GEAR_RATIO
#define AZIMUTH_STEP_SIZE 2 * PI / AZIMUTH_STEPPER_STEPS_PER_ROTATION / AZIMUTH_GEAR_RATIO
#define MARBLE_SIZE_IN_RADIUS_STEPS 80

using namespace std;

void run_tests(Print &print, Point &start, Point &finish, LineStepCalculator &calc, int debug) {
    StepBank steps(print);

    if (debug > 0) {
        cout << "Setting debug level to " << debug << "\n";
        steps.setDebug(debug);
        calc.setDebug(debug);
    }
    calc.addLineSteps(start, finish, steps);

    cout << "Got " << steps.getStepCount() << " steps\n";
    if (debug >= 3) {
        for (int i = 0; i < steps.getStepCount(); i++) {
            Step s = steps.getStep(i);
            cout << "Step (";
            cout << s.getRadiusStep();
            cout << ", ";
            cout << s.getAzimuthStep();
            cout << ")\n";
        }
    }
}

int main(int argc, char **argv) {
    initialize_mock_arduino();

    Print print;
    StatusUpdater status;
    PolarPlotter plotter(print, status, MAX_RADIUS, RADIUS_STEP_SIZE, AZIMUTH_STEP_SIZE, MARBLE_SIZE_IN_RADIUS_STEPS);
    PlotterController controller(print, status, plotter);
    String drawing("TestDrawing");

    cout << "Initializing MAX_RADIUS: " << MAX_RADIUS << "\n";
    cout << "Initializing RADIUS_STEP_SIZE: " << RADIUS_STEP_SIZE << "\n";
    cout << "Initializing AZIMUTH_STEP_SIZE: " << AZIMUTH_STEP_SIZE << "\n";
    cout << "Initializing MARBLE_SIZE_IN_RADIUS_STEPS: " << MARBLE_SIZE_IN_RADIUS_STEPS << "\n";
    cout << "Setting drawing: " << drawing.c_str() << "\n";
    controller.newDrawing(drawing);
    for (int i = 1; i < argc; i++) {
        String command(argv[i]);
        cout << "Adding command: " << command.c_str() << "\n";
        controller.addCommand(command);
    }

    cout << "Executing commands\n";
    while (controller.canCycle()) {
        controller.performCycle();
    }
}