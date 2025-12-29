#include "cadence.h"
#include "config.h"
#include <Arduino.h>

volatile unsigned long lastPulse = 0;
volatile unsigned long pulseInterval = 0;

void IRAM_ATTR cadenceISR() {
    unsigned long now = micros();
    pulseInterval = now - lastPulse;
    lastPulse = now;
}

void cadenceInit() {
    pinMode(CADENCE_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CADENCE_PIN), cadenceISR, FALLING);
}

int getCadenceRPM() {
    if (pulseInterval == 0) return 0;
    float rpm = 60000000.0 / pulseInterval;
    return rpm > 200 ? 0 : rpm;
}
