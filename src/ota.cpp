#include "ota.h"
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <ArduinoOTA.h>

extern AsyncWebServer server;

void otaInit() {
    // Initialize ArduinoOTA for PlatformIO OTA uploads
    ArduinoOTA.setHostname("gymbike");
    ArduinoOTA.setPassword("gymbike123");
    ArduinoOTA.setPort(3232);
    
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else {  // U_SPIFFS
            type = "filesystem";
        }
        Serial.println("Start updating " + type);
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    
    ArduinoOTA.begin();
    Serial.println("ArduinoOTA ready");
    
    // Initialize web-based OTA
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *r) {
        r->send(200, "text/html",
            "<h2>OTA Update</h2>"
            "<form method='POST' action='/update' enctype='multipart/form-data'>"
            "<input type='file' name='update'>"
            "<input type='submit' value='Update'>"
            "</form>");
    });

    server.on(
        "/update",
        HTTP_POST,
        [](AsyncWebServerRequest *r) {
            bool ok = !Update.hasError();
            if (ok) {
                r->send(200, "text/html",
                    "<html><head>"
                    "<meta http-equiv='refresh' content='15;url=/'>"
                    "<style>body{font-family:Arial;text-align:center;padding:50px;}</style>"
                    "</head><body>"
                    "<h2>Update Successful!</h2>"
                    "<p>Device is rebooting...</p>"
                    "<p>You will be redirected to the main page in 15 seconds.</p>"
                    "<p><a href='/'>Click here</a> if not redirected automatically.</p>"
                    "</body></html>");
            } else {
                r->send(200, "text/html",
                    "<html><head>"
                    "<meta http-equiv='refresh' content='5;url=/update'>"
                    "<style>body{font-family:Arial;text-align:center;padding:50px;}</style>"
                    "</head><body>"
                    "<h2>Update Failed!</h2>"
                    "<p>Redirecting back to update page...</p>"
                    "</body></html>");
            }
            delay(1000);
            if (ok) {
                ESP.restart();
            }
        },
        [](AsyncWebServerRequest *r, String filename, size_t index,
           uint8_t *data, size_t len, bool final) {

            if (!index) {
                Update.begin(UPDATE_SIZE_UNKNOWN);
            }
            Update.write(data, len);
            if (final) {
                Update.end(true);
            }
        }
    );
}

void otaLoop() {
    ArduinoOTA.handle();
}
