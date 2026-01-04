#!/bin/bash
# GymBike Kiosk Startup Script with Network Waiting
# This script waits for network and ESP32 to be ready before starting the kiosk

LOG_FILE="$HOME/gymbike-kiosk.log"

# Function to log messages
log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

log "=== GymBike Kiosk Startup ==="

# Get ESP32 IP from command line or use default
ESP32_IP="${1:-__ESP32_IP__}"
MAX_WAIT=120  # Maximum wait time in seconds
WAIT_COUNT=0

log "Target ESP32: $ESP32_IP"
log "Max wait time: ${MAX_WAIT}s"

# Wait for X server to be ready
log "Waiting for X server..."
while [ -z "$DISPLAY" ] && [ $WAIT_COUNT -lt 30 ]; do
    sleep 1
    WAIT_COUNT=$((WAIT_COUNT + 1))
    export DISPLAY=:0
done

if [ -z "$DISPLAY" ]; then
    log "ERROR: DISPLAY not set after 30s"
    export DISPLAY=:0
fi

log "DISPLAY is set to: $DISPLAY"

# Additional wait for X to fully initialize
sleep 5

# Configure display settings
log "Configuring display..."
xset s off 2>&1 | tee -a "$LOG_FILE"
xset -dpms 2>&1 | tee -a "$LOG_FILE"
xset s noblank 2>&1 | tee -a "$LOG_FILE"

# Wait for network to be up
log "Waiting for network interface..."
WAIT_COUNT=0
while ! ip route | grep -q default && [ $WAIT_COUNT -lt 30 ]; do
    sleep 1
    WAIT_COUNT=$((WAIT_COUNT + 1))
done

if ! ip route | grep -q default; then
    log "WARNING: No default route after 30s"
else
    log "Network interface is up"
fi

# Wait for network connectivity
log "Waiting for network connectivity..."
WAIT_COUNT=0
while ! ping -c 1 -W 2 8.8.8.8 > /dev/null 2>&1 && [ $WAIT_COUNT -lt 30 ]; do
    sleep 2
    WAIT_COUNT=$((WAIT_COUNT + 2))
done

if ping -c 1 -W 2 8.8.8.8 > /dev/null 2>&1; then
    log "Network connectivity confirmed"
else
    log "WARNING: No internet connectivity, but continuing..."
fi

# Wait for ESP32 to be reachable
log "Waiting for ESP32 at $ESP32_IP..."
WAIT_COUNT=0
ESP32_READY=false

while [ $WAIT_COUNT -lt $MAX_WAIT ]; do
    if ping -c 1 -W 2 "$ESP32_IP" > /dev/null 2>&1; then
        log "ESP32 is reachable via ping"
        
        # Check if web server is responding
        if curl -s --connect-timeout 3 "http://$ESP32_IP/" > /dev/null 2>&1; then
            log "ESP32 web server is responding"
            ESP32_READY=true
            break
        else
            log "ESP32 responds to ping but web server not ready yet..."
        fi
    fi
    
    sleep 2
    WAIT_COUNT=$((WAIT_COUNT + 2))
    
    if [ $((WAIT_COUNT % 10)) -eq 0 ]; then
        log "Still waiting... (${WAIT_COUNT}s elapsed)"
    fi
done

if [ "$ESP32_READY" = false ]; then
    log "ERROR: ESP32 not ready after ${MAX_WAIT}s"
    log "Attempting to start anyway..."
fi

# Kill any existing Chromium instances
log "Killing any existing Chromium instances..."
killall chromium-browser 2>&1 | tee -a "$LOG_FILE"
sleep 2

# Clear any crash flags
log "Clearing Chromium crash flags..."
rm -f ~/.config/chromium/SingletonLock 2>&1 | tee -a "$LOG_FILE"
rm -f ~/.config/chromium/SingletonCookie 2>&1 | tee -a "$LOG_FILE"
rm -f ~/.config/chromium/SingletonSocket 2>&1 | tee -a "$LOG_FILE"

# Start Chromium in kiosk mode
log "Starting Chromium kiosk mode..."
chromium-browser \
    --kiosk \
    --no-first-run \
    --disable-infobars \
    --disable-session-crashed-bubble \
    --disable-suggestions-service \
    --disable-translate \
    --disable-save-password-bubble \
    --disable-sync \
    --disable-background-networking \
    --incognito \
    --window-position=0,0 \
    --window-size=480,320 \
    "http://$ESP32_IP/" \
    >> "$LOG_FILE" 2>&1 &

CHROME_PID=$!
log "Chromium started with PID: $CHROME_PID"

# Wait a moment and verify it's running
sleep 3
if ps -p $CHROME_PID > /dev/null; then
    log "SUCCESS: Chromium is running"
else
    log "ERROR: Chromium failed to start or crashed immediately"
    log "Check log file: $LOG_FILE"
fi

log "=== Startup script complete ==="
