#pragma once

// ===== TOUCH =====
#define TOUCH_CS   4
#define TOUCH_IRQ 27

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