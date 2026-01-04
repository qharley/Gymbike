#!/bin/bash
# GymBike Kiosk Setup Script for Raspberry Pi
# This script helps set up Chromium in kiosk mode for the GymBike display

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "============================================"
echo "GymBike Chromium Kiosk Setup"
echo "============================================"
echo ""

# Check if running on Raspberry Pi
if [ ! -f /proc/device-tree/model ] || ! grep -q "Raspberry Pi" /proc/device-tree/model 2>/dev/null; then
    echo -e "${YELLOW}Warning: This doesn't appear to be a Raspberry Pi${NC}"
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Get ESP32 IP address
echo -e "${GREEN}Step 1: ESP32 Configuration${NC}"
read -p "Enter your ESP32's IP address (e.g., 192.168.1.100): " ESP32_IP

if [ -z "$ESP32_IP" ]; then
    echo -e "${RED}Error: IP address is required${NC}"
    exit 1
fi

# Validate IP format (basic check)
if ! [[ $ESP32_IP =~ ^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$ ]]; then
    echo -e "${RED}Error: Invalid IP address format${NC}"
    exit 1
fi

echo "Using ESP32 IP: $ESP32_IP"
echo ""

# Test connectivity
echo -e "${GREEN}Step 2: Testing Connectivity${NC}"
echo "Pinging ESP32..."
if ping -c 3 "$ESP32_IP" > /dev/null 2>&1; then
    echo -e "${GREEN}✓ ESP32 is reachable${NC}"
else
    echo -e "${RED}✗ Cannot reach ESP32 at $ESP32_IP${NC}"
    echo "Please check:"
    echo "  - ESP32 is powered on"
    echo "  - ESP32 is connected to the same network"
    echo "  - IP address is correct"
    exit 1
fi

echo "Testing HTTP connection..."
if curl -s --connect-timeout 5 "http://$ESP32_IP/" > /dev/null; then
    echo -e "${GREEN}✓ Web server is responding${NC}"
else
    echo -e "${RED}✗ Web server is not responding${NC}"
    exit 1
fi
echo ""

# Disable screen blanking
echo -e "${GREEN}Step 3: Disable Screen Blanking${NC}"
echo "Configuring screen to stay on..."

# Create autostart directory if it doesn't exist
mkdir -p ~/.config/lxsession/LXDE-pi/

# Add screen blanking disable to autostart
if [ ! -f ~/.config/lxsession/LXDE-pi/autostart ]; then
    cat > ~/.config/lxsession/LXDE-pi/autostart << EOF
@lxpanel --profile LXDE-pi
@pcmanfm --desktop --profile LXDE-pi
@xscreensaver -no-splash
EOF
fi

# Add our settings if not already present
if ! grep -q "xset s off" ~/.config/lxsession/LXDE-pi/autostart; then
    cat >> ~/.config/lxsession/LXDE-pi/autostart << EOF
@xset s off
@xset -dpms
@xset s noblank
EOF
    echo -e "${GREEN}✓ Screen blanking disabled${NC}"
else
    echo -e "${YELLOW}Screen blanking already configured${NC}"
fi
echo ""

# Create kiosk autostart
echo -e "${GREEN}Step 4: Configure Chromium Kiosk Mode${NC}"

# Create autostart directory if needed
mkdir -p ~/.config/autostart/

# Copy the startup script and configure it
if [ -f ~/gymbike-kiosk-startup.sh ]; then
    # Replace placeholder with actual IP
    sed "s/__ESP32_IP__/$ESP32_IP/g" ~/gymbike-kiosk-startup.sh > ~/gymbike-kiosk-startup-configured.sh
    chmod +x ~/gymbike-kiosk-startup-configured.sh
    
    # Create the desktop entry that calls our startup script
    cat > ~/.config/autostart/gymbike-kiosk.desktop << EOF
