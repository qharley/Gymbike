#include "control.h"
#include "cadence.h"
#include "rotary_encoder.h"
#include "buttons.h"
#include <ESP32Servo.h>

Servo brakeServo;

// Control targets
int targetCadence = 80;
int targetWatts = 150;
int manualServo = SERVO_MIN;
ControlMode controlMode = MODE_MANUAL;

// Workout state tracking
WorkoutState workoutState = WORKOUT_STOPPED;
unsigned long workoutStartTime = 0;
unsigned long workoutElapsedTime = 0;
unsigned long restStartTime = 0;

// PID parameters - tunable
float kp = 0.5;
float ki = 0.01;
float kd = 0.1;

// PID state
float integral = 0;
float lastError = 0;
int lastCadence = 0;

// Control loop timing
unsigned long lastControlTime = 0;
unsigned long lastCadenceTime = 0;
const unsigned long CONTROL_INTERVAL_MS = 100; // Run PID every 100ms

// Servo state
int servoPos = SERVO_MIN;
int targetServoPos = SERVO_MIN;

// Constants
const float INTEGRAL_MAX = 100.0;
const float INTEGRAL_MIN = -100.0;
const int CADENCE_DEADBAND = 2; // Don't adjust if within ±2 RPM
const int SERVO_RATE_LIMIT = 3; // Max servo change per control cycle

int getServoPosition() {
    return servoPos;
}

void setTargetCadence(int cadence) {
    targetCadence = constrain(cadence, 40, 120);
}

void resetPID() {
    integral = 0;
    lastError = 0;
    lastCadence = 0;
}

void setManualServo(int position) {
    position = constrain(position, SERVO_MIN, SERVO_MAX);
    manualServo = position;
    targetServoPos = position;
    
    if (controlMode == MODE_MANUAL) {
        servoPos = position;
        brakeServo.write(servoPos);
    }
}

void handleStartStop() {
    Serial.print("[BUTTON] Start/Stop pressed - ");
    if (workoutState == WORKOUT_STOPPED) {
        // Start workout
        workoutState = WORKOUT_RUNNING;
        workoutStartTime = millis();
        workoutElapsedTime = 0;
        resetPID();
        Serial.println("Starting workout");
    } else if (workoutState == WORKOUT_RUNNING) {
        // Stop workout
        workoutElapsedTime += millis() - workoutStartTime;
        workoutState = WORKOUT_STOPPED;
        // Release brake when stopping
        servoPos = SERVO_MIN;
        targetServoPos = SERVO_MIN;
        brakeServo.write(servoPos);
        resetPID();
        Serial.print("Stopping workout | Total time: ");
        Serial.print(workoutElapsedTime / 1000);
        Serial.println(" seconds");
    } else if (workoutState == WORKOUT_RESTING) {
        // Resume workout from rest
        workoutState = WORKOUT_RUNNING;
        workoutStartTime = millis();
        resetPID();
        Serial.println("Resuming from rest");
    }
}

void handleRest() {
    Serial.print("[BUTTON] Rest pressed - ");
    if (workoutState == WORKOUT_RUNNING) {
        // Pause for rest
        workoutElapsedTime += millis() - workoutStartTime;
        workoutState = WORKOUT_RESTING;
        restStartTime = millis();
        // Reduce brake during rest
        targetServoPos = SERVO_MIN;
        resetPID();
        Serial.println("Entering rest mode");
    } else if (workoutState == WORKOUT_RESTING) {
        // Resume from rest
        workoutState = WORKOUT_RUNNING;
        workoutStartTime = millis();
        resetPID();
        unsigned long restDuration = (millis() - restStartTime) / 1000;
        Serial.print("Resuming from rest | Rest duration: ");
        Serial.print(restDuration);
        Serial.println(" seconds");
    } else {
        Serial.println("Cannot rest (workout not running)");
    }
}

void handleReset() {
    Serial.println("[BUTTON] Reset pressed - Resetting all workout data");
    
    // Reset all workout tracking
    workoutState = WORKOUT_STOPPED;
    workoutElapsedTime = 0;
    workoutStartTime = 0;
    restStartTime = 0;
    
    // Reset servo to minimum position
    servoPos = SERVO_MIN;
    targetServoPos = SERVO_MIN;
    brakeServo.write(servoPos);
    
    // Reset PID
    resetPID();
    
    // Reset targets to defaults
    targetCadence = 80;
    targetWatts = 150;
    manualServo = SERVO_MIN;
    
    Serial.println("  -> Workout reset complete");
}

void controlInit() {
    brakeServo.attach(SERVO_PIN);
    pinMode(EMERGENCY_PIN, INPUT_PULLUP);
    
    servoPos = SERVO_MIN;
    targetServoPos = SERVO_MIN;
    brakeServo.write(servoPos);
    
    lastCadenceTime = millis();
    lastControlTime = millis();
    
    resetPID();
}

