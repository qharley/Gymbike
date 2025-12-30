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
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1'>"
    "<title>" + title + "</title>"
    "<style>"
    "*{margin:0;padding:0;box-sizing:border-box;}"
    "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#1a1a1a;color:#fff;height:100vh;display:flex;flex-direction:column;}"
    "header{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);padding:20px;text-align:center;box-shadow:0 4px 6px rgba(0,0,0,0.3);position:relative;}"
    "header h1{font-size:28px;font-weight:600;letter-spacing:0.5px;}"
    ".wake-indicator{position:absolute;top:20px;right:20px;font-size:12px;color:rgba(255,255,255,0.7);display:none;}"
    ".wake-indicator.active{display:block;}"
    ".container{flex:1;overflow-y:auto;padding:16px;max-width:1200px;width:100%;margin:0 auto;}"
    ".grid{display:grid;grid-template-columns:1fr;gap:16px;height:100%;}"
    "@media(min-width:768px){.grid{grid-template-columns:repeat(2,1fr);}.grid-full{grid-column:1/-1;}}"
    ".card{background:#2a2a2a;border-radius:16px;padding:24px;box-shadow:0 8px 16px rgba(0,0,0,0.4);display:flex;flex-direction:column;justify-content:center;transition:transform 0.2s;}"
    ".card:hover{transform:translateY(-2px);}"
    ".card-title{font-size:14px;text-transform:uppercase;letter-spacing:1px;color:#999;margin-bottom:12px;font-weight:600;}"
    ".card-value{font-size:72px;font-weight:700;line-height:1;color:#fff;text-align:center;margin:16px 0;}"
    ".card-value.large{font-size:96px;background:linear-gradient(135deg,#667eea,#764ba2);-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text;}"
    ".card-unit{font-size:24px;color:#999;font-weight:400;margin-left:8px;}"
    ".card-subtitle{font-size:20px;color:#ddd;text-align:center;margin-top:8px;font-weight:600;}"
    ".card-meta{font-size:14px;color:#666;text-align:center;margin-top:12px;font-family:monospace;}"
    ".badge{display:inline-block;padding:8px 16px;background:rgba(102,126,234,0.2);border:2px solid #667eea;border-radius:20px;font-size:18px;font-weight:600;color:#667eea;text-transform:uppercase;letter-spacing:1px;}"
    ".button{display:block;width:100%;padding:16px;font-size:16px;text-align:center;border-radius:12px;background:#667eea;color:white;text-decoration:none;border:none;font-weight:600;cursor:pointer;transition:all 0.2s;}"
    ".button:hover{background:#764ba2;transform:translateY(-2px);box-shadow:0 6px 12px rgba(102,126,234,0.3);}"
    ".button.secondary{background:#3a3a3a;}"
    ".button.secondary:hover{background:#4a4a4a;}"
    "input{width:100%;padding:14px;font-size:16px;margin-top:8px;border-radius:10px;border:2px solid #3a3a3a;background:#1a1a1a;color:#fff;}"
    "input:focus{outline:none;border-color:#667eea;}"
    ".settings-card{background:#2a2a2a;border-radius:16px;padding:24px;margin-bottom:16px;}"
    "</style></head><body>"
    "<header><h1>" + title + "</h1><div class='wake-indicator' id='wake-status'>&#128293; Screen Awake</div></header>"
    "<div class='container'>";
}

