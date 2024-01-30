#pragma once

typedef unsigned char byte;
#define PI          3.1415926535897932384626433832795


unsigned long millis();
void delay(unsigned long ms);

void initialize_mock_arduino(); 

#include "fakePrint.h"
#include "fakeStatus.h"
