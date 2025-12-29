#include <Arduino.h>
#include "wifi_manager.h"
#include "web_server.h"
#include "cadence.h"
#include "control.h"
#include "storage.h"
#include "captive_portal.h"
#include "ota.h"

void setup() {
    loadControlConfig();
    wifiInit();
    cadenceInit();
    controlInit();
    webServerInit();
    otaInit();
}

void loop() {
    captivePortalLoop();
    controlLoop();
    delay(20);
}

