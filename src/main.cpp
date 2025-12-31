#include <Arduino.h>
#include "wifi_manager.h"
#include "web_server.h"
#include "cadence.h"
#include "control.h"
#include "storage.h"
#include "captive_portal.h"
#include "ota.h"
#include "rotary_encoder.h"
#include "buttons.h"
#include "display.h"

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n\n=== GymBike Starting ===");
    Serial.println("Testing Physical Buttons:");
    Serial.println("  - Start/Stop: Pin 14");
    Serial.println("  - Rest: Pin 12");
    Serial.println("  - Reset: Pin 13");
    Serial.println();
    
    loadControlConfig();
    displayInit();
    Serial.println("[OK] Display initialized");
    wifiInit();
    cadenceInit();
    encoderInit();
    buttonsInit();
    Serial.println("[OK] Buttons initialized");
    controlInit();
    webServerInit();
    otaInit();
    Serial.println("[OK] System ready\n");
}

unsigned long lastStatusPrint = 0;
const unsigned long STATUS_INTERVAL = 5000; // Print status every 5 seconds
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_INTERVAL = 200; // Update display every 200ms

void loop() {
    captivePortalLoop();
    encoderLoop();
    controlLoop();
    
    unsigned long now = millis();
    
    // Update display
    if (now - lastDisplayUpdate >= DISPLAY_INTERVAL) {
        displayUpdate();
        lastDisplayUpdate = now;
    }
    
    // Periodic status output for testing
    if (now - lastStatusPrint >= STATUS_INTERVAL) {
        lastStatusPrint = now;
        
        extern WorkoutState workoutState;
        extern ControlMode controlMode;
        extern int targetCadence;
        
        Serial.print("[STATUS] Workout: ");
        switch(workoutState) {
            case WORKOUT_STOPPED: Serial.print("STOPPED"); break;
            case WORKOUT_RUNNING: Serial.print("RUNNING"); break;
            case WORKOUT_RESTING: Serial.print("RESTING"); break;
        }
        Serial.print(" | Mode: ");
        switch(controlMode) {
            case MODE_MANUAL: Serial.print("MANUAL"); break;
            case MODE_CADENCE: Serial.print("CADENCE"); break;
            case MODE_ERG: Serial.print("ERG"); break;
        }
        Serial.print(" | Target: ");
        Serial.print(targetCadence);
        Serial.print(" RPM | Servo: ");
        Serial.println(getServoPosition());
    }
    
    delay(20);
}

