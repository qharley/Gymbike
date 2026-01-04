#include "web_server.h"
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "wifi_manager.h"
#include "control.h"
#include "cadence.h"
#include "buttons.h"
#include "rotary_encoder.h"
#include "config.h"
#include "storage.h"
#include <ArduinoJson.h>

// Forward declaration if not in wifi_manager.h
bool wifiHasSavedCredentials();

AsyncWebServer server(80);

/* ---------- Shared mobile-friendly page wrapper ---------- */

static String pageHeader(const String& title, bool noScroll = false) {
    String bodyClass = noScroll ? " class='no-scroll'" : "";
    return
    "<!DOCTYPE html><html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1'>"
    "<link rel='icon' type='image/svg+xml' href='/favicon.svg'>"
    "<title>" + title + "</title>"
    "<style>"
    "*{margin:0;padding:0;box-sizing:border-box;}"
    "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#1a1a1a;color:#fff;height:100vh;display:flex;flex-direction:column;overflow:hidden;}"
    "body.no-scroll{height:100vh;overflow:hidden;}"
    "header{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);padding:3px 6px;box-shadow:0 2px 4px rgba(0,0,0,0.3);position:relative;display:flex;justify-content:space-between;align-items:center;}"
    "header h1{font-size:13px;font-weight:600;letter-spacing:0.3px;margin:0;}"
    ".settings-icon{font-size:16px;color:#fff;text-decoration:none;padding:3px;cursor:pointer;opacity:0.9;}"
    ".settings-icon:hover{opacity:1;}"
    ".wake-indicator{position:absolute;top:3px;right:35px;font-size:8px;color:rgba(255,255,255,0.7);display:none;}"
    ".wake-indicator.active{display:block;}"
    ".container{flex:1;padding:3px;width:100%;display:flex;flex-direction:column;overflow:hidden;}"
    "body.no-scroll .container{overflow:hidden;}"
    ".grid{display:grid;grid-template-columns:1fr 1fr;gap:3px;flex:1;grid-template-rows:auto 1fr auto;height:100%;}"
    "@media(min-width:768px){.grid{grid-template-columns:repeat(2,1fr);grid-template-rows:auto 1fr auto;}.grid-full{grid-column:1/-1;}}"
    ".grid-full{grid-column:1/-1;}"
    ".card{background:#2a2a2a;border-radius:6px;padding:3px 4px;box-shadow:0 2px 4px rgba(0,0,0,0.4);display:flex;flex-direction:column;justify-content:center;min-height:0;overflow:hidden;}"
    ".card-title{font-size:8px;text-transform:uppercase;letter-spacing:0.3px;color:#999;margin-bottom:2px;font-weight:600;}"
    ".card-value{font-size:64px;font-weight:700;line-height:0.85;color:#fff;text-align:center;margin:2px 0;}"
    ".card-value.large{font-size:72px;background:linear-gradient(135deg,#667eea,#764ba2);-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text;}"
    ".card-unit{font-size:16px;color:#999;font-weight:400;margin-left:2px;}"
    ".card-subtitle{font-size:10px;color:#ddd;text-align:center;margin-top:2px;font-weight:600;}"
    ".card-meta{font-size:7px;color:#666;text-align:center;margin-top:1px;font-family:monospace;}"
    ".badge{display:inline-block;padding:2px 6px;background:rgba(102,126,234,0.2);border:1px solid #667eea;border-radius:10px;font-size:9px;font-weight:600;color:#667eea;text-transform:uppercase;letter-spacing:0.3px;}"
    ".button{display:block;width:100%;padding:16px;font-size:16px;text-align:center;border-radius:12px;background:#667eea;color:white;text-decoration:none;border:none;font-weight:600;cursor:pointer;transition:all 0.2s;}"
    ".button:hover{background:#764ba2;transform:translateY(-2px);box-shadow:0 6px 12px rgba(102,126,234,0.3);}"
    ".button.secondary{background:#3a3a3a;}"
    ".button.secondary:hover{background:#4a4a4a;}"
    "input{width:100%;padding:14px;font-size:16px;margin-top:8px;border-radius:10px;border:2px solid #3a3a3a;background:#1a1a1a;color:#fff;}"
    "input:focus{outline:none;border-color:#667eea;}"
    ".settings-card{background:#2a2a2a;border-radius:16px;padding:24px;margin-bottom:16px;}"
    ".error-msg{display:none;position:fixed;top:10px;left:50%;transform:translateX(-50%);background:#ef4444;color:#fff;padding:10px 20px;border-radius:8px;font-size:14px;z-index:1000;}"
    ".error-msg.show{display:block;}"
    "</style></head><body" + bodyClass + ">"
    "<header><h1>" + title + "</h1><div class='wake-indicator' id='wake-status'>&#128293;</div><a href='/settings' class='settings-icon'>&#9881;</a></header>"
    "<div id='error-msg' class='error-msg'>Connection error</div>"
    "<div class='container'>";
}

