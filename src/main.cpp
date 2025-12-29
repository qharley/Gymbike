#include <Arduino.h>
#include "wifi_manager.h"
#include "web_server.h"
#include "cadence.h"
#include "control.h"
#include "storage.h"
#include "captive_portal.h"

void setup() {
    loadControlConfig();
    wifiInit();
    cadenceInit();
    controlInit();
    webServerInit();
}

void loop() {
    captivePortalLoop();
    controlLoop();
    delay(20);
}

