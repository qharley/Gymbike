#include "cadence.h"
#include "config.h"
#include <Arduino.h>

volatile unsigned long lastPulse = 0;
volatile unsigned long pulseInterval = 0;
volatile bool newPulseDetected = false;

void IRAM_ATTR cadenceISR() {
    unsigned long now = micros();
    unsigned long interval = now - lastPulse;
    
    // Debounce: ignore pulses shorter than 150ms (400 RPM max)
    if (interval > 150000) {
        pulseInterval = interval;
        lastPulse = now;
        newPulseDetected = true;
    }
}

void cadenceInit() {
    pinMode(CADENCE_PIN, INPUT_PULLUP);
    // Use RISING edge to catch the end of the short LOW pulse
    attachInterrupt(digitalPinToInterrupt(CADENCE_PIN), cadenceISR, RISING);
    lastPulse = micros();
}

int getCadenceRPM() {
    // Check if no pulse detected recently (timeout)
    unsigned long timeSinceLastPulse = micros() - lastPulse;
    
    // If more than 2 seconds since last pulse, return 0
    if (timeSinceLastPulse > CADENCE_TIMEOUT_MS * 1000) {
        return 0;
    }
    
    // If we haven't detected any pulses yet
    if (pulseInterval == 0) {
        return 0;
    }
    
    // Calculate RPM: 60 million microseconds per minute / interval
    float rpm = 60000000.0 / pulseInterval;
    
    // Sanity check: valid cadence range 20-250 RPM
    if (rpm < 20 || rpm > 250) {
        return 0;
    }
    
    return (int)rpm;
}

unsigned long getLastCadencePulseTime() {
    return lastPulse;
}

bool getRawCadencePin() {
    return digitalRead(CADENCE_PIN);
}