void controlLoop() {
    unsigned long now = millis();
    
    // Handle physical button presses
    if (buttonWasPressed(BTN_START_STOP)) {
        handleStartStop();
    }
    if (buttonWasPressed(BTN_REST)) {
        handleRest();
    }
    if (buttonWasPressed(BTN_RESET)) {
        handleReset();
    }
    
    // Handle rotary encoder input
    int encoderDelta = getEncoderDelta();
    if (encoderDelta != 0) {
        if (controlMode == MODE_MANUAL) {
            // In manual mode: adjust resistance
            // Each click = 2% resistance change
            int currentResistance = map(manualServo, SERVO_MIN, SERVO_MAX, 0, 100);
            int newResistance = constrain(currentResistance + (encoderDelta * 2), 0, 100);
            int newServo = map(newResistance, 0, 100, SERVO_MIN, SERVO_MAX);
            setManualServo(newServo);
        } else if (controlMode == MODE_CADENCE) {
            // In cadence mode: adjust target cadence
            // Each click = 2 RPM change
            targetCadence = constrain(targetCadence + (encoderDelta * 2), 40, 120);
            resetPID();  // Reset PID when target changes
        } else if (controlMode == MODE_ERG) {
            // In ERG mode: adjust target watts
            // Each click = 5 watt change
            targetWatts = constrain(targetWatts + (encoderDelta * 5), 50, 400);
        }
    }
    
    // Handle encoder button press - cycle through modes
    if (encoderButtonPressed()) {
        int nextMode = ((int)controlMode + 1) % 3;
        controlMode = (ControlMode)nextMode;
        
        if (controlMode == MODE_MANUAL) {
            manualServo = getServoPosition();
            resetPID();
        } else if (controlMode == MODE_CADENCE) {
            resetPID();
        }
    }
    
    // Emergency stop check
    if (digitalRead(EMERGENCY_PIN) == LOW) {
        servoPos = SERVO_MIN;
        targetServoPos = SERVO_MIN;
        brakeServo.write(servoPos);
        resetPID();
        return;
    }
    
    int currentCadence = getCadenceRPM();
    
    // Update last cadence time if we have movement
    if (currentCadence > 0) {
        lastCadenceTime = now;
    }
    
    // If workout is stopped or resting, release brake
    if (workoutState == WORKOUT_STOPPED || workoutState == WORKOUT_RESTING) {
        targetServoPos = SERVO_MIN;
        resetPID();
    }
    // Safety: Release brake if no cadence detected for timeout period
    else if (now - lastCadenceTime > CADENCE_TIMEOUT_MS) {
        targetServoPos = SERVO_MIN;
        integral = 0;
        lastError = 0;
    } else {
        // Normal operation based on mode
        switch (controlMode) {
            case MODE_MANUAL:
                // Manual mode: user directly controls servo position
                targetServoPos = manualServo;
                resetPID(); // Keep PID reset in manual mode
                break;
                
            case MODE_CADENCE:
                // Run PID control at fixed intervals
                if (now - lastControlTime >= CONTROL_INTERVAL_MS) {
                    lastControlTime = now;
                    
                    // REVERSED: error is positive when cadence is TOO HIGH (need more brake)
                    int error = currentCadence - targetCadence;
                    
                    // Apply deadband to prevent hunting
                    if (abs(error) <= CADENCE_DEADBAND) {
                        error = 0;
                    }
                    
                    // Update integral with anti-windup
                    integral += error * (CONTROL_INTERVAL_MS / 1000.0);
                    integral = constrain(integral, INTEGRAL_MIN, INTEGRAL_MAX);
                    
                    // Calculate derivative (with filtering)
                    float derivative = 0;
                    if (lastCadence > 0) { // Only calculate if we have previous data
                        derivative = (currentCadence - lastCadence) / (CONTROL_INTERVAL_MS / 1000.0);
                    }
                    
                    // PID output: positive output = increase brake
                    float output = (kp * error) + (ki * integral) + (kd * derivative);
                    
                    // Apply output to servo target
                    targetServoPos += (int)output;
                    targetServoPos = constrain(targetServoPos, SERVO_MIN, SERVO_MAX);
                    
                    // Store for next iteration
                    lastError = error;
                    lastCadence = currentCadence;
                }
                break;
                
            case MODE_ERG:
                // Power-based control: derive target resistance from target watts and
                // current cadence using the same linear model as estimatePower():
                //   Power = cadence × resistance_pct × NUMERATOR / DENOMINATOR
                //   → resistance_pct = Power × DENOMINATOR / (cadence × NUMERATOR)
                if (now - lastControlTime >= CONTROL_INTERVAL_MS) {
                    lastControlTime = now;
                    if (currentCadence > 0) {
                        long denom = (long)currentCadence * POWER_FACTOR_NUMERATOR;
                        int targetResistancePct = (int)(((long)targetWatts * POWER_FACTOR_DENOMINATOR) / denom);
                        targetResistancePct = constrain(targetResistancePct, 0, 100);
                        targetServoPos = map(targetResistancePct, 0, 100, SERVO_MIN, SERVO_MAX);
                    }
                    // If cadence is zero, hold current servo position until pedalling resumes
                }
                break;
        }
    }
    
    // Smooth servo movement with rate limiting
    if (servoPos < targetServoPos) {
        servoPos = min(servoPos + SERVO_RATE_LIMIT, targetServoPos);
    } else if (servoPos > targetServoPos) {
        servoPos = max(servoPos - SERVO_RATE_LIMIT, targetServoPos);
    }
    
    // Write to servo
    brakeServo.write(servoPos);
}
