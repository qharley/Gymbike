#pragma once

void encoderInit();
void encoderLoop();
int getEncoderDelta();  // Returns change since last call
bool encoderButtonPressed();  // Returns true if button was pressed

// Raw state getters for troubleshooting
int getEncoderPosition();
bool getRawEncoderClk();
bool getRawEncoderDt();
bool getRawEncoderButton();
