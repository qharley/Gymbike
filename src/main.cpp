#include <Arduino.h>
#include "config.h"
#include "cadence.h"
#include "servo_control.h"
#include "display.h"

void setup() {
    Serial.begin(115200);

    cadenceInit();
    servoInit();
    displayInit();

    Serial.println("Gym bike controller started");
}

void loop() {
    updateCadence();
    float rpm = getCadenceRPM();

    servoControlUpdate(rpm);
    displayUpdate(rpm);
}
