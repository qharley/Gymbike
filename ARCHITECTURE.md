# GymBike Raspberry Pi Kiosk Architecture

## System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                         Your Network                            │
│                                                                 │
│  ┌──────────────────────┐              ┌────────────────────┐   │
│  │                      │              │                    │   │
│  │      ESP32           │◄────WiFi────►│   Raspberry Pi     │   │
│  │   (GymBike ECU)      │              │   (Display Only)   │   │
│  │                      │              │                    │   │
│  │  ┌────────────────┐  │              │  ┌──────────────┐  │   │
│  │  │ AsyncWebServer │  │              │  │   Chromium   │  │   │
│  │  │   Port 80      │  │              │  │ Kiosk Mode   │  │   │
│  │  │                │  │   HTTP       │  │              │  │   │
│  │  │ GET /          │◄─┼──Requests────┤  │ JavaScript   │  │   │
│  │  │ GET /api/status│  │              │  │ Auto-refresh │  │   │
│  │  │                │  │              │  │              │  │   │
│  │  │ JSON Responses │──┼─────────────►│  │ Display UI   │  │   │
│  │  └────────────────┘  │              │  └──────┬───────┘  │   │
│  │                      │              │         │          │   │
│  │  ┌────────────────┐  │              │         ▼          │   │
│  │  │   Sensors      │  │              │  ┌──────────────┐  │   │
│  │  │  • Cadence     │  │              │  │  480x320     │  │   │
│  │  │  • Buttons     │  │              │  │  Display     │  │   │
│  │  │  • Encoder     │  │              │  │              │  │   │
│  │  └────────────────┘  │              │  │  Shows:      │  │   │
│  │                      │              │  │  • Cadence   │  │   │
│  │  ┌────────────────┐  │              │  │  • Resistance│  │   │
│  │  │   Actuators    │  │              │  │  • Workout   │  │   │
│  │  │  • Servo       │  │              │  │  • Time      │  │   │
│  │  └────────────────┘  │              │  └──────────────┘  │   │
│  └──────────────────────┘              └────────────────────┘   │
│          │                                                      │
│          │ (Future)                                             │
│          ▼                                                      │
│  ┌──────────────────────┐                                       │
│  │  Native ESP32 Display│                                       │
│  │  (When hardware ready)│                                      │
│  └──────────────────────┘                                       │
└─────────────────────────────────────────────────────────────────┘
```

## Data Flow

```
ESP32 Sensors → ESP32 Processing → Web Server
                                       ↓
                                   JSON API
                                       ↓
                        Chromium (JavaScript fetch)
                                       ↓
                                   Parse JSON
                                       ↓
                               Update HTML/CSS
                                       ↓
                              480x320 Display
```

## Request/Response Cycle

```
Every 1 second:
┌──────────────────┐
│ Raspberry Pi     │
│ (JavaScript)     │
└────────┬─────────┘
         │
         │ HTTP GET /api/status
         ▼
┌──────────────────┐
│ ESP32            │
│ web_server.cpp   │
└────────┬─────────┘
         │
         │ Read sensors
         │ Format JSON
         │
         │ {
         │   "cadence": 85,
         │   "resistance": 45,
         │   "mode": 1,
         │   "workoutState": 1,
         │   "workoutTime": 320
         │ }
         ▼
┌──────────────────┐
│ Raspberry Pi     │
│ Update UI        │
└──────────────────┘
```

## Component Responsibilities

### ESP32 (Brain)

**Does:**

- Reads all sensors (cadence, buttons, encoder)
- Controls servo motor
- Runs workout timer
- Handles PID control for cadence mode
- Serves web interface
- Processes API requests

**Doesn't:**

- Render graphics (handled by web browser)
- Store display state (stateless API)

### Raspberry Pi (Display)

**Does:**

- Runs Chromium browser
- Renders HTML/CSS
- Executes JavaScript
- Displays UI on screen
- Polls ESP32 every second
- Shows visual feedback

**Doesn't:**

- Control any hardware
- Store any data
- Make decisions
- Process sensor data

This is a **thin client** architecture - all logic is on ESP32.

## Network Requirements

```
┌─────────────────────────────────────────────┐
│ Must be on same network/subnet              │
│                                             │
│ ESP32:        192.168.1.100                 │
│ Raspberry Pi: 192.168.1.50                  │
│                                             │
│ Communication: HTTP (port 80)               │
│ Protocol: TCP/IP                            │
│ Format: JSON                                │
│                                             │
│ Firewall: Must allow HTTP between devices   │
│ Bandwidth: Minimal (~1KB/sec)               │
└─────────────────────────────────────────────┘
```

## Boot Sequence

### ESP32 Boot

```
1. Power on
2. Initialize WiFi
3. Connect to network
4. Get IP via DHCP
5. Start web server (port 80)
6. Initialize sensors
7. Start control loop
8. Ready to serve requests
```

### Raspberry Pi Boot

```
1. Power on
2. Boot Linux
3. Start X server
4. Run autostart scripts
5. Disable screen blanking
6. Launch Chromium kiosk
7. Load http://192.168.1.100/
8. Start JavaScript polling
9. Display updates every 1s
```

## File Locations

### ESP32

```
/src/web_server.cpp     ← Modified for CORS & CSS
/src/web_server.h
/src/main.cpp
/src/control.cpp        ← Workout state & modes
/src/cadence.cpp        ← RPM sensor
```

### Raspberry Pi

```
/home/pi/
├── setup-kiosk.sh           ← Run this first
├── start-gymbike.sh         ← Manual start
├── stop-gymbike.sh          ← Manual stop
├── test_esp32.py            ← Connection test
├── test_ui.html             ← Browser test
└── .config/
    └── autostart/
        └── gymbike-kiosk.desktop  ← Auto-start config
```

## Why This Architecture?

### Advantages

✅ **Simple** - Raspberry Pi is just a display
✅ **Flexible** - Can view from any device (phone, laptop, etc.)
✅ **Reliable** - ESP32 keeps running even if display fails
✅ **Upgradeable** - Easy to swap Raspberry Pi for native display later
✅ **Multi-viewer** - Multiple devices can view simultaneously
✅ **No tight coupling** - Display and controller are independent

### Disadvantages

❌ Requires network (but both devices need WiFi anyway)
❌ Small delay (1 second update rate)
❌ Extra power consumption (Raspberry Pi)

## Future Migration Path

When you add the native ESP32 display:

```
Current:
ESP32 → Network → Raspberry Pi → 480x320 Display

Future Option 1 (Replace):
ESP32 → Native Display Library → ESP32 Display
(Remove Raspberry Pi, web still available)

Future Option 2 (Both):
ESP32 → Native Display Library → ESP32 Display
     └→ Network → Raspberry Pi → 480x320 Display
(Keep both for redundancy/multi-room viewing)
```

The web interface will continue to work regardless!

## Performance Characteristics

- **Update Rate:** 1 Hz (once per second)
- **Latency:** ~100-200ms network + render time
- **Bandwidth:** ~800 bytes per request
- **CPU Usage (ESP32):** <5%
- **CPU Usage (Pi):** ~10-15%
- **Power Consumption:**
  - ESP32: ~500mA @ 5V = 2.5W
  - Raspberry Pi 3/4: ~700mA @ 5V = 3.5W
  - Total: ~6W

## Security Note

⚠️ **This is an open system:**

- No authentication required
- Anyone on network can access
- CORS allows all origins (`*`)
- Suitable for home networks only

For public/shared networks, consider:

- Add basic authentication
- Restrict CORS origins
- Use HTTPS (requires certificate)
- Enable firewall on Raspberry Pi
