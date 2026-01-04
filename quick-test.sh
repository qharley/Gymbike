#!/bin/bash
# Quick test to see what the ESP32 is serving

if [ -z "$1" ]; then
    echo "Usage: ./quick-test.sh <ESP32_IP>"
    echo "Example: ./quick-test.sh 192.168.1.100"
    exit 1
fi

ESP32_IP=$1

echo "========================================"
echo "Quick ESP32 Web Server Test"
echo "========================================"
echo ""

# Test 1: What does the root page return?
echo "1. Testing root page (/)..."
echo "---"
curl -v "http://$ESP32_IP/" 2>&1 | grep -E "(HTTP|Location|Content-Type|Access-Control)"
echo ""

# Test 2: Is it redirecting?
echo "2. Following redirects..."
echo "---"
FINAL_URL=$(curl -Ls -o /dev/null -w %{url_effective} "http://$ESP32_IP/")
echo "Final URL: $FINAL_URL"
echo ""

# Test 3: What page are we actually getting?
echo "3. Getting actual content..."
echo "---"
CONTENT=$(curl -sL "http://$ESP32_IP/" | head -20)
echo "$CONTENT"
echo ""
echo "..."
echo ""

# Test 4: Check if it's the settings page
if echo "$CONTENT" | grep -q "Wi-Fi Connection"; then
    echo "❌ ISSUE FOUND: ESP32 is showing SETTINGS page, not main page!"
    echo ""
    echo "This means WiFi credentials are not saved on the ESP32."
    echo ""
    echo "Solutions:"
    echo "1. Connect to ESP32 and save WiFi credentials:"
    echo "   chromium-browser http://$ESP32_IP/settings"
    echo "   Enter your WiFi SSID and password"
    echo ""
    echo "2. Or disable the redirect in web_server.cpp:"
    echo "   Comment out the 'if (!wifiHasSavedCredentials())' check"
    echo ""
elif echo "$CONTENT" | grep -q "Gym Bike"; then
    echo "✓ Page looks correct - showing Gym Bike interface"
    echo ""
    echo "If you're still seeing a blank page in kiosk mode:"
    echo "1. Clear browser cache: rm -rf ~/.cache/chromium"
    echo "2. Check JavaScript console (F12) for errors"
    echo "3. Run diagnostic.html to see detailed errors"
else
    echo "❓ Unknown content received"
fi

echo ""
echo "========================================"
