#include "captive_portal.h"
#include <DNSServer.h>
#include <WiFi.h>

DNSServer dnsServer;
const byte DNS_PORT = 53;

void captivePortalInit() {
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
}

void captivePortalLoop() {
    dnsServer.processNextRequest();
}
