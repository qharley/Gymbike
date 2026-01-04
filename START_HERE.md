# 🚨 BLANK PAGE? START HERE

## Your Situation

**Choose your scenario:**

### Scenario A: Blank after manual start

- ✅ `test_esp32.py` runs successfully
- ✅ Setup script completed
- ❌ Kiosk shows blank page when started manually

**→ Go to "Quick Fix A" below**

### Scenario B: Blank only on boot

- ✅ Manual start (`./start-gymbike.sh`) works fine
- ❌ Autostart on boot shows blank screen

**→ See [AUTOSTART_FIX.md](AUTOSTART_FIX.md)**

---

## 🔥 Quick Fix A: Manual Start Shows Blank

### Most Likely Cause

**The ESP32 is redirecting to `/settings` because WiFi credentials aren't saved in flash.**

### On Raspberry Pi:

```bash
# 1. Run the quick test to confirm
chmod +x quick-test.sh
./quick-test.sh <YOUR_ESP32_IP>
```

**If it says "showing SETTINGS page":**

```bash
# 2. Stop kiosk
./stop-gymbike.sh

# 3. Open browser normally
chromium-browser http://<YOUR_ESP32_IP>/settings

# 4. On the settings page:
#    - Enter your WiFi SSID
#    - Enter your WiFi password
#    - Click "Connect"
#    - Wait for "Connected to Wi-Fi" message

# 5. Restart kiosk
./start-gymbike.sh
```

**Problem should be fixed!** 🎉

---

## Alternative: Use Emergency Display

If you just want something working RIGHT NOW:

```bash
# Use the emergency display (no redirects, no complexity)
chromium-browser --kiosk file://$HOME/emergency.html
```

Enter your ESP32 IP when prompted. This will show cadence and resistance only.

---

## 🔍 Still Blank? Run Full Diagnostic

```bash
# 1. Full troubleshooting
chmod +x troubleshoot-kiosk.sh
./troubleshoot-kiosk.sh

# 2. Open diagnostic page
chromium-browser file://$HOME/diagnostic.html

# 3. Check for errors and follow instructions
```

---

## 📁 Files You Now Have

### Testing Tools:

- `quick-test.sh` - Fast check for redirect issue ⚡
- `troubleshoot-kiosk.sh` - Full diagnostic suite 🔍
- `diagnostic.html` - In-browser testing 🌐
- `minimal_test.html` - Simplified UI test 📊
- `emergency.html` - Bare-bones display 🚨
- `test_esp32.py` - Python connectivity test 🐍

### Setup Scripts:

- `setup-kiosk.sh` - Initial setup
- `start-gymbike.sh` - Start kiosk
- `stop-gymbike.sh` - Stop kiosk

### Documentation:

- `BLANK_PAGE_FIX.md` - Complete troubleshooting guide 📖
- `RASPBERRY_PI_KIOSK.md` - Full setup documentation 📚
- `QUICK_START.md` - Getting started guide 🚀

---

## 🎯 Decision Tree

```
Is the page blank?
│
├─ YES → Run: ./quick-test.sh <ESP32_IP>
│   │
│   ├─ Shows "Settings" page?
│   │   └─ Save WiFi credentials on ESP32
│   │      (see Quick Fix above)
│   │
│   └─ Shows "Gym Bike" page?
│       ├─ Run: ./troubleshoot-kiosk.sh
│       ├─ Open: diagnostic.html
│       └─ Clear cache: rm -rf ~/.cache/chromium
│
└─ NO → Great! Kiosk is working
```

---

## 💡 Common Issues & Fixes

| Issue                        | Solution                                               |
| ---------------------------- | ------------------------------------------------------ |
| **Blank white page**         | WiFi credentials not saved → Save them via `/settings` |
| **Worked before, blank now** | Browser cache → `rm -rf ~/.cache/chromium`             |
| **"Connection refused"**     | Wrong IP or ESP32 off → Check IP and power             |
| **Redirects to settings**    | Credentials not saved → Save via web interface         |
| **JavaScript errors**        | CORS missing → Re-upload firmware                      |
| **Partial content**          | Old cache → Clear and restart                          |

---

## 🆘 Emergency Contacts

### Get immediate display:

```bash
chromium-browser --kiosk file://$HOME/emergency.html
```

### Test without complexity:

```bash
chromium-browser file://$HOME/minimal_test.html
```

### See what's really happening:

```bash
chromium-browser file://$HOME/diagnostic.html
```

---

## ✅ Verification Checklist

After applying a fix:

- [ ] `curl http://<ESP32_IP>/ | grep "Gym Bike"` shows HTML
- [ ] Browser (non-kiosk) shows bike interface
- [ ] `./quick-test.sh` shows no redirect
- [ ] Kiosk mode displays correctly

---

## 📞 Need More Help?

1. Run `./troubleshoot-kiosk.sh` and save output
2. Open `diagnostic.html` and screenshot results
3. Check browser console (F12) for errors
4. Review `BLANK_PAGE_FIX.md` for detailed guide

---

## Remember

The **python test working** means:

- ✅ Network connectivity is fine
- ✅ ESP32 web server is running
- ✅ API endpoints work

The **blank page** usually means:

- ❌ Being redirected to settings
- ❌ JavaScript not executing
- ❌ Browser cache issues
- ❌ CORS blocking requests

**Most common:** WiFi credentials not saved → redirect to settings → blank in kiosk mode.

**Fix:** Save credentials via `/settings` page, then restart kiosk.
