#pragma once

// ===== SPI DISPLAY (Waveshare 3.5 LCD C) =====
#define TFT_CS   5
#define TFT_DC   21
#define TFT_RST  22

// ===== TOUCH =====
#define TOUCH_CS   4
#define TOUCH_IRQ 27

// ===== SERVO =====
#define SERVO_PIN 25
#define SERVO_MIN_ANGLE 30
#define SERVO_MAX_ANGLE 150

// ===== CADENCE SENSOR =====
#define CADENCE_PIN 33

// ===== CONTROL =====
#define CONTROL_INTERVAL_MS 20   // 50 Hz

// ===== PID TUNING =====
#define KP 0.08
#define KI 0.01

// ===== RESISTANCE LIMITS =====
#define RESISTANCE_MIN 0
#define RESISTANCE_MAX 100
#define RESISTANCE_DEFAULT 20