#pragma once
#include "config.h"

void controlInit();
void controlLoop();
int getServoPosition();
void setManualServo(int position);
void setTargetCadence(int cadence);
void resetPID();

extern int targetCadence;
extern int targetWatts;
extern int manualServo;
extern ControlMode controlMode;
extern float kp, ki, kd;
