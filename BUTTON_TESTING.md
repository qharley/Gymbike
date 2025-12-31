# Button Testing Guide

## Hardware Setup

### Required Components

- ESP32 (NodeMCU-32S)
- 3x Push buttons (normally open)
- 3x 10kΩ resistors (optional - using internal pull-ups)
- Breadboard and jumper wires

### Wiring Diagram

```
Button Connections:
┌─────────────────┬──────────┬────────────────┐
│ Button          │ GPIO Pin │ Connection     │
├─────────────────┼──────────┼────────────────┤
│ Start/Stop      │ 14       │ One side to    │
│ Rest            │ 12       │ GPIO, other    │
│ Reset           │ 13       │ side to GND    │
└─────────────────┴──────────┴────────────────┘

Each button:
  [Button] ─── GPIO Pin (with internal PULLUP)
      └──────── GND

Note: Buttons are configured with INPUT_PULLUP,
so they read HIGH when not pressed and LOW when pressed.
```

## Testing Methods

### Method 1: Simple Button Test (Standalone)

1. **Backup main.cpp**

   ```bash
   cd src
   copy main.cpp main.cpp.bak
   copy ..\test\button_test.cpp main.cpp
   ```

2. **Build and Upload**

   ```bash
   pio run --target upload
   ```

3. **Open Serial Monitor**

   ```bash
   pio device monitor
   ```

   Or open Serial Monitor in VS Code at 115200 baud

4. **Test Each Button**

   - Press Start/Stop → Should see "START/STOP BUTTON PRESSED"
   - Press Rest → Should see "REST BUTTON PRESSED"
   - Press Reset → Should see "RESET BUTTON PRESSED"
   - Every 10 seconds, see a summary of button presses

5. **Restore main.cpp**
   ```bash
   copy main.cpp.bak main.cpp
   del main.cpp.bak
   ```

### Method 2: Integrated Test (Full System)

The main application now includes comprehensive button logging.

1. **Build and Upload**

   ```bash
   pio run --target upload
   ```

2. **Open Serial Monitor at 115200 baud**

3. **Watch for startup messages**

   ```
   === GymBike Starting ===
   Testing Physical Buttons:
     - Start/Stop: Pin 14
     - Rest: Pin 12
     - Reset: Pin 13
   [OK] Buttons initialized
   [OK] System ready
   ```

4. **Test Button Sequence**

   **Test Start/Stop:**

   - Press Start/Stop when STOPPED → See "[BUTTON] Start/Stop pressed - Starting workout"
   - System changes to RUNNING state
   - Press Start/Stop when RUNNING → See "[BUTTON] Start/Stop pressed - Stopping workout"

   **Test Rest:**

   - Start workout first (Start/Stop)
   - Press Rest when RUNNING → See "[BUTTON] Rest pressed - Entering rest mode"
   - Press Rest when RESTING → See "[BUTTON] Rest pressed - Resuming from rest"
   - Press Rest when STOPPED → See "[BUTTON] Rest pressed - Cannot rest (workout not running)"

   **Test Reset:**

   - Press Reset anytime → See "[BUTTON] Reset pressed - Resetting all workout data"
   - All timers reset, servo goes to minimum, targets reset to defaults

5. **Monitor Status Updates**
   Every 5 seconds, see workout status:
   ```
   [STATUS] Workout: RUNNING | Mode: MANUAL | Target: 80 RPM | Servo: 5
   ```

## Expected Behavior

### Start/Stop Button Behavior

| Current State | Action | New State | Servo Action       |
| ------------- | ------ | --------- | ------------------ |
| STOPPED       | Press  | RUNNING   | Controlled by mode |
| RUNNING       | Press  | STOPPED   | Release to minimum |
| RESTING       | Press  | RUNNING   | Resume control     |

### Rest Button Behavior

| Current State | Action | New State | Servo Action        |
| ------------- | ------ | --------- | ------------------- |
| STOPPED       | Press  | STOPPED   | No change (warning) |
| RUNNING       | Press  | RESTING   | Release to minimum  |
| RESTING       | Press  | RUNNING   | Resume control      |

### Reset Button Behavior

- Always resets to STOPPED state
- Clears all workout timers
- Resets servo to minimum position
- Resets PID controller
- Resets targets to defaults (80 RPM, 150W)

## Troubleshooting

### Button Not Responding

1. **Check wiring** - Verify button connected between GPIO and GND
2. **Check GPIO pin** - Ensure correct pin numbers (14, 12, 13)
3. **Test continuity** - Use multimeter to verify button closes circuit
4. **Check Serial output** - No message means interrupt not firing

### Button Triggers Multiple Times

1. **Physical bounce** - This is normal, debouncing is set to 200ms
2. **Loose connection** - Check wire connections
3. **Bad button** - Try different button

### Servo Not Moving

1. **Check workout state** - Servo only active when RUNNING
2. **Check mode** - In MANUAL mode, use rotary encoder to set position
3. **Check emergency pin** - GPIO 26 must be HIGH (or disconnected with pullup)

### Serial Output Garbled

1. **Wrong baud rate** - Must be 115200
2. **Check USB connection** - Try different cable/port
3. **Reset ESP32** - Press reset button on board

## Advanced Testing

### Stress Test

Test rapid button presses:

```cpp
// Press each button 10 times quickly
// All presses should be registered (check counter)
// System should remain stable
```

### Simultaneous Press Test

Try pressing multiple buttons at once:

- Should handle gracefully (interrupts are independent)
- Each button action executes in sequence

### Long Runtime Test

Leave system running for extended period:

- Press buttons occasionally
- Verify no memory leaks or crashes
- Check status output remains consistent

## Pin Reference

```
ESP32 GPIO Allocations:
┌──────────┬───────────────────────────┐
│ GPIO Pin │ Function                  │
├──────────┼───────────────────────────┤
│ 14       │ Start/Stop Button         │
│ 12       │ Rest Button               │
│ 13       │ Reset Button              │
│ 25       │ Servo Control             │
│ 26       │ Emergency Stop            │
│ 27       │ Encoder Switch            │
│ 32       │ Encoder CLK               │
│ 33       │ Encoder DT                │
│ 34       │ Cadence Sensor (input)    │
└──────────┴───────────────────────────┘
```

## Quick Test Checklist

- [ ] Buttons wired correctly (GPIO to one side, GND to other)
- [ ] Firmware uploaded successfully
- [ ] Serial monitor open at 115200 baud
- [ ] Startup message shows "Buttons initialized"
- [ ] Start/Stop button changes workout state
- [ ] Rest button pauses/resumes workout
- [ ] Reset button clears all workout data
- [ ] Status updates show state changes
- [ ] Servo responds to workout state changes
- [ ] No crashes or unexpected behavior
