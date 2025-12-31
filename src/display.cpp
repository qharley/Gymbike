#include "display.h"
#include "config.h"
#include "control.h"
#include "cadence.h"
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

// Color definitions
#define COLOR_BG 0x1082        // Dark background
#define COLOR_CARD 0x2945      // Card background
#define COLOR_TEXT 0xFFFF      // White text
#define COLOR_LABEL 0x9CD3     // Gray labels
#define COLOR_STOPPED 0x9CD3   // Gray
#define COLOR_RUNNING 0x4FE8   // Green
#define COLOR_RESTING 0xFE60   // Amber/Yellow
#define COLOR_PRIMARY 0x667E   // Purple-ish

// Layout constants
#define MARGIN 10
#define CARD_RADIUS 12
#define CARD_PADDING 16

// Previous values for change detection
static int prevCadence = -1;
static int prevResistance = -1;
static int prevWorkoutTime = -1;
static WorkoutState prevWorkoutState = WORKOUT_STOPPED;
static ControlMode prevControlMode = MODE_MANUAL;
static int prevTargetCadence = -1;
static int prevTargetWatts = -1;

// Helper function to draw a rounded rectangle card
void drawCard(int x, int y, int w, int h) {
    tft.fillRoundRect(x, y, w, h, CARD_RADIUS, COLOR_CARD);
}

// Helper function to format time
String formatTime(unsigned long seconds) {
    int h = seconds / 3600;
    int m = (seconds % 3600) / 60;
    int s = seconds % 60;
    
    char buffer[16];
    if (h > 0) {
        sprintf(buffer, "%d:%02d:%02d", h, m, s);
    } else {
        sprintf(buffer, "%d:%02d", m, s);
    }
    return String(buffer);
}

void displayInit() {
    tft.init();
    tft.setRotation(TFT_ROTATION);
    tft.fillScreen(COLOR_BG);
    
    // Enable backlight if defined
    #ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    #endif
    
    // Draw initial layout
    displayClear();
}

void displayClear() {
    tft.fillScreen(COLOR_BG);
    
    // Reset previous values to force full redraw
    prevCadence = -1;
    prevResistance = -1;
    prevWorkoutTime = -1;
    prevWorkoutState = WORKOUT_STOPPED;
    prevControlMode = MODE_MANUAL;
    prevTargetCadence = -1;
    prevTargetWatts = -1;
}

