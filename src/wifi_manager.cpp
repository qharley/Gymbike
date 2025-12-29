#include "wifi_manager.h"
#include "storage.h"
#include "captive_portal.h"

void wifiInit() {
    prefs.begin("wifi", false);

    // Start AP (always)
    String apName = "GymBike-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    WiFi.softAP(apName.c_str(), "gymbike123");
    captivePortalInit();

    // Try STA if saved
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("pass", "");

    if (ssid.length()) {
        WiFi.begin(ssid.c_str(), pass.c_str());

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
            delay(100);
        }
    }
}

void wifiConnect(const String& ssid, const String& pass) {
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);

    WiFi.disconnect(true);
    delay(500);
    WiFi.begin(ssid.c_str(), pass.c_str());
}

void wifiDisconnectSTA() {
    prefs.remove("ssid");
    prefs.remove("pass");
    WiFi.disconnect(true);
}

bool wifiSTAConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String wifiSTAIP() {
    return WiFi.localIP().toString();
}

String wifiAPIP() {
    return WiFi.softAPIP().toString();
}
