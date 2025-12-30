#include "web_server.h"
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "wifi_manager.h"
#include "control.h"
#include "cadence.h"
#include <ArduinoJson.h>

// Forward declaration if not in wifi_manager.h
bool wifiHasSavedCredentials();

AsyncWebServer server(80);

/* ---------- Shared mobile-friendly page wrapper ---------- */

static String pageHeader(const String& title) {
    return
    "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>" + title + "</title>"
    "<style>"
    "body{font-family:sans-serif;margin:0;background:#f2f2f2;}"
    "header{background:#222;color:#fff;padding:14px;text-align:center;font-size:20px;}"
    ".card{background:#fff;margin:12px;padding:16px;border-radius:10px;box-shadow:0 2px 5px rgba(0,0,0,0.15);}"
    "a.button,button{display:block;width:100%;padding:14px;margin-top:10px;"
    "font-size:18px;text-align:center;border-radius:8px;"
    "background:#007bff;color:white;text-decoration:none;border:none;}"
    "button.secondary,a.secondary{background:#666;}"
    "input{width:100%;padding:12px;font-size:16px;margin-top:6px;border-radius:6px;border:1px solid #ccc;}"
    "p{font-size:16px;}"
    ".readout{text-align:center;font-size:36px;margin:10px 0;}"
    ".readout-large{text-align:center;font-size:48px;margin:10px 0;font-weight:bold;}"
    "</style></head><body>"
    "<header>" + title + "</header>";
}

static String pageFooter() {
    return "</body></html>";
}

/* ---------- Web server init ---------- */

