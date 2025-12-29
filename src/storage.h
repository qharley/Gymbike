#pragma once
#include <Preferences.h>

extern Preferences prefs;

extern float kp, ki, kd;
extern float integral, lastError;

void loadControlConfig();
void saveControlConfig();

