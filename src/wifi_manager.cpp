#include "wifi_manager.h"
#include "storage.h"
#include "captive_portal.h"

extern Preferences prefs;

void wifiInit() {
    // Start AP (always)
    String apName = "GymBike-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    WiFi.softAP(apName.c_str(), "gymbike123");
    captivePortalInit();

    // Try STA if saved
    prefs.begin("wifi", true); // Read-only
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("pass", "");
    prefs.end();

    if (ssid.length()) {
        Serial.printf("[LOAD] WiFi credentials: SSID=%s\n", ssid.c_str());
        WiFi.begin(ssid.c_str(), pass.c_str());

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
            delay(100);
        }
    } else {
        Serial.println("[LOAD] No saved WiFi credentials");
    }
}

void wifiConnect(const String& ssid, const String& pass) {
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.end();

    WiFi.disconnect(true);
    delay(500);
    WiFi.begin(ssid.c_str(), pass.c_str());
}

void wifiDisconnectSTA() {
    prefs.begin("wifi", false);
    prefs.remove("ssid");
    prefs.remove("pass");
    prefs.end();
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

bool wifiHasSavedCredentials() {
    prefs.begin("wifi", true); // read-only
    String ssid = prefs.getString("ssid", "");
    prefs.end();
    return ssid.length() > 0;
}