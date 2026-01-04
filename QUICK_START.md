# Quick Start Guide - Raspberry Pi Kiosk for GymBike

## 🚀 Get Up and Running in 5 Minutes

### Prerequisites

- Raspberry Pi with 480x320 display
- ESP32 with GymBike firmware running
- Both devices on the same network
- Know your ESP32's IP address

---

## Step 1: Update ESP32 Firmware ⚙️

The web server code has been updated with CORS headers and optimized CSS for your 480x320 display.

### Using PlatformIO (Recommended)

```bash
cd c:\Users\quent\OneDrive\Documents\Gymbike
pio run --target upload
```

### Using Arduino IDE

1. Open the project
2. Click "Upload" button
3. Wait for upload to complete

**What changed:** [web_server.cpp](src/web_server.cpp) now includes CORS headers and responsive CSS for small screens.

---

## Step 2: Test Connection from Raspberry Pi 🔍

### Option A: Quick Test with Python

```bash
# On Raspberry Pi
python3 test_esp32.py 192.168.1.100
```

You should see all ✓ checkmarks. If not, see troubleshooting below.

### Option B: Browser Test

```bash
chromium-browser file:///home/pi/test_ui.html
```

Enter your ESP32 IP and click "Connect". Watch the log for errors.

---

## Step 3: Setup Kiosk Mode 🖥️

### Automated Setup (Easiest)

```bash
# On Raspberry Pi
chmod +x setup-kiosk.sh
./setup-kiosk.sh
```

Follow the prompts. It will:

- Test connectivity
- Disable screen blanking
- Configure auto-start
- Create control scripts

### Manual Setup

```bash
# Create autostart entry
mkdir -p ~/.config/autostart
nano ~/.config/autostart/gymbike.desktop
```

Add:

```ini
[Desktop Entry]
Type=Application
Name=GymBike
Exec=chromium-browser --kiosk http://192.168.1.100/
X-GNOME-Autostart-enabled=true
```

Save and reboot.

---

## Step 4: Control Your Kiosk 🎮

After running the setup script, you'll have:

### Start Kiosk

```bash
./start-gymbike.sh
```

### Stop Kiosk

```bash
./stop-gymbike.sh
```

Or press **Alt+F4** or **Ctrl+W** while in kiosk mode.

---

## ⚠️ Troubleshooting

### White Page Still Appears

1. **Check firmware was uploaded:**

   ```bash
   curl -I http://192.168.1.100/ | grep Access-Control
   ```

   Should show: `Access-Control-Allow-Origin: *`

2. **Verify API works:**

   ```bash
   curl http://192.168.1.100/api/status
   ```

   Should return JSON data.

3. **Check browser console:**

   - Press F12
   - Look at Console tab
   - Should see "GymBike UI Ready"

4. **Clear browser cache:**
   ```bash
   rm -rf ~/.cache/chromium
   ```

### Connection Refused

- **Verify IP address:** `ping 192.168.1.100`
- **Check ESP32 is running:** Look at serial output
- **Ensure same network:** Both devices must be on same subnet
- **Check firewall:** Some routers block device-to-device traffic

### Display Issues

- **Fonts too large:** Make sure new firmware is uploaded
- **Screen resolution wrong:** Check `/boot/config.txt`
- **Display not detected:** Follow manufacturer's setup guide

---

## 📁 File Reference

### On Your Computer (Windows)

- `web_server.cpp` - Modified ESP32 code (upload this!)
- `test_ui.html` - Standalone test interface
- `test_esp32.py` - Connection test script
- `setup-kiosk.sh` - Automated setup for Pi
- `transfer-to-pi.bat` - Batch script to copy files to Pi

### On Raspberry Pi (after transfer)

- `~/setup-kiosk.sh` - Run this first
- `~/start-gymbike.sh` - Start kiosk manually
- `~/stop-gymbike.sh` - Stop kiosk
- `~/test_esp32.py` - Test connection
- `~/test_ui.html` - Browser-based test

---

## 🎯 Success Checklist

- [ ] ESP32 firmware uploaded with new changes
- [ ] `test_esp32.py` shows all ✓ checks passing
- [ ] Browser shows GymBike UI (no white page)
- [ ] Values update every second
- [ ] Screen stays on (no blanking)
- [ ] Auto-starts on boot (after reboot)
- [ ] Can exit with Alt+F4

---

## 💡 Tips

### Better Performance

- Use wired Ethernet instead of WiFi on Pi
- Assign static IP to ESP32 in router
- Increase GPU memory: `gpu_mem=128` in `/boot/config.txt`

### Remote Access

You can access the web UI from any device on your network:

- From computer: `http://192.168.1.100/`
- From phone: `http://192.168.1.100/`
- From Pi: `http://192.168.1.100/`

The Raspberry Pi display is just one way to view it!

### Development Mode

Instead of kiosk mode during testing:

```bash
chromium-browser http://192.168.1.100/
```

Press F12 for developer tools and debugging.

---

## 📖 More Information

- **Complete Setup Guide:** [RASPBERRY_PI_KIOSK.md](RASPBERRY_PI_KIOSK.md)
- **Detailed Changes:** [KIOSK_FIX_SUMMARY.md](KIOSK_FIX_SUMMARY.md)
- **CSS Modifications:** [CSS_CHANGES.md](CSS_CHANGES.md)

---

## 🆘 Still Need Help?

If you're still seeing a white page after following all steps:

1. Run `test_esp32.py` and share output
2. Check browser console (F12) for errors
3. Verify ESP32 serial console for web server messages
4. Confirm both devices are on same network
5. Try accessing from another device (phone/laptop)

Most issues are:

- Firmware not uploaded (missing CORS headers)
- Wrong IP address
- Network/firewall blocking traffic
- Browser cache needs clearing

Good luck! 🚴‍♂️
