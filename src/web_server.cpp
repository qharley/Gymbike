#include "web_server.h"
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "wifi_manager.h"

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
    "</style></head><body>"
    "<header>" + title + "</header>";
}

static String pageFooter() {
    return "</body></html>";
}

/* ---------- Web server init ---------- */

void webServerInit() {

    /* ===== Root dashboard ===== */
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *r) {
        String html = pageHeader("Gym Bike");

        html += "<div class='card'>";
        html += "<p><b>Status</b></p>";
        html += wifiSTAConnected()
            ? "<p>LAN IP: " + wifiSTAIP() + "</p>"
            : "<p>Not connected to local Wi-Fi</p>";
        html += "<p>AP IP: " + wifiAPIP() + "</p>";
        html += "</div>";

        html += "<div class='card'>";
        html += "<a class='button' href='/wifi'>Wi-Fi Settings</a>";
        html += "<a class='button secondary' href='/update'>Firmware Update</a>";
        html += "</div>";

        html += pageFooter();
        r->send(200, "text/html", html);
    });

    /* ===== Wi-Fi setup page ===== */
    server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *r) {
        String html = pageHeader("Wi-Fi Setup");

        html += "<div class='card'>";
        if (wifiSTAConnected()) {
            html += "<p><b>Connected</b></p>";
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
        html += "<a class='button secondary' href='/'>Back</a>";
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
        r->redirect("/wifi");
    });

    /* ===== Wi-Fi disconnect ===== */
    server.on("/wifi/disconnect", HTTP_POST, [](AsyncWebServerRequest *r) {
        wifiDisconnectSTA();
        r->redirect("/wifi");
    });

    /* ===== Captive portal fallback ===== */
    server.onNotFound([](AsyncWebServerRequest *r) {
        r->redirect("/");
    });

    server.begin();
}
