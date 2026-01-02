#pragma once
#include "config.h"

// Workout state enum
enum WorkoutState {
    WORKOUT_STOPPED,
    WORKOUT_RUNNING,
    WORKOUT_RESTING
};

void controlInit();
void controlLoop();
int getServoPosition();
void setManualServo(int position);
void setTargetCadence(int cadence);
void resetPID();
void resetCadenceTimer();

// Button action handlers
void handleStartStop();
void handleRest();
void handleReset();

// Workout state
extern WorkoutState workoutState;
extern unsigned long workoutStartTime;
extern unsigned long workoutElapsedTime;
extern unsigned long restStartTime;

extern int targetCadence;
extern int targetWatts;
extern int manualServo;
extern ControlMode controlMode;
extern float kp, ki, kd;
extern float cadenceCurveWidth;
extern float cadenceCurveGain;

// Cadence curve parameter setters/getters
void setCadenceCurveWidth(float width);
float getCadenceCurveWidth();
void setCadenceCurveGain(float gain);
float getCadenceCurveGain();
