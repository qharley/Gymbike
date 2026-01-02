#include "storage.h"
#include "control.h"
#include "config.h"
#include <Preferences.h>

extern float cadenceCurveWidth;
extern float cadenceCurveGain;

Preferences prefs;

void loadControlConfig() {
    prefs.begin("ctrl", true);
    kp = prefs.getFloat("kp", 1.2);
    ki = prefs.getFloat("ki", 0.02);
    kd = prefs.getFloat("kd", 0.0);
    targetCadence = prefs.getInt("cad", 80);
    setCadenceCurveWidth(prefs.getFloat("cadWidth", 10.0));
    setCadenceCurveGain(prefs.getFloat("cadGain", 2.0));
    prefs.end();
}

void saveControlConfig() {
    prefs.begin("ctrl", false);
    prefs.putFloat("kp", kp);
    prefs.putFloat("ki", ki);
    prefs.putFloat("kd", kd);
    prefs.putInt("cad", targetCadence);
    prefs.putFloat("cadWidth", getCadenceCurveWidth());
    prefs.putFloat("cadGain", getCadenceCurveGain());
    prefs.end();
}