static String pageFooter() {
    return "</div></body></html>";
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

        html += "<div class='grid'>";
        
        // Cadence - Large prominent display
        html += "<div class='card grid-full'>";
        html += "<div class='card-title'>Current Cadence</div>";
        html += "<div class='card-value large' id='cadence'>--</div>";
        html += "</div>";

        // Control Mode
        html += "<div class='card'>";
        html += "<div class='card-title'>Control Mode</div>";
        html += "<div style='text-align:center;margin:20px 0;'>";
        html += "<span class='badge' id='mode'>--</span>";
        html += "</div>";
        html += "<div class='card-subtitle' id='target-info'>&nbsp;</div>";
        html += "</div>";

        // Resistance
        html += "<div class='card'>";
        html += "<div class='card-title'>Resistance Level</div>";
        html += "<div class='card-value' id='resistance'>--</div>";
        html += "<div class='card-meta' id='servo-debug'>Servo: --°</div>";
        html += "</div>";

        // Settings button
        html += "<div class='card grid-full' style='padding:16px;'>";
        html += "<a class='button secondary' href='/settings'>&#9881; Settings</a>";
        html += "</div>";

        html += "</div>"; // end grid

        html += "<script>";
        html += "let wakeLock=null;";
        html += "async function requestWakeLock(){";
        html += "if(!('wakeLock' in navigator)){";
        html += "console.log('Wake Lock API not supported');";
        html += "return;";
        html += "}";
        html += "try{";
        html += "wakeLock=await navigator.wakeLock.request('screen');";
        html += "document.getElementById('wake-status').classList.add('active');";
        html += "console.log('Wake Lock acquired');";
        html += "wakeLock.addEventListener('release',()=>{";
        html += "console.log('Wake Lock released');";
        html += "document.getElementById('wake-status').classList.remove('active');";
        html += "});";
        html += "}catch(err){";
        html += "console.error('Wake Lock error:',err.name,err.message);";
        html += "}";
        html += "}";
        html += "document.addEventListener('visibilitychange',()=>{";
        html += "if(document.visibilityState==='visible'){";
        html += "requestWakeLock();";
        html += "}";
        html += "});";
        html += "window.addEventListener('click',()=>{";
        html += "if(wakeLock===null){requestWakeLock();}";
        html += "},{once:true});";
        html += "setTimeout(requestWakeLock,500);";
        html += "function updateStatus(){fetch('/api/status').then(r=>r.json()).then(d=>{";
        html += "document.getElementById('cadence').innerHTML=d.cadence+'<span class=\"card-unit\">rpm</span>';";
        html += "document.getElementById('resistance').innerHTML=d.resistance+'<span class=\"card-unit\">%</span>';";
        html += "document.getElementById('servo-debug').innerText='Servo: '+d.servo+'°';";
        html += "let modes=['Manual','Cadence','ERG'];";
        html += "document.getElementById('mode').innerText=modes[d.mode];";
        html += "let info='';";
        html += "if(d.mode==1)info='Target: '+d.targetCadence+' rpm';";
        html += "if(d.mode==2)info='Target: '+d.targetWatts+' W';";
        html += "document.getElementById('target-info').innerHTML=info||'&nbsp;';";
        html += "}).catch(e=>console.error(e));}";
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

        html += "<div class='settings-card'>";
        html += "<div class='card-title'>Wi-Fi Connection</div>";
        if (wifiSTAConnected()) {
            html += "<p style='margin:12px 0;font-size:16px;'>&#10003; Connected to Wi-Fi</p>";
            html += "<p style='margin:12px 0;font-size:14px;color:#999;'>IP: " + wifiSTAIP() + "</p>";
            html +=
                "<form method='POST' action='/wifi/disconnect'>"
                "<button class='button secondary' style='margin-top:12px;'>Disconnect</button>"
                "</form>";
        } else {
            html += "<p style='margin:12px 0;font-size:16px;color:#999;'>&#10007; Not connected</p>";
        }
        html += "</div>";

        html += "<div class='settings-card'>";
        html += "<div class='card-title'>Connect to Network</div>";
        html += "<form method='POST' action='/wifi/connect'>";
        html += "<input name='ssid' placeholder='Network Name (SSID)' required>";
        html += "<input name='pass' type='password' placeholder='Password (optional)'>";
        html += "<button class='button' style='margin-top:16px;'>Connect</button>";
        html += "</form>";
        html += "</div>";

        html += "<div class='settings-card'>";
        html += "<a class='button secondary' href='/update'>&#8635; Firmware Update</a>";
        html += "<a class='button secondary' href='/' style='margin-top:12px;'>&#8592; Back to Bike</a>";
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
