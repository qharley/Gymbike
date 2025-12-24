#include <TFT_eSPI.h>
#include "display.h"

static TFT_eSPI tft = TFT_eSPI();

void displayInit() {
    Serial.println("[DISPLAY] init() start");

    tft.init();
    Serial.println("[DISPLAY] init() done");

    tft.setRotation(1);
    Serial.println("[DISPLAY] rotation set");

    tft.fillScreen(TFT_RED);
    Serial.println("[DISPLAY] fill RED");

    delay(500);
    tft.fillScreen(TFT_GREEN);
    Serial.println("[DISPLAY] fill GREEN");

    delay(500);
    tft.fillScreen(TFT_BLUE);
    Serial.println("[DISPLAY] fill BLUE");

    delay(500);
    tft.fillScreen(TFT_BLACK);
    Serial.println("[DISPLAY] fill BLACK");

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("DISPLAY OK", 40, 40);

    Serial.println("[DISPLAY] text drawn");
}


void displayUpdate(float cadenceRPM) {
    static int lastRPM = -1;
    int rpm = (int)cadenceRPM;

    if (rpm == lastRPM) return;
    lastRPM = rpm;

    tft.fillRect(40, 120, 200, 40, TFT_BLACK);
    tft.drawNumber(rpm, 40, 120);
    tft.drawString("RPM", 110, 120);
}

