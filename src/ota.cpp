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