[Desktop Entry]
Type=Application
Name=GymBike Kiosk
Comment=Auto-start GymBike web interface in kiosk mode with network waiting
NoDisplay=false
Exec=$HOME/gymbike-kiosk-startup-configured.sh
X-GNOME-Autostart-enabled=true
EOF
    
    echo -e "${GREEN}✓ Kiosk mode configured with network waiting${NC}"
    echo "  Startup script: ~/gymbike-kiosk-startup-configured.sh"
    echo "  Autostart file: ~/.config/autostart/gymbike-kiosk.desktop"
    echo "  Log file will be: ~/gymbike-kiosk.log"
else
    echo -e "${RED}✗ Warning: gymbike-kiosk-startup.sh not found${NC}"
    echo "  Creating simple autostart without network waiting..."
    
    # Fallback to simple version with delay
    cat > ~/.config/autostart/gymbike-kiosk.desktop << EOF
[Desktop Entry]
Type=Application
Name=GymBike Kiosk
Comment=Auto-start GymBike web interface in kiosk mode
NoDisplay=false
Exec=sh -c 'sleep 30 && chromium-browser --kiosk --no-first-run --disable-infobars --disable-session-crashed-bubble --disable-suggestions-service --disable-translate --disable-save-password-bubble --incognito --window-position=0,0 --window-size=480,320 http://$ESP32_IP/'
X-GNOME-Autostart-enabled=true
EOF
    
    echo -e "${YELLOW}✓ Simple autostart configured (30s delay)${NC}"
    echo "  Note: Transfer gymbike-kiosk-startup.sh and re-run for better startup"
fi

echo ""

# Create manual start script
echo -e "${GREEN}Step 5: Create Manual Start Script${NC}"

cat > ~/start-gymbike.sh << 'EOF'
#!/bin/bash
# Manual start script for GymBike kiosk mode

# Kill any existing Chromium instances
killall chromium-browser 2>/dev/null || true
sleep 1

# Disable screen blanking
xset s off
xset -dpms
xset s noblank

# Start Chromium in kiosk mode
EOF

echo "chromium-browser --kiosk --no-first-run --disable-infobars --disable-session-crashed-bubble --disable-suggestions-service --disable-translate --disable-save-password-bubble --incognito --window-position=0,0 --window-size=480,320 http://$ESP32_IP/" >> ~/start-gymbike.sh

chmod +x ~/start-gymbike.sh

echo -e "${GREEN}✓ Manual start script created${NC}"
echo "  Script: ~/start-gymbike.sh"
echo ""

# Create stop script
cat > ~/stop-gymbike.sh << 'EOF'
#!/bin/bash
# Stop the GymBike kiosk

killall chromium-browser 2>/dev/null
echo "GymBike kiosk stopped"
EOF

chmod +x ~/stop-gymbike.sh

echo -e "${GREEN}✓ Stop script created${NC}"
echo "  Script: ~/stop-gymbike.sh"
echo ""

# Summary
echo "============================================"
echo -e "${GREEN}Setup Complete!${NC}"
echo "============================================"
echo ""
echo "What's been configured:"
echo "  ✓ Screen blanking disabled"
echo "  ✓ Auto-start on boot enabled"
echo "  ✓ Manual control scripts created"
echo ""
echo "Manual Controls:"
echo "  Start:  ./start-gymbike.sh"
echo "  Stop:   ./stop-gymbike.sh"
echo ""
echo "To test now (without rebooting):"
echo "  ./start-gymbike.sh"
echo ""
echo "To exit kiosk mode:"
echo "  Press Alt+F4 or Ctrl+W"
echo ""
echo "Auto-start will take effect on next boot."
echo ""

# Ask to start now
read -p "Start GymBike kiosk now? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Starting kiosk mode..."
    ./start-gymbike.sh &
    echo -e "${GREEN}Kiosk mode started!${NC}"
else
    echo "Run ./start-gymbike.sh when ready"
fi

echo ""
echo "For troubleshooting, see RASPBERRY_PI_KIOSK.md"
