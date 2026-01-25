#include "storage.h"
#include "control.h"
#include "config.h"
#include <Preferences.h>

extern float cadenceCurveWidth;
extern float cadenceCurveGain;

Preferences prefs;
int displayMargin = 5; // Default 5px margin for kiosk display

void loadControlConfig() {
    prefs.begin("ctrl", true);
    kp = prefs.getFloat("kp", 1.2);
    ki = prefs.getFloat("ki", 0.02);
    kd = prefs.getFloat("kd", 0.0);
    targetCadence = prefs.getInt("cad", 80);
    setCadenceCurveWidth(prefs.getFloat("cadWidth", 10.0));
    setCadenceCurveGain(prefs.getFloat("cadGain", 2.0));
    prefs.end();
    
    Serial.println("[LOAD] Control Config:");
    Serial.printf("  PID: kp=%.2f, ki=%.3f, kd=%.2f\n", kp, ki, kd);
    Serial.printf("  Target Cadence: %d RPM\n", targetCadence);
    Serial.printf("  Curve: width=%.1f, gain=%.1f\n", getCadenceCurveWidth(), getCadenceCurveGain());
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

void loadDisplayConfig() {
    prefs.begin("display", true);
    displayMargin = prefs.getInt("margin", 5); // Default 5px
    prefs.end();
    
    Serial.printf("[LOAD] Display margin: %d px\n", displayMargin);
}

void saveDisplayConfig() {
    prefs.begin("display", false);
    prefs.putInt("margin", displayMargin);
    prefs.end();
}
