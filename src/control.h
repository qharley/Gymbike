#pragma once
#include "config.h"

void controlInit();
void controlLoop();

extern int targetCadence;
extern int targetWatts;
extern int manualServo;
extern ControlMode controlMode;
