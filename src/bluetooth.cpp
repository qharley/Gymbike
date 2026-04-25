/*
 * bluetooth.cpp – Zwift-compatible BLE FTMS (Fitness Machine Service) for GymBike.
 *
 * Implements the Bluetooth SIG Fitness Machine Service (0x1826) so that Zwift (and
 * other compatible apps) can:
 *   • Discover the device and read its capabilities.
 *   • Receive live Indoor Bike Data (cadence, speed, estimated power) once per second.
 *   • Send control commands: Set Resistance Level, Set Target Power, Start/Stop.
 *
 * FTMS UUIDs are Bluetooth SIG-assigned 16-bit UUIDs expanded to 128-bit form.
 */

#include "bluetooth.h"
#include "cadence.h"
#include "control.h"
#include "config.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ---------------------------------------------------------------------------
// FTMS service / characteristic UUIDs
// ---------------------------------------------------------------------------
#define FTMS_SERVICE_UUID                     "00001826-0000-1000-8000-00805f9b34fb"
#define FITNESS_MACHINE_FEATURE_UUID          "00002acc-0000-1000-8000-00805f9b34fb"
#define INDOOR_BIKE_DATA_UUID                 "00002ad2-0000-1000-8000-00805f9b34fb"
#define TRAINING_STATUS_UUID                  "00002ad3-0000-1000-8000-00805f9b34fb"
#define SUPPORTED_RESISTANCE_LEVEL_RANGE_UUID "00002ad6-0000-1000-8000-00805f9b34fb"
#define SUPPORTED_POWER_RANGE_UUID            "00002ad8-0000-1000-8000-00805f9b34fb"
#define FITNESS_MACHINE_CONTROL_POINT_UUID    "00002ad9-0000-1000-8000-00805f9b34fb"
#define FITNESS_MACHINE_STATUS_UUID           "00002ada-0000-1000-8000-00805f9b34fb"

// ---------------------------------------------------------------------------
// FTMS Control Point op-codes (client → server)
// ---------------------------------------------------------------------------
#define FTMS_OP_REQUEST_CONTROL  0x00
#define FTMS_OP_RESET            0x01
#define FTMS_OP_SET_RESISTANCE   0x04  // param: sint16, 0.1 units
#define FTMS_OP_SET_POWER        0x05  // param: sint16, watts
#define FTMS_OP_START_RESUME     0x07
#define FTMS_OP_STOP_PAUSE       0x08

// ---------------------------------------------------------------------------
// FTMS Control Point response / result codes
// ---------------------------------------------------------------------------
#define FTMS_RESPONSE_CODE       0x80
#define FTMS_RESULT_SUCCESS      0x01
#define FTMS_RESULT_NOT_SUPPORTED 0x02
#define FTMS_RESULT_INVALID_PARAM 0x03
#define FTMS_RESULT_NO_CONTROL   0x05

// ---------------------------------------------------------------------------
// FTMS Status characteristic op-codes (server → client)
// ---------------------------------------------------------------------------
#define FTMS_STATUS_RESET                     0x01
#define FTMS_STATUS_STOPPED                   0x02
#define FTMS_STATUS_STARTED                   0x04
#define FTMS_STATUS_TARGET_RESISTANCE_CHANGED 0x07
#define FTMS_STATUS_TARGET_POWER_CHANGED      0x08

// ---------------------------------------------------------------------------
// Module-level state
// ---------------------------------------------------------------------------
static BLEServer*         pServer       = nullptr;
static BLECharacteristic* pBikeData     = nullptr;
static BLECharacteristic* pCtrlPoint    = nullptr;
static BLECharacteristic* pStatus       = nullptr;
static BLECharacteristic* pTrainStatus  = nullptr;

static volatile bool deviceConnected = false;
static volatile bool hasControl      = false;

static unsigned long lastNotifyTime = 0;
static const unsigned long NOTIFY_INTERVAL_MS = 1000;

// Speed estimate coefficient: ~0.4 km/h per RPM on a typical gym bike
// (52T chainring / 16T cog, 2.1 m wheel circumference ≈ 0.41 km/h per RPM).
// Stored in 0.01 km/h units, so multiply cadence by 40.
static const int SPEED_COEFF_PER_RPM_HUNDREDTHS = 40;

