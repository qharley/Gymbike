# 🔧 Blank Page Troubleshooting Guide

## You're Here Because...

The kiosk shows a blank/white page even though `test_esp32.py` works.

## Most Common Cause ⚠️

**The ESP32 is redirecting to the settings page because WiFi credentials aren't saved.**

Even though the ESP32 is connected to WiFi and serving pages, if the credentials aren't stored in flash memory, `wifiHasSavedCredentials()` returns false and redirects all requests to `/settings`.

---

## Quick Diagnosis (Run These on Raspberry Pi)

### 1. Quick Test Script

```bash
chmod +x quick-test.sh
./quick-test.sh <ESP32_IP>
```

This will tell you immediately if you're being redirected to settings.

### 2. Check What Page Loads

```bash
curl http://<ESP32_IP>/ | head -20
```

**If you see "Wi-Fi Connection" or "Settings"** → That's your problem!

**If you see "Gym Bike"** → Different issue, continue below.

---

## Solution 1: Save WiFi Credentials on ESP32

### From Raspberry Pi:

```bash
# Stop kiosk if running
./stop-gymbike.sh

# Open browser normally (not kiosk)
chromium-browser http://<ESP32_IP>/settings
```

1. You'll see the settings page
2. Enter your WiFi SSID and password under "Connect to Network"
3. Click "Connect"
4. Wait for ESP32 to connect
5. Page should show "Connected to Wi-Fi"

Now restart the kiosk:

```bash
./start-gymbike.sh
```

The main page should now load instead of being redirected.

---

## Solution 2: Disable the Redirect (Developer Option)

If you don't want to save credentials (testing, different network, etc.):

### Edit `src/web_server.cpp` on line ~81:

**Before:**

```cpp
server.on("/", HTTP_GET, [](AsyncWebServerRequest *r) {
    if (!wifiHasSavedCredentials()) {
        r->redirect("/settings");
        return;
    }
```

**After:**

```cpp
server.on("/", HTTP_GET, [](AsyncWebServerRequest *r) {
    // Temporarily disabled for testing
    // if (!wifiHasSavedCredentials()) {
    //     r->redirect("/settings");
    //     return;
    // }
```

Then re-upload firmware:

```bash
pio run --target upload
```

---

## Other Possible Issues

### Issue 2: Browser Cache

**Symptoms:** Page worked before, now blank.

**Solution:**

```bash
killall chromium-browser
rm -rf ~/.cache/chromium
./start-gymbike.sh
```

### Issue 3: JavaScript Disabled

**Check:**

```bash
chromium-browser --version
```

**Solution:** Ensure you're not using `--disable-javascript` flag in start script.

### Issue 4: CORS Headers Missing

**Check:**

```bash
curl -I http://<ESP32_IP>/ | grep Access-Control
```

**Expected:** Should show `Access-Control-Allow-Origin: *`

**Solution:** Firmware wasn't updated. Re-upload with updated `web_server.cpp`.

### Issue 5: Display/Render Issues

**Test with minimal page:**

```bash
chromium-browser file://$HOME/minimal_test.html
```

If this works but the real page doesn't, it's a CSS/JavaScript issue.

### Issue 6: Network/Fetch Failures

**Test with diagnostic page:**

```bash
chromium-browser file://$HOME/diagnostic.html
```

Watch the diagnostic output for any red ❌ errors.

---

## Full Troubleshooting Workflow

Run these in order:

```bash
# 1. Quick test - identifies redirect issue
./quick-test.sh <ESP32_IP>

# 2. Full troubleshooting - comprehensive tests
./troubleshoot-kiosk.sh

# 3. Open diagnostic page - detailed browser tests
chromium-browser file://$HOME/diagnostic.html

# 4. Try minimal test - stripped down version
chromium-browser file://$HOME/minimal_test.html

# 5. Test actual page without kiosk
chromium-browser http://<ESP32_IP>/
# Press F12 to see console errors
```

---

## Understanding the Redirect Logic

The ESP32 code checks if WiFi credentials are saved:

```cpp
if (!wifiHasSavedCredentials()) {
    r->redirect("/settings");
    return;
}
```

**Why it exists:**

- First-time setup UX
- Ensures device is properly configured
- Captive portal behavior

**Why it causes blank page:**

- Kiosk mode might not follow redirects properly
- Settings page might not render correctly in kiosk
- Creates confusion when already connected to WiFi

---

## Verification Steps

After applying a solution:

1. **Test from command line:**

   ```bash
   curl http://<ESP32_IP>/ | grep "Gym Bike"
   ```

   Should output HTML with "Gym Bike" in it.

2. **Test in normal browser:**

   ```bash
   chromium-browser http://<ESP32_IP>/
   ```

   Should show the bike interface, not settings.

3. **Test in kiosk mode:**
   ```bash
   ./start-gymbike.sh
   ```
   Should show the bike interface.

---

## Still Stuck?

### Gather this information:

1. **Output from quick-test.sh:**

   ```bash
   ./quick-test.sh <ESP32_IP> > test-output.txt
   ```

2. **Browser console output:**

   - Open without kiosk: `chromium-browser http://<ESP32_IP>/`
   - Press F12
   - Screenshot or copy errors from Console tab

3. **Diagnostic results:**

   - Run `diagnostic.html`
   - Screenshot or copy all output

4. **ESP32 serial output:**
   - Connect to serial port
   - Copy boot messages and web server logs

### Check these common mistakes:

- ❌ Wrong IP address
- ❌ ESP32 not on same network as Raspberry Pi
- ❌ Firmware not uploaded after making changes
- ❌ Browser cache not cleared
- ❌ WiFi credentials not saved on ESP32
- ❌ JavaScript disabled in browser
- ❌ Display environment not set (`$DISPLAY`)

---

## Summary Decision Tree

```
Blank page?
├── Run quick-test.sh
│   ├── Shows "Wi-Fi Connection" or "Settings"?
│   │   └── → Solution 1: Save WiFi credentials
│   │       OR → Solution 2: Disable redirect
│   │
│   └── Shows "Gym Bike"?
│       ├── Run troubleshoot-kiosk.sh
│       │   ├── CORS missing?
│       │   │   └── → Re-upload firmware
│       │   │
│       │   ├── API not responding?
│       │   │   └── → Check ESP32 serial output
│       │   │
│       │   └── All tests pass?
│       │       ├── Open diagnostic.html
│       │       │   └── Check for JavaScript errors
│       │       │
│       │       └── Clear cache and retry
│       │           └── rm -rf ~/.cache/chromium
```

---

## Prevention for Next Time

1. **Always save WiFi credentials** on ESP32 before using kiosk mode
2. **Test in normal browser first** before going to kiosk mode
3. **Keep a backup** of working firmware
4. **Document your ESP32's IP** for quick troubleshooting

---

## Quick Reference Commands

```bash
# Stop kiosk
./stop-gymbike.sh

# Clear cache
rm -rf ~/.cache/chromium

# Test without kiosk
chromium-browser http://<ESP32_IP>/

# Start kiosk
./start-gymbike.sh

# Full diagnostic
./troubleshoot-kiosk.sh

# Quick check
./quick-test.sh <ESP32_IP>
```
