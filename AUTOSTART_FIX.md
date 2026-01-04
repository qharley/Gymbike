# 🔄 Autostart Blank Page Fix

## Problem

- ✅ Manual start (`./start-gymbike.sh`) works fine
- ❌ Autostart on boot shows blank screen

## Root Cause

**Timing issue:** The autostart runs before the network/ESP32 is ready.

When the Pi boots:

1. Autostart triggers immediately
2. Chromium launches and tries to load `http://ESP32_IP/`
3. Network isn't fully up yet
4. ESP32 isn't reachable
5. Chromium shows blank page and doesn't retry

---

## ✅ Quick Fix

### On Raspberry Pi:

```bash
# 1. Get the new startup script
# (Already done if you ran transfer-to-pi.bat)
chmod +x gymbike-kiosk-startup.sh

# 2. Re-run setup with the new startup script
./setup-kiosk.sh
# Enter your ESP32 IP when prompted

# 3. Reboot to test
sudo reboot
```

The new startup script waits for:

- X server to be ready
- Network interface to be up
- ESP32 to respond to ping
- Web server to respond
- Maximum wait: 120 seconds

---

## 🔍 What Changed

### Old Autostart (Immediate Launch)

```
Boot → Autostart → Chromium launches → Network not ready → Blank page
```

### New Autostart (Smart Waiting)

```
Boot → Autostart → Wait for network → Wait for ESP32 → Launch Chromium → Success!
```

### Startup Log

Check the log to see what happened:

```bash
cat ~/gymbike-kiosk.log
```

You'll see:

- When each component became ready
- How long it waited
- Any errors encountered

---

## 🛠️ Manual Fix (If Re-running Setup Doesn't Work)

### Option 1: Update Autostart File Directly

Edit the autostart file:

```bash
nano ~/.config/autostart/gymbike-kiosk.desktop
```

Change the `Exec=` line to:

```ini
Exec=/home/pi/gymbike-kiosk-startup-configured.sh
```

Save and exit (Ctrl+X, Y, Enter).

### Option 2: Add Simple Delay

If you don't have the startup script, add a delay:

```bash
nano ~/.config/autostart/gymbike-kiosk.desktop
```

Change `Exec=` to:

```ini
Exec=sh -c 'sleep 45 && chromium-browser --kiosk http://YOUR_ESP32_IP/'
```

This waits 45 seconds before launching (adjust as needed).

---

## 🔬 Debugging Autostart Issues

### Check if autostart file exists:

```bash
cat ~/.config/autostart/gymbike-kiosk.desktop
```

### Check if startup script exists:

```bash
ls -l ~/gymbike-kiosk-startup-configured.sh
```

### Check the log after boot:

```bash
cat ~/gymbike-kiosk.log
```

### Test the startup script manually:

```bash
./gymbike-kiosk-startup-configured.sh
```

### Check autostart errors:

```bash
journalctl --user -xe | grep gymbike
```

---

## ⚙️ Adjusting Wait Times

If ESP32 takes longer to boot, edit the startup script:

```bash
nano ~/gymbike-kiosk-startup-configured.sh
```

Find this line:

```bash
MAX_WAIT=120  # Maximum wait time in seconds
```

Change 120 to a higher value (e.g., 180 for 3 minutes).

---

## 🎯 Verification

After rebooting, check these:

1. **Did Chromium start?**

   ```bash
   ps aux | grep chromium
   ```

2. **Is the page loaded?**

   - Look at the display
   - Should show bike interface, not blank

3. **Check the log:**

   ```bash
   tail -n 50 ~/gymbike-kiosk.log
   ```

   Should show:

   ```
   ESP32 is reachable via ping
   ESP32 web server is responding
   Starting Chromium kiosk mode...
   SUCCESS: Chromium is running
   ```

---

## 💡 Alternative: systemd Service

For more reliability, use a systemd service instead of autostart:

```bash
sudo nano /etc/systemd/system/gymbike-kiosk.service
```

Add:

```ini
[Unit]
Description=GymBike Kiosk Display
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=pi
Environment=DISPLAY=:0
ExecStart=/home/pi/gymbike-kiosk-startup-configured.sh
Restart=on-failure
RestartSec=10

[Install]
WantedBy=graphical.target
```

Enable and start:

```bash
sudo systemctl enable gymbike-kiosk.service
sudo systemctl start gymbike-kiosk.service
```

Check status:

```bash
sudo systemctl status gymbike-kiosk.service
```

---

## 📊 Comparison

| Method                   | Pros                        | Cons                               |
| ------------------------ | --------------------------- | ---------------------------------- |
| **Direct Chromium**      | Simple                      | No waiting, fails on boot          |
| **With delay (sleep)**   | Easy to implement           | Fixed delay, may be too short/long |
| **Smart startup script** | Waits for actual readiness  | Slightly more complex              |
| **systemd service**      | Most reliable, auto-restart | Requires root, more setup          |

**Recommended:** Smart startup script (what we just implemented)

---

## 🆘 Still Not Working?

1. **Increase wait time** in startup script to 180s
2. **Check log file** for specific errors
3. **Test network after boot:**
   ```bash
   # Wait 30 seconds after boot, then:
   ping YOUR_ESP32_IP
   curl http://YOUR_ESP32_IP/
   ```
4. **Verify autostart runs:**
   ```bash
   ls ~/.config/autostart/
   ```
5. **Try systemd service** instead (more reliable)

---

## Summary

**Problem:** Network/ESP32 not ready when autostart runs
**Solution:** Use `gymbike-kiosk-startup.sh` which waits intelligently
**How to apply:** Re-run `./setup-kiosk.sh` with the new startup script
**Verification:** Check `~/gymbike-kiosk.log` after boot
