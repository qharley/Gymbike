#pragma once
#include <WiFi.h>

void wifiInit();
void wifiConnect(const String& ssid, const String& pass);
void wifiDisconnectSTA();

bool wifiSTAConnected();
String wifiSTAIP();
String wifiAPIP();
