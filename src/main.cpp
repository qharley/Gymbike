#include <Arduino.h>
#include "config.h"
#include "cadence.h"
#include "servo_control.h"
#include "display.h"

void setup() {
    delay(2000); // Wait for power to stabilize
    Serial.begin(115200);

    cadenceInit();
    servoInit();
    displayInit();


    // Print the pin numbers for verification
    Serial.print("TOUCH_CS pin: ");
    Serial.println(TOUCH_CS);
    Serial.print("TOUCH_IRQ pin: ");
    Serial.println(TOUCH_IRQ);
    Serial.print("TFT_CS pin: ");
    Serial.println(TFT_CS);
    Serial.print("TFT_DC pin: ");
    Serial.println(TFT_DC);
    Serial.print("TFT_RST pin: ");
    Serial.println(TFT_RST);
    Serial.print("TFT_BL pin: ");
    Serial.println(TFT_BL);
    Serial.print("TFT_MOSI pin: ");
    Serial.println(TFT_MOSI);
    Serial.print("TFT_SCLK pin: ");
    Serial.println(TFT_SCLK);
    Serial.print("TFT_MISO pin: ");
    Serial.println(TFT_MISO);
        

    Serial.println("Gym bike controller started");
    
}

void loop() {
    updateCadence();
    float rpm = getCadenceRPM();

    servoControlUpdate(rpm);
    displayUpdate(rpm);
}
