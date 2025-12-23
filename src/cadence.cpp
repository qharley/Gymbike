#include <Arduino.h>
#include "cadence.h"
#include "config.h"

volatile uint32_t lastPulseMicros = 0;
volatile uint32_t pulseIntervalMicros = 0;

static float cadenceFiltered = 0;

void IRAM_ATTR cadenceISR() {
    uint32_t now = micros();
    uint32_t interval = now - lastPulseMicros;

    // Reject impossible speeds (>300 RPM)
    if (interval > 20000) {
        pulseIntervalMicros = interval;
        lastPulseMicros = now;
    }
}

void cadenceInit() {
    pinMode(CADENCE_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CADENCE_PIN), cadenceISR, FALLING);
}

void updateCadence() {
    uint32_t interval;

    noInterrupts();
    interval = pulseIntervalMicros;
    interrupts();

    float rpm = 0;
    if (interval > 0 && micros() - lastPulseMicros < 2000000) {
        rpm = 60.0e6 / interval;
    }

    // Low-pass filter
    cadenceFiltered = cadenceFiltered * 0.8 + rpm * 0.2;
}

float getCadenceRPM() {
    return cadenceFiltered;
}
