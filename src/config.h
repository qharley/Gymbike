#pragma once

#define AP_SSID_PREFIX "GymBike"
#define AP_PASSWORD    "gymbike123"

#define SERVO_PIN 25
#define SERVO_MIN 20
#define SERVO_MAX 160

#define CADENCE_PIN 34
#define CADENCE_TIMEOUT_MS 2000

enum ControlMode {
    MODE_MANUAL,
    MODE_CADENCE,
    MODE_ERG
};
