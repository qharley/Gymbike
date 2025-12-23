#include <Arduino.h>
#include <ESP32Servo.h>
#include "servo_control.h"
#include "config.h"

static Servo resistanceServo;

static float resistance = 30;
static float integral = 0;
static int targetRPM = 90;

static unsigned long lastControl = 0;

void servoInit() {
    resistanceServo.attach(SERVO_PIN, 500, 2500);
    resistanceServo.write(SERVO_MIN_ANGLE);
}

void servoControlUpdate(float cadenceRPM) {
    if (millis() - lastControl < CONTROL_INTERVAL_MS) return;
    lastControl = millis();

    // Drop resistance if rider stops
    if (cadenceRPM < 5) {
        resistance = RESISTANCE_MIN;
        integral = 0;
    } else {
        float error = targetRPM - cadenceRPM;

        integral += error;
        integral = constrain(integral, -50, 50);

        float output = KP * error + KI * integral;

        resistance += output;
        resistance = constrain(resistance, RESISTANCE_MIN, RESISTANCE_MAX);
    }

    int angle = map(resistance, 0, 100, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    resistanceServo.write(angle);
}
