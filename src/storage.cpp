#include "storage.h"
#include "control.h"
#include "config.h"
#include <Preferences.h>

Preferences prefs;

void loadControlConfig() {
    prefs.begin("ctrl", true);
    kp = prefs.getFloat("kp", 1.2);
    ki = prefs.getFloat("ki", 0.02);
    kd = prefs.getFloat("kd", 0.0);
    targetCadence = prefs.getInt("cad", 80);
    prefs.end();
}

void saveControlConfig() {
    prefs.begin("ctrl", false);
    prefs.putFloat("kp", kp);
    prefs.putFloat("ki", ki);
    prefs.putFloat("kd", kd);
    prefs.putInt("cad", targetCadence);
    prefs.end();
}
