#pragma once

#define AP_SSID_PREFIX "GymBike"
#define AP_PASSWORD    "gymbike123"

#define SERVO_PIN 25
#define SERVO_MIN 5
#define SERVO_MAX 175
#define EMERGENCY_PIN 26


#define CADENCE_PIN 34
#define CADENCE_TIMEOUT_MS 2000

// Rotary encoder pins
#define ENCODER_CLK_PIN 32
#define ENCODER_DT_PIN 33
#define ENCODER_SW_PIN 27  // Optional button press

// Physical button pins
#define BTN_START_STOP_PIN 14
#define BTN_REST_PIN 12
#define BTN_RESET_PIN 13

enum ControlMode {
    MODE_MANUAL,
    MODE_CADENCE,
    MODE_ERG
};
