# GymBike Web UI - Raspberry Pi Kiosk Fix Summary

## Problem

The GymBike web UI shows a white page when accessed from a Raspberry Pi running Chromium in kiosk mode (480x320 display), even though other web pages work fine.

## Root Causes Identified

1. **Missing CORS headers** - Browser security prevents cross-origin requests
2. **Oversized fonts** - CSS designed for larger screens (72px, 90px fonts)
3. **No error feedback** - JavaScript errors fail silently
4. **Small display constraints** - 480x320 requires optimized layout

## Changes Made

### 1. Web Server Code ([web_server.cpp](src/web_server.cpp))

#### Added CORS Headers

```cpp
// Enable CORS for all responses
DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
```

#### Optimized CSS for 480x320

- Reduced header height: 12px → 8px padding
- Smaller fonts: 72px → 56px, 90px → 64px
- Tighter spacing: 8px → 6px gaps
- Added responsive breakpoint for screens < 500px:
  - Cadence/Resistance: 48px font
  - Header: 14px font
  - Units: 16px font

#### Enhanced JavaScript Error Handling

- Added console logging for debugging
- Visual error messages that appear on screen
- Connection error counter (shows error after 3 failures)
- Better fetch error handling with try/catch

#### Added Error Display Element

```html
<div id="error-msg" class="error-msg">Connection error</div>
```

### 2. Testing Tools Created

#### `test_ui.html`

Standalone HTML file for browser-based testing:

- Enter ESP32 IP address
- Real-time connection testing
- Built-in debug log viewer
- Optimized for 480x320 display
- No server dependencies

**Usage:**

```bash
chromium-browser file:///path/to/test_ui.html
```

#### `test_esp32.py`

Python script for command-line testing:

- Tests basic connectivity
- Validates API responses
- Checks CORS headers
- Verifies JSON format
- Tests favicon availability

**Usage:**

```bash
python3 test_esp32.py 192.168.1.100
```

#### `setup-kiosk.sh`

Automated setup script for Raspberry Pi:

- Tests ESP32 connectivity
- Disables screen blanking
- Configures auto-start on boot
- Creates manual control scripts
- Interactive setup process

**Usage:**

```bash
chmod +x setup-kiosk.sh
./setup-kiosk.sh
```

### 3. Documentation

#### `RASPBERRY_PI_KIOSK.md`

Comprehensive troubleshooting guide covering:

- Step-by-step setup instructions
- Common issues and solutions
- Chromium kiosk configuration
- Screen resolution settings
- Network configuration
- Performance optimization tips
- Debugging tools and techniques

## How to Apply the Fix

### Option 1: Rebuild and Upload Firmware

1. Open the project in PlatformIO/Arduino IDE
2. Build the project (changes are in [web_server.cpp](src/web_server.cpp))
3. Upload to your ESP32
4. The web UI will now include CORS headers and optimized CSS

### Option 2: Test Before Uploading

Use the testing tools to diagnose the current issue:

```bash
# 1. Test connectivity from Raspberry Pi
python3 test_esp32.py <ESP32_IP>

# 2. Test with standalone HTML
chromium-browser file:///path/to/test_ui.html
# Enter ESP32 IP and click "Connect"
# Check the log for errors

# 3. Check browser console
chromium-browser http://<ESP32_IP>/
# Press F12, check Console and Network tabs
```

### Option 3: Automated Setup

Transfer files to Raspberry Pi and run setup:

```bash
# Copy files to Raspberry Pi
scp setup-kiosk.sh test_esp32.py RASPBERRY_PI_KIOSK.md pi@raspberrypi:~/

# SSH to Raspberry Pi
ssh pi@raspberrypi

# Run setup script
chmod +x setup-kiosk.sh
./setup-kiosk.sh
```

## Expected Results

### Before Fix

- ❌ White page in browser
- ❌ CORS errors in console
- ❌ No error feedback
- ❌ Fonts too large for small screen

### After Fix

- ✅ Web UI displays correctly
- ✅ No CORS errors
- ✅ Visual error messages if connection fails
- ✅ Optimized layout for 480x320
- ✅ Console logging for debugging

## Testing Checklist

- [ ] `test_esp32.py` shows all ✓ checks passing
- [ ] `test_ui.html` connects and shows data
- [ ] Browser console shows "GymBike UI Ready"
- [ ] API endpoint returns valid JSON
- [ ] Page renders without white screen
- [ ] All UI elements visible and readable
- [ ] Updates every second with live data

## Quick Troubleshooting

### Still seeing white page?

1. **Check if firmware was updated:**

   ```bash
   curl -I http://<ESP32_IP>/ | grep Access-Control
   ```

   Should show: `Access-Control-Allow-Origin: *`

2. **Check JavaScript console (F12):**

   - Look for errors in Console tab
   - Check Network tab for failed requests
   - Should see "GymBike UI Ready" message

3. **Verify API endpoint:**

   ```bash
   curl http://<ESP32_IP>/api/status
   ```

   Should return JSON with cadence, resistance, etc.

4. **Check ESP32 serial output:**
   - Connect to serial port (115200 baud)
   - Look for errors or warnings
   - Verify web server started

### Common Solutions

- **Clear browser cache:** `rm -rf ~/.cache/chromium`
- **Restart ESP32:** Power cycle the device
- **Check network:** `ping <ESP32_IP>`
- **Try different browser:** Test with Firefox or another browser
- **Use test UI:** Open `test_ui.html` to isolate the issue

## Files Modified

- [src/web_server.cpp](src/web_server.cpp) - Added CORS, optimized CSS, enhanced JS

## Files Created

- [test_ui.html](test_ui.html) - Standalone test interface
- [test_esp32.py](test_esp32.py) - Python connectivity test
- [setup-kiosk.sh](setup-kiosk.sh) - Raspberry Pi setup script
- [RASPBERRY_PI_KIOSK.md](RASPBERRY_PI_KIOSK.md) - Complete documentation
- [KIOSK_FIX_SUMMARY.md](KIOSK_FIX_SUMMARY.md) - This file

## Next Steps

1. **Upload updated firmware** to ESP32
2. **Run setup script** on Raspberry Pi
3. **Test with `test_ui.html`** to verify
4. **Configure auto-start** for kiosk mode
5. **Enjoy your GymBike display!**

## Support

If issues persist after applying these fixes:

1. Run all three test tools and note the results
2. Check the ESP32 serial console output
3. Capture browser console errors (F12 → Console)
4. Verify network connectivity between devices

The most common remaining issue is usually network-related (firewall, wrong IP, different subnets).
