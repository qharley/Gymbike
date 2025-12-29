#include "control.h"
#include "cadence.h"
#include <ESP32Servo.h>

Servo brakeServo;

int targetCadence = 80;
int targetWatts = 150;
int manualServo = 90;
ControlMode controlMode = MODE_CADENCE;

float kp = 1.2, ki = 0.02, kd = 0.0;
float integral = 0, lastError = 0;

int servoPos = 90;

void controlInit() {
    brakeServo.attach(SERVO_PIN);
    brakeServo.write(servoPos);
}

void controlLoop() {
    int cadence = getCadenceRPM();

    if (controlMode == MODE_MANUAL) {
        servoPos = manualServo;
    } else {
        int error = targetCadence - cadence;
        integral += error;
        float derivative = error - lastError;

        float output = kp * error + ki * integral + kd * derivative;
        servoPos += output;

        lastError = error;
    }

    servoPos = constrain(servoPos, SERVO_MIN, SERVO_MAX);
    brakeServo.write(servoPos);
}
