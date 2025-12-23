#include <TFT_eSPI.h>
#include "display.h"

static TFT_eSPI tft = TFT_eSPI();

void displayInit() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);

    tft.drawString("GYM BIKE PROTOTYPE", 40, 10);
    tft.drawString("CADENCE:", 40, 80);
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
