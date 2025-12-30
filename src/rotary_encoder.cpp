#include "rotary_encoder.h"
#include "config.h"
#include <Arduino.h>

// Encoder state
volatile int encoderPos = 0;
volatile int lastEncoderPos = 0;
volatile unsigned long lastEncoderTime = 0;

// Button state
volatile bool buttonPressed = false;
volatile unsigned long lastButtonTime = 0;
const unsigned long BUTTON_DEBOUNCE_MS = 200;

// Encoder debouncing
const unsigned long ENCODER_DEBOUNCE_MS = 5;

// Last stable states
int lastCLK = HIGH;
int lastDT = HIGH;

void IRAM_ATTR encoderISR() {
    unsigned long now = millis();
    
    // Debounce
    if (now - lastEncoderTime < ENCODER_DEBOUNCE_MS) {
        return;
    }
    
    int clk = digitalRead(ENCODER_CLK_PIN);
    int dt = digitalRead(ENCODER_DT_PIN);
    
    // Check for valid transition
    if (clk != lastCLK) {
        if (dt != clk) {
            encoderPos++;  // Clockwise
        } else {
            encoderPos--;  // Counter-clockwise
        }
        lastEncoderTime = now;
    }
    
    lastCLK = clk;
    lastDT = dt;
}

void IRAM_ATTR buttonISR() {
    unsigned long now = millis();
    if (now - lastButtonTime > BUTTON_DEBOUNCE_MS) {
        buttonPressed = true;
        lastButtonTime = now;
    }
}

void encoderInit() {
    pinMode(ENCODER_CLK_PIN, INPUT_PULLUP);
    pinMode(ENCODER_DT_PIN, INPUT_PULLUP);
    pinMode(ENCODER_SW_PIN, INPUT_PULLUP);
    
    // Read initial states
    lastCLK = digitalRead(ENCODER_CLK_PIN);
    lastDT = digitalRead(ENCODER_DT_PIN);
    
    // Attach interrupts
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK_PIN), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_SW_PIN), buttonISR, FALLING);
}

void encoderLoop() {
    // Nothing needed here - interrupts handle everything
}

int getEncoderDelta() {
    int delta = encoderPos - lastEncoderPos;
    lastEncoderPos = encoderPos;
    return delta;
}

bool encoderButtonPressed() {
    if (buttonPressed) {
        buttonPressed = false;
        return true;
    }
    return false;
}
