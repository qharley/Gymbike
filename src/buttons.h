#pragma once
#include <Arduino.h>

// Button state enum
enum ButtonType {
    BTN_START_STOP = 0,
    BTN_REST = 1,
    BTN_RESET = 2
};

// Initialize button pins and interrupts
void buttonsInit();

// Check if a specific button was pressed (clears the flag)
bool buttonWasPressed(ButtonType button);

// Get the state of all buttons as a bitmask
uint8_t getButtonStates();

// Get raw button pin states (for troubleshooting)
bool getRawButtonState(ButtonType button);
