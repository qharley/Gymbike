#pragma once

// Initialise the BLE FTMS (Fitness Machine Service) stack.
// Must be called once from setup() AFTER cadenceInit() and controlInit().
void bluetoothInit();

// Must be called every iteration of loop() to push Indoor Bike Data
// notifications to connected clients (e.g. Zwift).
void bluetoothLoop();

// Returns true when a BLE central (e.g. Zwift) is connected.
bool bluetoothConnected();

// Estimate power in watts from cadence (RPM) and resistance (0-100 %).
// Uses the linear model defined by POWER_FACTOR_NUMERATOR / POWER_FACTOR_DENOMINATOR
// in config.h.  Exposed so that control.cpp can share the same formula.
int estimatePower(int cadenceRPM, int resistancePct);
