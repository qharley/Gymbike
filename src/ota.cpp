#include "ota.h"
#include <ESPAsyncWebServer.h>
#include <Update.h>

extern AsyncWebServer server;

void otaInit() {
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
            r->send(200, "text/plain", ok ? "Update OK. Rebooting..." : "Update failed");
            delay(1000);
            ESP.restart();
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
