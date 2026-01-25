#pragma once
#include <Preferences.h>

extern Preferences prefs;

extern float kp, ki, kd;
extern float integral, lastError;
extern int displayMargin; // User-configurable margin in pixels for kiosk display

void loadControlConfig();
void saveControlConfig();
void loadDisplayConfig();
void saveDisplayConfig();

