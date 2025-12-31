#pragma once

#define AP_SSID_PREFIX "GymBike"
#define AP_PASSWORD    "gymbike123"

#define SERVO_PIN 25
#define SERVO_MIN 5
#define SERVO_MAX 175
#define EMERGENCY_PIN 26

// Set a flag to disable the emergency pin
//#define DISABLE_EMERGENCY_PIN


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

// TFT Display (480x320 SPI)
// Note: TFT_WIDTH and TFT_HEIGHT are defined in platformio.ini build flags
#ifndef TFT_WIDTH
#define TFT_WIDTH 480
#endif
#ifndef TFT_HEIGHT
#define TFT_HEIGHT 320
#endif
#define TFT_ROTATION 1  // Landscape mode

// SPI pins for display (use VSPI)
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_SCLK 18
#define TFT_CS 5
#define TFT_DC 17
#define TFT_RST 16
#define TFT_BL 4  // Backlight pin (optional)

// XPT2046 Touch Controller (shares SPI bus with TFT)
// Reserved for future development
#define TP_CS 15   // Touch controller chip select
#define TP_IRQ 2   // Touch controller interrupt

enum ControlMode {
    MODE_MANUAL,
    MODE_CADENCE,
    MODE_ERG
};