// ---------------------------------------------------------------------------
// Internal helper: read a little-endian int16 from a byte buffer
// ---------------------------------------------------------------------------
static inline int16_t readLeInt16(const uint8_t* buf) {
    return (int16_t)((uint16_t)buf[0] | ((uint16_t)buf[1] << 8));
}

// ---------------------------------------------------------------------------
// Public helper: power estimation
// ---------------------------------------------------------------------------
int estimatePower(int cadenceRPM, int resistancePct) {
    if (cadenceRPM <= 0 || resistancePct <= 0) return 0;
    return (cadenceRPM * resistancePct * POWER_FACTOR_NUMERATOR) / POWER_FACTOR_DENOMINATOR;
}

// ---------------------------------------------------------------------------
// Static helper: send a Fitness Machine Status notification
// ---------------------------------------------------------------------------
static void sendFTMSStatus(uint8_t statusOpCode, const uint8_t* params, size_t paramLen) {
    if (!pStatus || !deviceConnected) return;

    uint8_t buf[8];
    buf[0] = statusOpCode;
    size_t copyLen = (paramLen < 7) ? paramLen : 7;
    if (copyLen > 0 && params != nullptr) {
        memcpy(buf + 1, params, copyLen);
    }

    pStatus->setValue(buf, 1 + copyLen);
    pStatus->notify();
}

// ---------------------------------------------------------------------------
// BLE server connection callbacks
// ---------------------------------------------------------------------------
class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer*) override {
        deviceConnected = true;
        hasControl      = false;
        Serial.println("[BLE] Client connected");
    }

    void onDisconnect(BLEServer*) override {
        deviceConnected = false;
        hasControl      = false;
        Serial.println("[BLE] Client disconnected – restarting advertising");
        BLEDevice::startAdvertising();
    }
};

// ---------------------------------------------------------------------------
// FTMS Control Point callbacks (commands from Zwift)
// ---------------------------------------------------------------------------
class ControlPointCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override {
        uint8_t* data = pChar->getData();
        size_t   len  = pChar->getLength();
        if (len == 0) return;

        uint8_t opCode = data[0];
        uint8_t response[3] = { FTMS_RESPONSE_CODE, opCode, FTMS_RESULT_SUCCESS };

        switch (opCode) {

            // ---- 0x00  Request Control ----------------------------------------
            case FTMS_OP_REQUEST_CONTROL:
                hasControl = true;
                Serial.println("[BLE] FTMS control granted");
                break;

            // ---- 0x01  Reset ---------------------------------------------------
            case FTMS_OP_RESET:
                if (!hasControl) { response[2] = FTMS_RESULT_NO_CONTROL; break; }
                handleReset();
                sendFTMSStatus(FTMS_STATUS_RESET, nullptr, 0);
                Serial.println("[BLE] Reset");
                break;

            // ---- 0x04  Set Target Resistance Level ----------------------------
            //  Parameter: sint16, units of 0.1 (so 1000 = 100 %)
            case FTMS_OP_SET_RESISTANCE: {
                if (!hasControl)  { response[2] = FTMS_RESULT_NO_CONTROL;   break; }
                if (len < 3)      { response[2] = FTMS_RESULT_INVALID_PARAM; break; }

                int16_t raw = readLeInt16(data + 1);
                // raw is in 0.1 units; clamp to [0, 1000] → [0 %, 100 %]
                int resistancePct = constrain((int)raw / 10, 0, 100);
                int newServo = map(resistancePct, 0, 100, SERVO_MIN, SERVO_MAX);

                controlMode = MODE_MANUAL;
                setManualServo(newServo);

                Serial.print("[BLE] Set resistance: ");
                Serial.print(resistancePct);
                Serial.println("%");

                uint8_t statusParams[2] = {
                    (uint8_t)((uint16_t)raw & 0xFF),
                    (uint8_t)((uint16_t)raw >> 8)
                };
                sendFTMSStatus(FTMS_STATUS_TARGET_RESISTANCE_CHANGED, statusParams, 2);
                break;
            }

            // ---- 0x05  Set Target Power (ERG mode) ----------------------------
            //  Parameter: sint16, watts
            case FTMS_OP_SET_POWER: {
                if (!hasControl)  { response[2] = FTMS_RESULT_NO_CONTROL;   break; }
                if (len < 3)      { response[2] = FTMS_RESULT_INVALID_PARAM; break; }

                int16_t watts = readLeInt16(data + 1);
                targetWatts = constrain((int)watts, 0, 400);
                controlMode = MODE_ERG;

                Serial.print("[BLE] Set target power: ");
                Serial.print(targetWatts);
                Serial.println(" W");

                uint8_t statusParams[2] = {
                    (uint8_t)((uint16_t)targetWatts & 0xFF),
                    (uint8_t)((uint16_t)targetWatts >> 8)
                };
                sendFTMSStatus(FTMS_STATUS_TARGET_POWER_CHANGED, statusParams, 2);
                break;
            }

            // ---- 0x07  Start or Resume ----------------------------------------
            case FTMS_OP_START_RESUME:
                if (!hasControl) { response[2] = FTMS_RESULT_NO_CONTROL; break; }
                if (workoutState != WORKOUT_RUNNING) handleStartStop();
                sendFTMSStatus(FTMS_STATUS_STARTED, nullptr, 0);
                Serial.println("[BLE] Start/Resume");
                break;

            // ---- 0x08  Stop or Pause ------------------------------------------
            case FTMS_OP_STOP_PAUSE:
                if (!hasControl) { response[2] = FTMS_RESULT_NO_CONTROL; break; }
                if (workoutState == WORKOUT_RUNNING) handleStartStop();
                sendFTMSStatus(FTMS_STATUS_STOPPED, nullptr, 0);
                Serial.println("[BLE] Stop/Pause");
                break;

            default:
                response[2] = FTMS_RESULT_NOT_SUPPORTED;
                Serial.print("[BLE] Unsupported op-code: 0x");
                Serial.println(opCode, HEX);
                break;
        }

