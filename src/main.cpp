#include <Arduino.h>
#include "wifi_manager.h"
#include "web_server.h"
#include "cadence.h"
#include "control.h"

void setup() {
    wifiInit();
    cadenceInit();
    controlInit();
    webServerInit();
}

void loop() {
    controlLoop();
    delay(20); // 50 Hz control loop
}
