#include "buttons.h"
#include "config.h"

// Button state tracking
volatile bool buttonFlags[3] = {false, false, false};
volatile unsigned long lastButtonPress[3] = {0, 0, 0};
const unsigned long BUTTON_DEBOUNCE_MS = 200;

// Button pin mapping
const uint8_t buttonPins[3] = {
    BTN_START_STOP_PIN,
    BTN_REST_PIN,
    BTN_RESET_PIN
};

// ISR handlers for each button
void IRAM_ATTR startStopButtonISR() {
    unsigned long now = millis();
    if (now - lastButtonPress[BTN_START_STOP] > BUTTON_DEBOUNCE_MS) {
        buttonFlags[BTN_START_STOP] = true;
        lastButtonPress[BTN_START_STOP] = now;
    }
}

void IRAM_ATTR restButtonISR() {
    unsigned long now = millis();
    if (now - lastButtonPress[BTN_REST] > BUTTON_DEBOUNCE_MS) {
        buttonFlags[BTN_REST] = true;
        lastButtonPress[BTN_REST] = now;
    }
}

void IRAM_ATTR resetButtonISR() {
    unsigned long now = millis();
    if (now - lastButtonPress[BTN_RESET] > BUTTON_DEBOUNCE_MS) {
        buttonFlags[BTN_RESET] = true;
        lastButtonPress[BTN_RESET] = now;
    }
}

void buttonsInit() {
    // Configure all button pins as INPUT_PULLUP
    pinMode(BTN_START_STOP_PIN, INPUT_PULLUP);
    pinMode(BTN_REST_PIN, INPUT_PULLUP);
    pinMode(BTN_RESET_PIN, INPUT_PULLUP);
    
    // Attach interrupts on FALLING edge (button press)
    attachInterrupt(digitalPinToInterrupt(BTN_START_STOP_PIN), startStopButtonISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_REST_PIN), restButtonISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_RESET_PIN), resetButtonISR, FALLING);
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
