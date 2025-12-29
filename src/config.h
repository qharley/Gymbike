#pragma once

// ===== DISPLAY =====
//#define RPI_DISPLAY_TYPE
//#define ILI9486_DRIVER
//#define RPI_ILI9486_DRIVER


// ===== TOUCH =====
#define TOUCH_CS 21
#define TOUCH_IRQ 22
#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST 4
#define TFT_BL 32
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_MISO 19

// ===== SERVO =====
#define SERVO_PIN 25
#define SERVO_MIN_ANGLE 30
#define SERVO_MAX_ANGLE 150

// ===== CADENCE SENSOR =====
#define CADENCE_PIN 33  // pin 33 is labelled as 

// ===== CONTROL =====
#define CONTROL_INTERVAL_MS 20   // 50 Hz

// ===== PID TUNING =====
#define KP 0.08
#define KI 0.01

// ===== RESISTANCE LIMITS =====
#define RESISTANCE_MIN 0
#define RESISTANCE_MAX 100
#define RESISTANCE_DEFAULT 20