# GymBike Raspberry Pi Chromium Kiosk Setup

This guide helps you display the GymBike web UI on a Raspberry Pi with a 480x320 display using Chromium in kiosk mode.

## Changes Made

### 1. Web Server Improvements

- **Added CORS headers** to allow cross-origin requests
- **Optimized CSS** for 480x320 displays with smaller fonts and padding
- **Added error handling** with visual feedback for connection issues
- **Improved JavaScript logging** for easier debugging

### 2. Testing Tools

Three new files have been created to help diagnose issues:

#### `test_ui.html`

A standalone HTML file to test the connection from your browser:

- Open this file in Chromium on your Raspberry Pi
- Enter the ESP32's IP address
- Click "Connect" to test the connection
- Use the "Log" button to see detailed connection information

#### `test_esp32.py`

A Python script to test the ESP32 web server:

```bash
python3 test_esp32.py 192.168.1.100
```

This will check:

- Basic connectivity
- API endpoint responses
- CORS headers
- Favicon availability

## Troubleshooting Steps

### Step 1: Find Your ESP32's IP Address

1. Connect to the ESP32's serial port (115200 baud)
2. Watch the boot messages for the IP address
3. Or check your router's DHCP client list

### Step 2: Test Connectivity from Raspberry Pi

```bash
# Install required Python package
pip3 install requests

# Test the connection
python3 test_esp32.py <ESP32_IP>
```

### Step 3: Test with the HTML File

```bash
# On Raspberry Pi, open the test UI
chromium-browser --kiosk file:///path/to/test_ui.html
```

Enter the ESP32 IP and click Connect. Watch the log for errors.

### Step 4: Check Chromium Console

If the page is still white:

1. Exit kiosk mode (Ctrl+W or Alt+F4)
2. Open Chromium normally: `chromium-browser`
3. Navigate to `http://<ESP32_IP>/`
4. Press F12 to open Developer Tools
5. Check the Console tab for JavaScript errors
6. Check the Network tab to see if API calls are failing

### Common Issues and Solutions

#### Issue: White page, no errors in console

**Solution:** The ESP32 might be redirecting to `/settings`. Check if WiFi credentials are saved.

#### Issue: CORS errors in console

**Solution:** The code now includes CORS headers. Recompile and upload the firmware.

#### Issue: "Failed to fetch" errors

**Solutions:**

- Verify the ESP32 is on the same network as the Raspberry Pi
- Check firewall rules
- Try pinging the ESP32: `ping <ESP32_IP>`

#### Issue: Page loads but shows "--" for all values

**Solution:** The API endpoint isn't responding. Check:

- ESP32 serial output for errors
- `/api/status` URL directly in browser
- Network connectivity

#### Issue: Fonts too large for 480x320 screen

**Solution:** The updated CSS includes responsive sizing. Clear browser cache:

```bash
rm -rf ~/.cache/chromium
```

## Chromium Kiosk Mode Setup

### Basic Kiosk Command

```bash
chromium-browser \
  --kiosk \
  --no-first-run \
  --disable-infobars \
  --disable-session-crashed-bubble \
  --disable-suggestions-service \
  --disable-translate \
  --disable-save-password-bubble \
  --incognito \
  http://<ESP32_IP>/
```

### Auto-start on Boot

Create `/home/pi/.config/autostart/gymbike.desktop`:

```ini
[Desktop Entry]
Type=Application
Name=GymBike Kiosk
Exec=chromium-browser --kiosk --no-first-run --disable-infobars http://192.168.1.100/
X-GNOME-Autostart-enabled=true
```

### Disable Screen Blanking

Add to `/home/pi/.xinitrc` or `/etc/xdg/lxsession/LXDE-pi/autostart`:

```bash
@xset s off
@xset -dpms
@xset s noblank
```

## Screen Resolution for 480x320

### For Waveshare 3.5" Display

Edit `/boot/config.txt`:

```ini
# Disable default HDMI settings
hdmi_force_hotplug=1
hdmi_group=2
hdmi_mode=87
hdmi_cvt=480 320 60 1 0 0 0

# Rotate if needed (0, 90, 180, 270)
display_rotate=0
```

### For Other SPI Displays

Follow the manufacturer's instructions, but ensure the resolution is set to 480x320.

## Network Configuration

### Static IP for ESP32 (Recommended)

In your router, assign a static DHCP lease to the ESP32's MAC address. This ensures the IP doesn't change.

### mDNS (Alternative)

If your ESP32 supports mDNS, you can use `gymbike.local` instead of an IP address.

## Performance Tips

1. **Disable unnecessary Chromium features:**

   - Add `--disable-extensions` flag
   - Add `--disable-gpu` if rendering issues occur

2. **Optimize Raspberry Pi:**

   ```bash
   # Increase GPU memory (in /boot/config.txt)
   gpu_mem=128

   # Overclock if needed (Pi 3/4)
   arm_freq=1500
   ```

3. **Reduce refresh rate:**
   The UI updates every second. This can be changed in the JavaScript if needed.

## Debugging Tools

### View Live Logs from ESP32

```bash
# Using PlatformIO
pio device monitor

# Or using screen
screen /dev/ttyUSB0 115200
```

### Check Network Connectivity

```bash
# Ping ESP32
ping <ESP32_IP>

# Check open ports
nmap <ESP32_IP>

# Test HTTP connection
curl -v http://<ESP32_IP>/
curl -v http://<ESP32_IP>/api/status
```

### Browser Developer Tools

```bash
# Start Chromium with remote debugging
chromium-browser --remote-debugging-port=9222 http://<ESP32_IP>/
```

Then access from another device: `http://<RASPBERRY_PI_IP>:9222`

## Next Steps

Once you have the hardware display for the ESP32, you can:

1. Keep using the Raspberry Pi as a secondary display
2. Switch to the ESP32's built-in display
3. Use both simultaneously

The web interface will continue to work regardless of the hardware display implementation.

## Need Help?

If you're still seeing a white page:

1. Run `test_esp32.py` and share the output
2. Open `test_ui.html` and check the log
3. Check the ESP32 serial console for errors
4. Open Chromium DevTools (F12) and check Console/Network tabs

Common signs of success:

- `test_esp32.py` shows all ✓ checks passing
- `test_ui.html` shows "Data received" in the log
- Browser console shows "GymBike UI Ready"
- API endpoint returns JSON: `curl http://<ESP32_IP>/api/status`