static String pageFooter() {
    return "</div></body></html>";
}

/* ---------- Web server init ---------- */

void webServerInit() {

    // Enable CORS for all responses
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    /* ===== Gym Bike Main Display ===== */
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *r) {

        if (!wifiHasSavedCredentials()) {
            r->redirect("/settings");
            return;
        }

        String html = pageHeader("Gym Bike", true);

        html += "<div class='grid'>";
        
        // Workout Status - compact
        html += "<div class='card grid-full' style='padding:3px 4px;min-height:auto;'>";
        html += "<div style='display:flex;justify-content:space-between;align-items:center;'>";
        html += "<span class='badge' id='workout-state' style='margin:0;'>Stopped</span>";
        html += "<div style='text-align:right;'><div style='font-size:18px;font-weight:700;color:#fff;line-height:1;' id='workout-timer'>00:00</div>";
        html += "<div style='font-size:7px;color:#666;line-height:1;' id='rest-info'>&nbsp;</div></div>";
        html += "</div></div>";
        
        // Cadence - Large prominent display
        html += "<div class='card'>";
        html += "<div class='card-title'>Cadence</div>";
        html += "<div class='card-value' id='cadence'>--</div>";
        html += "</div>";

        // Resistance
        html += "<div class='card'>";
        html += "<div class='card-title'>Resistance</div>";
        html += "<div class='card-value' id='resistance'>--</div>";
        html += "<div class='card-meta' id='servo-debug'>Servo: --°</div>";
        html += "</div>";

        // Control Mode - bottom row
        html += "<div class='card grid-full' style='padding:3px 4px;min-height:auto;'>";
        html += "<div style='display:flex;justify-content:space-between;align-items:center;'>";
        html += "<div><div class='card-title' style='margin:0 0 1px 0;'>Mode</div><span class='badge' id='mode' style='margin:0;font-size:9px;'>--</span></div>";
        html += "<div class='card-subtitle' style='margin:0;font-size:11px;' id='target-info'>&nbsp;</div>";
        html += "</div></div>";

        html += "</div>"; // end grid

        html += "<script>";
        html += "console.log('GymBike UI Loading...');";
        html += "let wakeLock=null;";
        html += "let fetchErrors=0;";
        html += "function showError(msg){";
        html += "let el=document.getElementById('error-msg');";
        html += "el.textContent=msg;";
        html += "el.classList.add('show');";
        html += "setTimeout(()=>el.classList.remove('show'),3000);";
        html += "}";
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
        html += "function formatTime(seconds){";
        html += "let h=Math.floor(seconds/3600);";
        html += "let m=Math.floor((seconds%3600)/60);";
        html += "let s=seconds%60;";
        html += "if(h>0)return h+':'+(m<10?'0':'')+m+':'+(s<10?'0':'')+s;";
        html += "return m+':'+(s<10?'0':'')+s;";
        html += "}";
        html += "function updateStatus(){";
        html += "fetch('/api/status').then(r=>{";
        html += "if(!r.ok)throw new Error('HTTP '+r.status);";
        html += "return r.json();";
        html += "}).then(d=>{";
        html += "fetchErrors=0;";
        html += "console.log('Status update:',d);";
        html += "document.getElementById('cadence').innerHTML=d.cadence+'<span class=\"card-unit\">rpm</span>';";
        html += "document.getElementById('resistance').innerHTML=d.resistance+'<span class=\"card-unit\">%</span>';";
        html += "document.getElementById('servo-debug').innerText='Servo: '+d.servo+'°';";
        html += "let modes=['Manual','Cadence','ERG'];";
        html += "document.getElementById('mode').innerText=modes[d.mode];";
        html += "let info='';";
        html += "if(d.mode==1)info='Target: '+d.targetCadence+' rpm';";
        html += "if(d.mode==2)info='Target: '+d.targetWatts+' W';";
        html += "document.getElementById('target-info').innerHTML=info||'&nbsp;';";
        html += "let states=['Stopped','Running','Resting'];";
        html += "let stateColors=['#999','#4ade80','#fbbf24'];";
        html += "let stateBadge=document.getElementById('workout-state');";
        html += "stateBadge.innerText=states[d.workoutState];";
        html += "stateBadge.style.borderColor=stateColors[d.workoutState];";
        html += "stateBadge.style.color=stateColors[d.workoutState];";
        html += "document.getElementById('workout-timer').innerHTML=formatTime(d.workoutTime);";
        html += "let restInfo='';";
        html += "if(d.workoutState==2&&d.restTime)restInfo='Rest: '+formatTime(d.restTime);";
        html += "document.getElementById('rest-info').innerHTML=restInfo||'&nbsp;';";
        html += "}).catch(e=>{";
        html += "console.error('Fetch error:',e);";
        html += "fetchErrors++;";
        html += "if(fetchErrors>=3)showError('Connection lost');";
        html += "});}";
        html += "setInterval(updateStatus,1000);updateStatus();";
        html += "console.log('GymBike UI Ready');";
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
        
        // Workout state and timer
        doc["workoutState"] = (int)workoutState;
        unsigned long currentTime = millis();
        unsigned long totalElapsed = workoutElapsedTime;
        
        if (workoutState == WORKOUT_RUNNING) {
            totalElapsed += (currentTime - workoutStartTime);
        } else if (workoutState == WORKOUT_RESTING) {
            // Show rest duration
            doc["restTime"] = (currentTime - restStartTime) / 1000;
        }
        
        doc["workoutTime"] = totalElapsed / 1000; // Convert to seconds
        
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

    /* ===== Cadence Settings API ===== */
    server.on("/api/cadence/settings", HTTP_GET, [](AsyncWebServerRequest *r) {
        // GET: Return current settings as JSON
        String json = "{";
        json += "\"targetCadence\":" + String(targetCadence) + ",";
        json += "\"curveWidth\":" + String(getCadenceCurveWidth(), 2) + ",";
        json += "\"curveGain\":" + String(getCadenceCurveGain(), 2);
        json += "}";
        r->send(200, "application/json", json);
    });

    server.on("/api/cadence/settings", HTTP_POST, [](AsyncWebServerRequest *r) {
        // POST: Update settings
        if (r->hasParam("targetCadence", true)) {
            setTargetCadence(r->getParam("targetCadence", true)->value().toInt());
        }
        if (r->hasParam("curveWidth", true)) {
            setCadenceCurveWidth(r->getParam("curveWidth", true)->value().toFloat());
        }
        if (r->hasParam("curveGain", true)) {
            setCadenceCurveGain(r->getParam("curveGain", true)->value().toFloat());
        }
        saveControlConfig();
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
        html += "<div class='card-title'>Cadence Mode Settings</div>";
        html += "<form method='POST' action='/settings/cadence' id='cadenceForm'>";
        html += "<label style='display:block;margin:12px 0 4px;font-size:14px;color:#ccc;'>Default Target Cadence (RPM)</label>";
        html += "<input name='targetCadence' type='number' min='40' max='120' step='1' value='" + String(targetCadence) + "' style='margin-top:4px;'>";
        html += "<label style='display:block;margin:12px 0 4px;font-size:14px;color:#ccc;'>Curve Width (RPM) - Controls responsiveness near target</label>";
        html += "<input name='curveWidth' type='number' min='1' max='30' step='0.5' value='" + String(getCadenceCurveWidth(), 1) + "' style='margin-top:4px;'>";
        html += "<p style='margin:4px 0;font-size:12px;color:#666;'>Smaller = tighter control, Larger = gentler response</p>";
        html += "<label style='display:block;margin:12px 0 4px;font-size:14px;color:#ccc;'>Curve Gain - Controls maximum rate of change</label>";
        html += "<input name='curveGain' type='number' min='0.1' max='10' step='0.1' value='" + String(getCadenceCurveGain(), 1) + "' style='margin-top:4px;'>";
        html += "<p style='margin:4px 0;font-size:12px;color:#666;'>Higher = faster changes when far from target</p>";
        html += "<button class='button' style='margin-top:16px;'>Save Cadence Settings</button>";
        html += "</form>";
        html += "</div>";

        html += "<div class='settings-card'>";
        html += "<a class='button secondary' href='/update'>&#8635; Firmware Update</a>";
        html += "<a class='button secondary' href='/input' style='margin-top:12px;'>&#128295; Input Troubleshooting</a>";
        html += "<a class='button secondary' href='/' style='margin-top:12px;'>&#8592; Back to Bike</a>";
        html += "</div>";

        html += "<div class='settings-card'>";
        html += "<div class='card-title'>System Information</div>";
        html += "<p style='margin:8px 0;font-size:14px;color:#999;'><strong>Version:</strong> " + String(BUILD_VERSION) + "</p>";
        html += "<p style='margin:8px 0;font-size:14px;color:#999;'><strong>Build Date:</strong> " + String(BUILD_DATE) + " " + String(BUILD_TIME) + "</p>";
        html += "<p style='margin:8px 0;font-size:14px;color:#999;'><strong>Chip ID:</strong> " + String((uint32_t)ESP.getEfuseMac(), HEX) + "</p>";
        html += "<p style='margin:8px 0;font-size:14px;color:#999;'><strong>Free Heap:</strong> " + String(ESP.getFreeHeap() / 1024) + " KB</p>";
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

    /* ===== Cadence Settings Save ===== */
    server.on("/settings/cadence", HTTP_POST, [](AsyncWebServerRequest *r) {
        if (r->hasParam("targetCadence", true)) {
            int cadence = r->getParam("targetCadence", true)->value().toInt();
            setTargetCadence(cadence);
        }
        if (r->hasParam("curveWidth", true)) {
            float width = r->getParam("curveWidth", true)->value().toFloat();
            setCadenceCurveWidth(width);
        }
        if (r->hasParam("curveGain", true)) {
            float gain = r->getParam("curveGain", true)->value().toFloat();
            setCadenceCurveGain(gain);
        }
        saveControlConfig();
        r->redirect("/settings");
    });

    /* ===== Input Troubleshooting Page ===== */
    server.on("/input", HTTP_GET, [](AsyncWebServerRequest *r) {
        String html = pageHeader("Input Troubleshooting");
        
        html += "<style>"
                ".input-grid{display:grid;grid-template-columns:1fr;gap:12px;}"
                ".input-row{display:flex;justify-content:space-between;align-items:center;padding:12px;background:#1a1a1a;border-radius:8px;}"
                ".input-label{font-size:14px;color:#999;}"
                ".input-value{font-size:16px;font-weight:600;font-family:monospace;}"
                ".status-active{color:#4ade80;}"
                ".status-inactive{color:#666;}"
                ".status-high{color:#fbbf24;}"
                ".status-low{color:#60a5fa;}"
                "</style>";
        
        html += "<div class='settings-card'>";
        html += "<div class='card-title'>Physical Buttons (Active Low)</div>";
        html += "<div class='input-grid'>";
        html += "<div class='input-row'><span class='input-label'>Start/Stop (Pin 12)</span><span id='btn-start' class='input-value'>--</span></div>";
        html += "<div class='input-row'><span class='input-label'>Rest (Pin 13)</span><span id='btn-rest' class='input-value'>--</span></div>";
        html += "<div class='input-row'><span class='input-label'>Reset (Pin 14)</span><span id='btn-reset' class='input-value'>--</span></div>";
        html += "</div></div>";
        
        html += "<div class='settings-card'>";
        html += "<div class='card-title'>Rotary Encoder</div>";
        html += "<div class='input-grid'>";
        html += "<div class='input-row'><span class='input-label'>Position</span><span id='enc-pos' class='input-value'>--</span></div>";
        html += "<div class='input-row'><span class='input-label'>CLK Pin (32)</span><span id='enc-clk' class='input-value'>--</span></div>";
        html += "<div class='input-row'><span class='input-label'>DT Pin (33)</span><span id='enc-dt' class='input-value'>--</span></div>";
        html += "<div class='input-row'><span class='input-label'>SW Button (27)</span><span id='enc-btn' class='input-value'>--</span></div>";
        html += "</div></div>";
        
        html += "<div class='settings-card'>";
        html += "<div class='card-title'>Cadence Sensor</div>";
        html += "<div class='input-grid'>";
        html += "<div class='input-row'><span class='input-label'>Current RPM</span><span id='cadence-rpm' class='input-value'>--</span></div>";
        html += "<div class='input-row'><span class='input-label'>Pin State (26)</span><span id='cadence-pin' class='input-value'>--</span></div>";
        html += "<div class='input-row'><span class='input-label'>Last Pulse</span><span id='cadence-last' class='input-value'>--</span></div>";
        html += "</div></div>";
        
        html += "<div class='settings-card'>";
        html += "<div class='card-title'>Emergency Stop</div>";
        html += "<div class='input-grid'>";
        html += "<div class='input-row'><span class='input-label'>Pin (34)</span><span id='emergency' class='input-value'>--</span></div>";
        html += "</div></div>";
        
        html += "<div class='settings-card'>";
        html += "<a class='button secondary' href='/settings'>&#8592; Back to Settings</a>";
        html += "</div>";
        
        html += "<script>"
                "function updateInputs(){"
                "fetch('/api/inputs').then(r=>r.json()).then(d=>{"
                "document.getElementById('btn-start').textContent=d.btnStart?'PRESSED':'Released';"
                "document.getElementById('btn-start').className='input-value '+(d.btnStart?'status-active':'status-inactive');"
                "document.getElementById('btn-rest').textContent=d.btnRest?'PRESSED':'Released';"
                "document.getElementById('btn-rest').className='input-value '+(d.btnRest?'status-active':'status-inactive');"
                "document.getElementById('btn-reset').textContent=d.btnReset?'PRESSED':'Released';"
                "document.getElementById('btn-reset').className='input-value '+(d.btnReset?'status-active':'status-inactive');"
                "document.getElementById('enc-pos').textContent=d.encPos;"
                "document.getElementById('enc-clk').textContent=d.encClk?'HIGH':'LOW';"
                "document.getElementById('enc-clk').className='input-value '+(d.encClk?'status-high':'status-low');"
                "document.getElementById('enc-dt').textContent=d.encDt?'HIGH':'LOW';"
                "document.getElementById('enc-dt').className='input-value '+(d.encDt?'status-high':'status-low');"
                "document.getElementById('enc-btn').textContent=d.encBtn?'Released':'PRESSED';"
                "document.getElementById('enc-btn').className='input-value '+(d.encBtn?'status-inactive':'status-active');"
                "document.getElementById('cadence-rpm').textContent=d.cadenceRpm+' RPM';"
                "document.getElementById('cadence-pin').textContent=d.cadencePin?'HIGH':'LOW';"
                "document.getElementById('cadence-pin').className='input-value '+(d.cadencePin?'status-high':'status-low');"
                "let timeSince=Date.now()-d.cadenceLast;"
                "document.getElementById('cadence-last').textContent=timeSince<5000?timeSince+' ms ago':'No recent pulse';"
                "document.getElementById('emergency').textContent=d.emergency?'NORMAL':'TRIGGERED';"
                "document.getElementById('emergency').className='input-value '+(d.emergency?'status-inactive':'status-active');"
                "}).catch(e=>console.error(e));}"
                "setInterval(updateInputs,100);"
                "updateInputs();"
                "</script>";
        
        html += pageFooter();
        r->send(200, "text/html", html);
    });

    /* ===== API: Input States ===== */
    server.on("/api/inputs", HTTP_GET, [](AsyncWebServerRequest *r) {
        JsonDocument doc;
        
        // Physical buttons (active low - inverted for display)
        doc["btnStart"] = !getRawButtonState(BTN_START_STOP);
        doc["btnRest"] = !getRawButtonState(BTN_REST);
        doc["btnReset"] = !getRawButtonState(BTN_RESET);
        
        // Rotary encoder
        doc["encPos"] = getEncoderPosition();
        doc["encClk"] = getRawEncoderClk();
        doc["encDt"] = getRawEncoderDt();
        doc["encBtn"] = getRawEncoderButton();
        
        // Cadence
        doc["cadenceRpm"] = getCadenceRPM();
        doc["cadencePin"] = getRawCadencePin();
        doc["cadenceLast"] = getLastCadencePulseTime() / 1000; // Convert to ms
        
        // Emergency stop
#ifndef DISABLE_EMERGENCY_PIN
        doc["emergency"] = digitalRead(EMERGENCY_PIN);
#else
        doc["emergency"] = true; // Disabled
#endif
        
        String output;
        serializeJson(doc, output);
        r->send(200, "application/json", output);
    });

    /* ===== Favicon ===== */
    server.on("/favicon.svg", HTTP_GET, [](AsyncWebServerRequest *r) {
        const char* svg = 
            "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'>"
            "<defs>"
            "<linearGradient id='grad' x1='0%' y1='0%' x2='100%' y2='100%'>"
            "<stop offset='0%' style='stop-color:#667eea;stop-opacity:1'/>"
            "<stop offset='100%' style='stop-color:#764ba2;stop-opacity:1'/>"
            "</linearGradient>"
            "</defs>"
            "<circle cx='50' cy='50' r='48' fill='url(#grad)'/>"
            "<path d='M35 60 L35 35 L45 35 L45 50 L55 35 L65 35 L55 50 L65 60 L55 60 L45 50 L45 60 Z' fill='white'/>"
            "<circle cx='50' cy='70' r='3' fill='white'/>"
            "</svg>";
        r->send(200, "image/svg+xml", svg);
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
