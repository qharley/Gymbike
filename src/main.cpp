#include <Arduino.h>
#include "wifi_manager.h"
#include "web_server.h"
#include "cadence.h"
#include "control.h"
#include "storage.h"
#include "captive_portal.h"
#include "ota.h"
#include "rotary_encoder.h"

void setup() {
    loadControlConfig();
    wifiInit();
    cadenceInit();
    encoderInit();
    controlInit();
    webServerInit();
    otaInit();
}

void loop() {
    captivePortalLoop();
    encoderLoop();
    controlLoop();
    delay(20);
}

