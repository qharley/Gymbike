/**
 * Button Test Program
 * 
 * This is a standalone test program to verify the three physical buttons work correctly.
 * To use this test:
 * 1. Temporarily rename main.cpp to main.cpp.bak
 * 2. Rename this file to main.cpp
 * 3. Build and upload
 * 4. Open Serial Monitor at 115200 baud
 * 5. Press each button and verify the output
 * 
 * Expected behavior:
 * - Pressing Start/Stop button should print "START/STOP BUTTON PRESSED"
 * - Pressing Rest button should print "REST BUTTON PRESSED"
 * - Pressing Reset button should print "RESET BUTTON PRESSED"
 */

#include <Arduino.h>
#include "buttons.h"

unsigned long lastTest = 0;
int startStopCount = 0;
int restCount = 0;
int resetCount = 0;

void setup() {
    Serial.begin(115200);
    delay(500);
    
    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("   GYMBIKE BUTTON TEST MODE");
    Serial.println("========================================");
    Serial.println();
    Serial.println("Button Pin Assignments:");
    Serial.println("  Start/Stop: GPIO 14");
    Serial.println("  Rest:       GPIO 12");
    Serial.println("  Reset:      GPIO 13");
    Serial.println();
    Serial.println("Initializing buttons...");
    
    buttonsInit();
    
    Serial.println("[OK] Buttons initialized successfully!");
    Serial.println();
    Serial.println("Press any button to test...");
    Serial.println("----------------------------------------");
    Serial.println();
}

void loop() {
    // Check for button presses
    if (buttonWasPressed(BTN_START_STOP)) {
        startStopCount++;
        Serial.print(">>> START/STOP BUTTON PRESSED (");
        Serial.print(startStopCount);
        Serial.println(" times)");
    }
    
    if (buttonWasPressed(BTN_REST)) {
        restCount++;
        Serial.print(">>> REST BUTTON PRESSED (");
        Serial.print(restCount);
        Serial.println(" times)");
    }
    
    if (buttonWasPressed(BTN_RESET)) {
        resetCount++;
        Serial.print(">>> RESET BUTTON PRESSED (");
        Serial.print(resetCount);
        Serial.println(" times)");
    }
    
    // Print summary every 10 seconds
    unsigned long now = millis();
    if (now - lastTest >= 10000) {
        lastTest = now;
        Serial.println();
        Serial.println("--- Test Summary ---");
        Serial.print("Start/Stop: "); Serial.print(startStopCount); Serial.println(" presses");
        Serial.print("Rest:       "); Serial.print(restCount); Serial.println(" presses");
        Serial.print("Reset:      "); Serial.print(resetCount); Serial.println(" presses");
        Serial.println("--------------------");
        Serial.println();
    }
    
    delay(10);
}