void webServerInit() {

    /* ===== Gym Bike Main Display ===== */
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *r) {

        if (!wifiHasSavedCredentials()) {
            r->redirect("/settings");
            return;
        }

        String html = pageHeader("Gym Bike");

        html += "<div class='card'>";
        html += "<p><b>Cadence</b></p>";
        html += "<p class='readout-large' id='cadence'>-- rpm</p>";
        html += "</div>";

        html += "<div class='card'>";
        html += "<p><b>Control Mode</b></p>";
        html += "<p id='mode' style='text-align:center;'>--</p>";
        html += "<p style='font-size:14px;color:#666;text-align:center;' id='target-info'>--</p>";
        html += "<button onclick='setMode(0)'>Manual</button>";
        html += "<button onclick='setMode(1)'>Cadence</button>";
        html += "<button onclick='setMode(2)'>ERG</button>";
        html += "</div>";

        html += "<div class='card'>";
        html += "<p><b>Resistance</b></p>";
        html += "<p class='readout' id='resistance'>-- %</p>";
        html += "<p style='font-size:12px;color:#666;text-align:center;' id='servo-debug'>Servo: -- | Target: --</p>";
        html += "<button onclick='adjustResistance(-10)'>- 10%</button>";
        html += "<button onclick='adjustResistance(-5)'>- 5%</button>";
        html += "<button onclick='adjustResistance(5)'>+ 5%</button>";
        html += "<button onclick='adjustResistance(10)'>+ 10%</button>";
        html += "</div>";

        html += "<div class='card'>";
        html += "<a class='button secondary' href='/settings'>Settings</a>";
        html += "</div>";

        html += "<script>";
        html += "function setMode(m){fetch('/api/mode?mode='+m).then(updateStatus);}";
        html += "function adjustResistance(d){fetch('/api/resistance?delta='+d).then(updateStatus);}";
        html += "function updateStatus(){fetch('/api/status').then(r=>r.json()).then(d=>{";
        html += "document.getElementById('cadence').innerText=d.cadence+' rpm';";
        html += "document.getElementById('resistance').innerText=d.resistance+' %';";
        html += "document.getElementById('servo-debug').innerText='Servo: '+d.servo+'° | Target: '+d.targetCadence+'°';";
        html += "let modes=['Manual','Cadence','ERG'];";
        html += "document.getElementById('mode').innerText=modes[d.mode];";
        html += "let info='';";
        html += "if(d.mode==1)info='Target: '+d.targetCadence+' rpm';";
        html += "if(d.mode==2)info='Target: '+d.targetWatts+' W';";
        html += "document.getElementById('target-info').innerText=info;";
        html += "});}";
        html += "setInterval(updateStatus,1000);updateStatus();";
        html += "</script>";

        html += pageFooter();
        r->send(200, "text/html", html);
    });

    /* ===== API: Get Status ===== */
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *r) {
        JsonDocument doc;
        doc["cadence"] = getCadenceRPM();
        doc["mode"] = (int)controlMode;
        
        int currentPos = getServoPosition();
        int resistance = map(currentPos, SERVO_MIN, SERVO_MAX, 0, 100);
        resistance = constrain(resistance, 0, 100);
        doc["resistance"] = resistance;
        doc["servo"] = currentPos;
        doc["targetCadence"] = targetCadence;
        doc["targetWatts"] = targetWatts;
        
        String output;
        serializeJson(doc, output);
        r->send(200, "application/json", output);
    });

    /* ===== API: Set Mode ===== */
    server.on("/api/mode", HTTP_GET, [](AsyncWebServerRequest *r) {
        if (r->hasParam("mode")) {
            int mode = r->getParam("mode")->value().toInt();
            if (mode >= 0 && mode <= 2) {
                ControlMode newMode = (ControlMode)mode;
                
                // Smooth mode transition
                if (newMode == MODE_MANUAL) {
                    manualServo = getServoPosition(); // Keep current position
                    resetPID();
                } else if (controlMode == MODE_MANUAL && newMode == MODE_CADENCE) {
                    resetPID(); // Reset PID when entering cadence mode
                }
                
                controlMode = newMode;
            }
        }
        r->send(200, "text/plain", "OK");
    });

    /* ===== API: Adjust Resistance ===== */
    server.on("/api/resistance", HTTP_GET, [](AsyncWebServerRequest *r) {
        if (r->hasParam("delta")) {
            // Automatically switch to manual mode if not already
            if (controlMode != MODE_MANUAL) {
                controlMode = MODE_MANUAL;
                manualServo = getServoPosition();
                resetPID();
            }
            
            int delta = r->getParam("delta")->value().toInt();
            int current = map(getServoPosition(), SERVO_MIN, SERVO_MAX, 0, 100);
            current = constrain(current, 0, 100);
            int newValue = constrain(current + delta, 0, 100);
            int newServo = map(newValue, 0, 100, SERVO_MIN, SERVO_MAX);
            
            setManualServo(newServo);
        }
        r->send(200, "text/plain", "OK");
    });

    /* ===== API: Set Target Cadence ===== */
    server.on("/api/target", HTTP_GET, [](AsyncWebServerRequest *r) {
        if (r->hasParam("cadence")) {
            int cadence = r->getParam("cadence")->value().toInt();
            setTargetCadence(cadence);
        }
        r->send(200, "text/plain", "OK");
    });

    /* ===== Settings Page ===== */
    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *r) {

        String html = pageHeader("Settings");

        html += "<div class='card'>";
        if (wifiSTAConnected()) {
            html += "<p><b>Connected to Wi-Fi</b></p>";
            html += "<p>IP: " + wifiSTAIP() + "</p>";
            html +=
                "<form method='POST' action='/wifi/disconnect'>"
                "<button class='secondary'>Disconnect</button>"
                "</form>";
        } else {
            html += "<p><b>Not connected</b></p>";
        }
        html += "</div>";

        html += "<div class='card'>";
        html += "<form method='POST' action='/wifi/connect'>";
        html += "<p>Connect to network</p>";
        html += "<input name='ssid' placeholder='SSID' required>";
        html += "<input name='pass' type='password' placeholder='Password'>";
        html += "<button>Connect</button>";
        html += "</form>";
        html += "</div>";

        html += "<div class='card'>";
        html += "<a class='button secondary' href='/update'>Firmware Update</a>";
        html += "<a class='button secondary' href='/'>Back to Bike</a>";
        html += "</div>";

        html += pageFooter();
        r->send(200, "text/html", html);
    });

    /* ===== Wi-Fi connect ===== */
    server.on("/wifi/connect", HTTP_POST, [](AsyncWebServerRequest *r) {
        if (r->hasParam("ssid", true)) {
            String ssid = r->getParam("ssid", true)->value();
            String pass = r->hasParam("pass", true)
                            ? r->getParam("pass", true)->value()
                            : "";
            wifiConnect(ssid, pass);
        }
        r->redirect("/settings");
    });

    /* ===== Wi-Fi disconnect ===== */
    server.on("/wifi/disconnect", HTTP_POST, [](AsyncWebServerRequest *r) {
        wifiDisconnectSTA();
        r->redirect("/settings");
    });

    /* ===== Captive Portal / Fallback ===== */
    server.onNotFound([](AsyncWebServerRequest *r) {
        if (!wifiHasSavedCredentials()) {
            r->redirect("/settings");
        } else {
            r->redirect("/");
        }
    });

    server.begin();
}
