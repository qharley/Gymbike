#include "wifi_manager.h"
#include "config.h"
#include "storage.h"
#include "captive_portal.h"
#include <Preferences.h>

void wifiInit() {
    prefs.begin("wifi", false);

    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("pass", "");

    // Always start AP
    String apName = String(AP_SSID_PREFIX) + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    WiFi.softAP(apName.c_str(), AP_PASSWORD);
    captivePortalInit();

    // Try STA if credentials exist
    if (ssid.length() > 0) {
        WiFi.begin(ssid.c_str(), pass.c_str());

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
            delay(200);
        }
    }
}

bool wifiIsConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String wifiIP() {
    if (wifiIsConnected())
        return WiFi.localIP().toString();
    return WiFi.softAPIP().toString();
}