        // Send indication response back to the client
        pCtrlPoint->setValue(response, 3);
        pCtrlPoint->indicate();
    }
};

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void bluetoothInit() {
    BLEDevice::init(BLE_DEVICE_NAME);

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Create FTMS service (30 handles covers all characteristics + descriptors)
    BLEService* pService = pServer->createService(BLEUUID(FTMS_SERVICE_UUID), 30);

    // --- Fitness Machine Feature (read) ---
    // Fitness Machine Features (4 bytes):
    //   Bit 1 = Cadence Supported, Bit 6 = Resistance Level Supported,
    //   Bit 7 = Power Measurement Supported  → 0x000000C2
    // Target Setting Features (4 bytes):
    //   Bit 2 = Resistance Level Target Setting Supported,
    //   Bit 4 = Power Target Setting Supported  → 0x00000014
    BLECharacteristic* pFeature = pService->createCharacteristic(
        BLEUUID(FITNESS_MACHINE_FEATURE_UUID),
        BLECharacteristic::PROPERTY_READ
    );
    uint8_t features[8] = { 0xC2, 0x00, 0x00, 0x00,   // machine features
                             0x14, 0x00, 0x00, 0x00 }; // target setting features
    pFeature->setValue(features, 8);

    // --- Indoor Bike Data (notify) ---
    pBikeData = pService->createCharacteristic(
        BLEUUID(INDOOR_BIKE_DATA_UUID),
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pBikeData->addDescriptor(new BLE2902());

    // --- Training Status (read + notify) ---
    pTrainStatus = pService->createCharacteristic(
        BLEUUID(TRAINING_STATUS_UUID),
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pTrainStatus->addDescriptor(new BLE2902());
    // Flags=0x00 (no string), Status=0x01 (Idle)
    uint8_t trainStatus[2] = { 0x00, 0x01 };
    pTrainStatus->setValue(trainStatus, 2);

    // --- Supported Resistance Level Range (read) ---
    // sint16 min=0, sint16 max=1000 (=100.0 in 0.1 units), uint16 increment=10 (=1.0 %)
    BLECharacteristic* pResRange = pService->createCharacteristic(
        BLEUUID(SUPPORTED_RESISTANCE_LEVEL_RANGE_UUID),
        BLECharacteristic::PROPERTY_READ
    );
    uint8_t resRange[6] = { 0x00, 0x00,   // min  =   0
                             0xE8, 0x03,   // max  = 1000
                             0x0A, 0x00 }; // incr =  10
    pResRange->setValue(resRange, 6);

    // --- Supported Power Range (read) ---
    // sint16 min=0 W, sint16 max=400 W, uint16 increment=1 W
    BLECharacteristic* pPwrRange = pService->createCharacteristic(
        BLEUUID(SUPPORTED_POWER_RANGE_UUID),
        BLECharacteristic::PROPERTY_READ
    );
    uint8_t pwrRange[6] = { 0x00, 0x00,   // min  =   0 W
                             0x90, 0x01,   // max  = 400 W
                             0x01, 0x00 }; // incr =   1 W
    pPwrRange->setValue(pwrRange, 6);

    // --- Fitness Machine Control Point (write + indicate) ---
    pCtrlPoint = pService->createCharacteristic(
        BLEUUID(FITNESS_MACHINE_CONTROL_POINT_UUID),
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_INDICATE
    );
    pCtrlPoint->addDescriptor(new BLE2902());
    pCtrlPoint->setCallbacks(new ControlPointCallbacks());

    // --- Fitness Machine Status (notify) ---
    pStatus = pService->createCharacteristic(
        BLEUUID(FITNESS_MACHINE_STATUS_UUID),
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pStatus->addDescriptor(new BLE2902());
    // Initial status: Stopped
    uint8_t initialStatus[1] = { FTMS_STATUS_STOPPED };
    pStatus->setValue(initialStatus, 1);

    pService->start();

    // Advertising – include the FTMS service UUID so Zwift can discover the device
    BLEAdvertising* pAdv = BLEDevice::getAdvertising();
    pAdv->addServiceUUID(BLEUUID(FTMS_SERVICE_UUID));
    pAdv->setScanResponse(true);
    pAdv->setMinPreferred(0x06);
    pAdv->setMaxPreferred(0x12);
    BLEDevice::startAdvertising();

    Serial.println("[BLE] FTMS advertising started");
    Serial.print("[BLE] Device name: ");
    Serial.println(BLE_DEVICE_NAME);
}

void bluetoothLoop() {
    if (!deviceConnected) return;

    unsigned long now = millis();
    if (now - lastNotifyTime < NOTIFY_INTERVAL_MS) return;
    lastNotifyTime = now;

    int cadence    = getCadenceRPM();
    int servoPos   = getServoPosition();
    int resistance = constrain(map(servoPos, SERVO_MIN, SERVO_MAX, 0, 100), 0, 100);
    int power      = estimatePower(cadence, resistance);

    // Indoor Bike Data flags (uint16):
    //   Bit 0 = 0  → Instantaneous Speed present
    //   Bit 2 = 1  → Instantaneous Cadence present
    //   Bit 6 = 1  → Instantaneous Power present
    //   flags = 0x0044
    const uint16_t FLAGS = 0x0044;

    // Speed: uint16, units of 0.01 km/h
    uint16_t speed    = (uint16_t)(cadence * SPEED_COEFF_PER_RPM_HUNDREDTHS);

    // Cadence: uint16, units of 0.5 rpm
    uint16_t cadenceVal = (uint16_t)(cadence * 2);

    // Power: sint16, units of 1 W
    int16_t powerVal = (int16_t)power;

    uint8_t buf[8];
    buf[0] = (uint8_t)(FLAGS & 0xFF);
    buf[1] = (uint8_t)(FLAGS >> 8);
    buf[2] = (uint8_t)(speed & 0xFF);
    buf[3] = (uint8_t)(speed >> 8);
    buf[4] = (uint8_t)(cadenceVal & 0xFF);
    buf[5] = (uint8_t)(cadenceVal >> 8);
    buf[6] = (uint8_t)((uint16_t)powerVal & 0xFF);
    buf[7] = (uint8_t)((uint16_t)powerVal >> 8);

    pBikeData->setValue(buf, 8);
    pBikeData->notify();

    Serial.print("[BLE] Cadence: ");
    Serial.print(cadence);
    Serial.print(" rpm | Power: ");
    Serial.print(power);
    Serial.print(" W | Resistance: ");
    Serial.print(resistance);
    Serial.println("%");
}

bool bluetoothConnected() {
    return deviceConnected;
}
