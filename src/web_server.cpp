#include "web_server.h"
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include "cadence.h"
#include "control.h"
#include "storage.h"

AsyncWebServer server(80);

extern float kp;
extern float ki;

void webServerInit() {
    prefs.begin("cfg", false);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *r) {
        String html = R"rawliteral(
<!DOCTYPE html>
<html>
<body>
<h1>Gym Bike</h1>
<div id="data"></div>
<script>
setInterval(() => {
 fetch('/api/status')
 .then(r => r.json())
 .then(j => {
   document.getElementById('data').innerHTML =
     'Cadence: ' + j.cadence + ' RPM<br>' +
     'Servo: ' + j.servo + '<br>' +
     'Mode: ' + j.mode;
 });
}, 500);
</script>
<a href="/config">Settings</a>
</body>
</html>
)rawliteral";
        r->send(200, "text/html", html);
    });

    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *r) {
        String json = "{";
        json += "\"cadence\":" + String(getCadenceRPM()) + ",";
        json += "\"servo\":" + String(manualServo) + ",";
        json += "\"mode\":" + String(controlMode);
        json += "}";
        r->send(200, "application/json", json);
    });

    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *r) {
        r->send(200, "text/html",
            "<form method='POST' action='/save'>"
            "Target Cadence:<input name='cad' value='80'><br>"
            "Kp:<input name='kp'><br>"
            "Ki:<input name='ki'><br>"
            "<input type='submit'></form>");
    });

    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *r) {
        if (r->hasParam("cad", true))
            targetCadence = r->getParam("cad", true)->value().toInt();
        if (r->hasParam("kp", true))
            kp = r->getParam("kp", true)->value().toFloat();
        if (r->hasParam("ki", true))
            ki = r->getParam("ki", true)->value().toFloat();

        r->send(200, "text/plain", "Saved");
    });

    server.begin();
}
