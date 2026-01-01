#include "buttons.h"
#include "config.h"

// Button state tracking
volatile bool buttonFlags[3] = {false, false, false};
volatile unsigned long lastButtonPress[3] = {0, 0, 0};
volatile bool lastButtonState[3] = {HIGH, HIGH, HIGH};
const unsigned long BUTTON_DEBOUNCE_MS = 50;  // Hardware debounce time
const unsigned long BUTTON_LOCKOUT_MS = 300;   // Prevent double-trigger

// Button pin mapping
const uint8_t buttonPins[3] = {
    BTN_START_STOP_PIN,
    BTN_REST_PIN,
    BTN_RESET_PIN
};

// ISR handlers for each button
void IRAM_ATTR startStopButtonISR() {
    unsigned long now = millis();
    bool currentState = digitalRead(BTN_START_STOP_PIN);
    
    // Check if enough time has passed and button is actually pressed (LOW)
    if (now - lastButtonPress[BTN_START_STOP] > BUTTON_LOCKOUT_MS && 
        currentState == LOW && 
        lastButtonState[BTN_START_STOP] == HIGH) {
        buttonFlags[BTN_START_STOP] = true;
        lastButtonPress[BTN_START_STOP] = now;
    }
    lastButtonState[BTN_START_STOP] = currentState;
}

void IRAM_ATTR restButtonISR() {
    unsigned long now = millis();
    bool currentState = digitalRead(BTN_REST_PIN);
    
    // Check if enough time has passed and button is actually pressed (LOW)
    if (now - lastButtonPress[BTN_REST] > BUTTON_LOCKOUT_MS && 
        currentState == LOW && 
        lastButtonState[BTN_REST] == HIGH) {
        buttonFlags[BTN_REST] = true;
        lastButtonPress[BTN_REST] = now;
    }
    lastButtonState[BTN_REST] = currentState;
}

void IRAM_ATTR resetButtonISR() {
    unsigned long now = millis();
    bool currentState = digitalRead(BTN_RESET_PIN);
    
    // Check if enough time has passed and button is actually pressed (LOW)
    if (now - lastButtonPress[BTN_RESET] > BUTTON_LOCKOUT_MS && 
        currentState == LOW && 
        lastButtonState[BTN_RESET] == HIGH) {
        buttonFlags[BTN_RESET] = true;
        lastButtonPress[BTN_RESET] = now;
    }
    lastButtonState[BTN_RESET] = currentState;
}

void buttonsInit() {
    // Configure all button pins as INPUT_PULLUP
    pinMode(BTN_START_STOP_PIN, INPUT_PULLUP);
    pinMode(BTN_REST_PIN, INPUT_PULLUP);
    pinMode(BTN_RESET_PIN, INPUT_PULLUP);
    
    // Initialize button states
    lastButtonState[BTN_START_STOP] = digitalRead(BTN_START_STOP_PIN);
    lastButtonState[BTN_REST] = digitalRead(BTN_REST_PIN);
    lastButtonState[BTN_RESET] = digitalRead(BTN_RESET_PIN);
    
    // Small delay to let pins stabilize
    delay(50);
    
    // Attach interrupts on CHANGE (both press and release) for better debouncing
    attachInterrupt(digitalPinToInterrupt(BTN_START_STOP_PIN), startStopButtonISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_REST_PIN), restButtonISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_RESET_PIN), resetButtonISR, CHANGE);
}

bool buttonWasPressed(ButtonType button) {
    if (button < 0 || button > 2) {
        return false;
    }
    
    if (buttonFlags[button]) {
        buttonFlags[button] = false;
        return true;
    }
    return false;
}

uint8_t getButtonStates() {
    uint8_t states = 0;
    for (int i = 0; i < 3; i++) {
        if (buttonFlags[i]) {
            states |= (1 << i);
        }
    }
    return states;
}