void displayUpdate() {
    // Get current data
    int currentCadence = getCadenceRPM();
    int currentPos = getServoPosition();
    int resistance = map(currentPos, SERVO_MIN, SERVO_MAX, 0, 100);
    resistance = constrain(resistance, 0, 100);
    
    // Calculate workout time
    unsigned long currentTime = millis();
    unsigned long totalElapsed = workoutElapsedTime;
    unsigned long restTime = 0;
    
    if (workoutState == WORKOUT_RUNNING) {
        totalElapsed += (currentTime - workoutStartTime);
    } else if (workoutState == WORKOUT_RESTING) {
        restTime = (currentTime - restStartTime) / 1000;
    }
    
    int workoutTime = totalElapsed / 1000;
    
    // Layout calculations
    int cardWidth = (TFT_WIDTH - MARGIN * 3) / 2;
    int cardHeight = 110;
    
    // ---------- Workout Status Card (Top, Full Width) ----------
    int statusCardY = MARGIN;
    int statusCardHeight = 140;
    
    if (prevWorkoutState != workoutState || prevWorkoutTime != workoutTime) {
        drawCard(MARGIN, statusCardY, TFT_WIDTH - MARGIN * 2, statusCardHeight);
        
        // Label
        tft.setTextColor(COLOR_LABEL);
        tft.setTextDatum(TC_DATUM);
        tft.drawString("WORKOUT STATUS", TFT_WIDTH / 2, statusCardY + CARD_PADDING, 2);
        
        // Status badge
        const char* stateText[] = {"STOPPED", "RUNNING", "RESTING"};
        uint16_t stateColors[] = {COLOR_STOPPED, COLOR_RUNNING, COLOR_RESTING};
        
        int badgeY = statusCardY + 40;
        int badgeW = 140;
        int badgeH = 36;
        int badgeX = (TFT_WIDTH - badgeW) / 2;
        
        tft.fillRoundRect(badgeX, badgeY, badgeW, badgeH, 18, stateColors[workoutState]);
        tft.setTextColor(COLOR_BG);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(stateText[workoutState], TFT_WIDTH / 2, badgeY + badgeH / 2, 4);
        
        // Timer
        tft.setTextColor(COLOR_TEXT);
        tft.setTextDatum(TC_DATUM);
        String timeStr = formatTime(workoutTime);
        tft.drawString(timeStr, TFT_WIDTH / 2, statusCardY + 90, 7);
        
        // Rest info if resting
        if (workoutState == WORKOUT_RESTING && restTime > 0) {
            tft.setTextColor(COLOR_LABEL);
            String restStr = "Rest: " + formatTime(restTime);
            tft.drawString(restStr, TFT_WIDTH / 2, statusCardY + 125, 2);
        }
        
        prevWorkoutState = workoutState;
        prevWorkoutTime = workoutTime;
    }
    
    // ---------- Cadence Card (Left) ----------
    int row2Y = statusCardY + statusCardHeight + MARGIN;
    
    if (prevCadence != currentCadence) {
        drawCard(MARGIN, row2Y, cardWidth, cardHeight);
        
        tft.setTextColor(COLOR_LABEL);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("CADENCE", MARGIN + CARD_PADDING, row2Y + CARD_PADDING, 2);
        
        tft.setTextColor(COLOR_TEXT);
        tft.setTextDatum(MC_DATUM);
        String cadenceStr = String(currentCadence);
        tft.drawString(cadenceStr, MARGIN + cardWidth / 2, row2Y + 60, 7);
        
        tft.setTextColor(COLOR_LABEL);
        tft.drawString("rpm", MARGIN + cardWidth / 2, row2Y + 95, 2);
        
        prevCadence = currentCadence;
    }
    
    // ---------- Resistance Card (Right) ----------
    if (prevResistance != resistance) {
        int rightCardX = MARGIN * 2 + cardWidth;
        drawCard(rightCardX, row2Y, cardWidth, cardHeight);
        
        tft.setTextColor(COLOR_LABEL);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("RESISTANCE", rightCardX + CARD_PADDING, row2Y + CARD_PADDING, 2);
        
        tft.setTextColor(COLOR_TEXT);
        tft.setTextDatum(MC_DATUM);
        String resistStr = String(resistance) + "%";
        tft.drawString(resistStr, rightCardX + cardWidth / 2, row2Y + 60, 7);
        
        tft.setTextColor(COLOR_LABEL);
        String servoStr = "Servo: " + String(currentPos) + "\xB0";
        tft.drawString(servoStr, rightCardX + cardWidth / 2, row2Y + 95, 2);
        
        prevResistance = resistance;
    }
    
    // ---------- Control Mode Card (Bottom, Full Width) ----------
    int row3Y = row2Y + cardHeight + MARGIN;
    int modeCardHeight = 100;
    
    bool modeChanged = (prevControlMode != controlMode || 
                       prevTargetCadence != targetCadence || 
                       prevTargetWatts != targetWatts);
    
    if (modeChanged) {
        drawCard(MARGIN, row3Y, TFT_WIDTH - MARGIN * 2, modeCardHeight);
        
        tft.setTextColor(COLOR_LABEL);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("CONTROL MODE", MARGIN + CARD_PADDING, row3Y + CARD_PADDING, 2);
        
        const char* modeText[] = {"MANUAL", "CADENCE", "ERG"};
        tft.setTextColor(COLOR_PRIMARY);
        tft.setTextDatum(TC_DATUM);
        tft.drawString(modeText[controlMode], TFT_WIDTH / 2, row3Y + 45, 4);
        
        // Target info
        tft.setTextColor(COLOR_TEXT);
        String targetStr = "";
        if (controlMode == MODE_CADENCE) {
            targetStr = "Target: " + String(targetCadence) + " rpm";
        } else if (controlMode == MODE_ERG) {
            targetStr = "Target: " + String(targetWatts) + " W";
        }
        
        if (targetStr.length() > 0) {
            tft.drawString(targetStr, TFT_WIDTH / 2, row3Y + 75, 2);
        }
        
        prevControlMode = controlMode;
        prevTargetCadence = targetCadence;
        prevTargetWatts = targetWatts;
    }
}
