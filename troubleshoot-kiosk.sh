#!/bin/bash
# GymBike Kiosk Troubleshooting Script
# Run this on the Raspberry Pi when you have a blank page issue

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo "========================================"
echo "GymBike Kiosk Troubleshooting"
echo "========================================"
echo ""

# Get ESP32 IP from autostart file if it exists
ESP32_IP=""
if [ -f ~/.config/autostart/gymbike-kiosk.desktop ]; then
    ESP32_IP=$(grep -oP 'http://\K[0-9.]+' ~/.config/autostart/gymbike-kiosk.desktop | head -1)
fi

if [ -z "$ESP32_IP" ]; then
    read -p "Enter ESP32 IP address: " ESP32_IP
fi

echo -e "${BLUE}Testing ESP32 at: $ESP32_IP${NC}"
echo ""

# Test 1: Network connectivity
echo -e "${YELLOW}1. Testing network connectivity...${NC}"
if ping -c 2 -W 2 "$ESP32_IP" > /dev/null 2>&1; then
    echo -e "${GREEN}   ✓ ESP32 is reachable${NC}"
else
    echo -e "${RED}   ✗ Cannot ping ESP32${NC}"
    echo "   Possible issues:"
    echo "   - ESP32 is not powered on"
    echo "   - Wrong IP address"
    echo "   - Not on same network"
    exit 1
fi

# Test 2: HTTP connection
echo -e "${YELLOW}2. Testing HTTP connection...${NC}"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" "http://$ESP32_IP/" --connect-timeout 5)
if [ "$HTTP_CODE" = "200" ]; then
    echo -e "${GREEN}   ✓ Web server responding (HTTP $HTTP_CODE)${NC}"
else
    echo -e "${RED}   ✗ Web server not responding (HTTP $HTTP_CODE)${NC}"
    exit 1
fi

# Test 3: CORS headers
echo -e "${YELLOW}3. Checking CORS headers...${NC}"
CORS=$(curl -s -I "http://$ESP32_IP/" | grep -i "access-control-allow-origin")
if [ ! -z "$CORS" ]; then
    echo -e "${GREEN}   ✓ CORS headers present${NC}"
    echo "   $CORS"
else
    echo -e "${RED}   ✗ No CORS headers found${NC}"
    echo "   This will cause browser blocking!"
    echo "   Solution: Re-upload firmware with updated web_server.cpp"
fi

# Test 4: API endpoint
echo -e "${YELLOW}4. Testing API endpoint...${NC}"
API_RESPONSE=$(curl -s "http://$ESP32_IP/api/status")
if echo "$API_RESPONSE" | jq . > /dev/null 2>&1; then
    echo -e "${GREEN}   ✓ API returns valid JSON${NC}"
    echo "   Sample data:"
    echo "$API_RESPONSE" | jq '.' | head -5
else
    echo -e "${RED}   ✗ API not returning valid JSON${NC}"
    echo "   Response: $API_RESPONSE"
fi

# Test 5: Chromium processes
echo -e "${YELLOW}5. Checking Chromium processes...${NC}"
CHROME_COUNT=$(pgrep -c chromium || echo "0")
if [ "$CHROME_COUNT" -gt 0 ]; then
    echo -e "${GREEN}   ✓ Chromium is running ($CHROME_COUNT processes)${NC}"
else
    echo -e "${RED}   ✗ Chromium is not running${NC}"
fi

# Test 6: Display environment
echo -e "${YELLOW}6. Checking display environment...${NC}"
if [ ! -z "$DISPLAY" ]; then
    echo -e "${GREEN}   ✓ DISPLAY set to: $DISPLAY${NC}"
else
    echo -e "${RED}   ✗ DISPLAY not set${NC}"
    echo "   Kiosk mode requires X server"
fi

# Test 7: Browser cache
echo -e "${YELLOW}7. Checking browser cache...${NC}"
CACHE_SIZE=$(du -sh ~/.cache/chromium 2>/dev/null | cut -f1)
if [ ! -z "$CACHE_SIZE" ]; then
    echo -e "${YELLOW}   Cache size: $CACHE_SIZE${NC}"
    read -p "   Clear cache? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        killall chromium-browser 2>/dev/null
        rm -rf ~/.cache/chromium
        echo -e "${GREEN}   ✓ Cache cleared${NC}"
    fi
fi

# Test 8: Load diagnostic page
echo ""
echo -e "${YELLOW}8. Running diagnostic page...${NC}"
echo "   Opening diagnostic.html in browser..."
echo "   Check the output on screen."
echo ""

if [ -f ~/diagnostic.html ]; then
    chromium-browser --new-window "file://$HOME/diagnostic.html" &
    sleep 3
else
    echo -e "${RED}   ✗ diagnostic.html not found${NC}"
    echo "   Copy diagnostic.html to $HOME/"
fi

# Test 9: Try minimal test
echo -e "${YELLOW}9. Opening minimal test page...${NC}"
if [ -f ~/minimal_test.html ]; then
    chromium-browser --new-window "file://$HOME/minimal_test.html" &
    sleep 3
else
    echo -e "${YELLOW}   minimal_test.html not found (skipping)${NC}"
fi

# Summary and recommendations
echo ""
echo "========================================"
echo "Troubleshooting Summary"
echo "========================================"
echo ""
echo "Common solutions for blank page:"
echo ""
echo -e "${GREEN}1. Clear cache and restart:${NC}"
echo "   killall chromium-browser"
echo "   rm -rf ~/.cache/chromium"
echo "   ./start-gymbike.sh"
echo ""
echo -e "${GREEN}2. Test without kiosk mode:${NC}"
echo "   chromium-browser http://$ESP32_IP/"
echo "   (Press F12 to see console errors)"
echo ""
echo -e "${GREEN}3. Check JavaScript console:${NC}"
echo "   Open without --kiosk flag"
echo "   Press F12 → Console tab"
echo "   Look for red errors"
echo ""
echo -e "${GREEN}4. Try the minimal test:${NC}"
echo "   chromium-browser file://$HOME/minimal_test.html"
echo ""
echo -e "${GREEN}5. Verify firmware was updated:${NC}"
echo "   Check that CORS headers are present (see test #3 above)"
echo "   If not, re-upload the firmware"
echo ""
echo -e "${GREEN}6. Check browser compatibility:${NC}"
echo "   Run: chromium-browser --version"
echo "   Ensure JavaScript is enabled"
echo ""
echo -e "${YELLOW}Still having issues?${NC}"
echo "1. Check results from diagnostic.html"
echo "2. Share any errors from console (F12)"
echo "3. Run: journalctl -xe | grep chromium"
echo ""
