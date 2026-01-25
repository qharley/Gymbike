# Display Margin Feature for 480x320 Kiosk

## Overview

Added a user-configurable margin setting to prevent text and graphics from falling off the edges of the 480x320 web kiosk display. This feature allows users to adjust the screen edge margin from 0-50 pixels to accommodate different display bezels and viewing areas.

## How It Works

### 1. Storage System

- **Variable**: `displayMargin` (integer, default: 5px)
- **Storage**: Saved to ESP32 NVS flash memory in the "display" namespace
- **Persistence**: Settings are retained across reboots
- **Functions**:
  - `loadDisplayConfig()` - Loads margin setting on startup
  - `saveDisplayConfig()` - Saves margin setting when changed

### 2. CSS Implementation

The margin is applied using CSS custom properties (variables):

```css
:root {
  --display-margin: 5px;
}
body {
  padding: var(--display-margin);
}
```

This creates a uniform padding around the entire viewport, ensuring all content stays within safe viewing bounds.

### 3. User Interface

#### Settings Location

Navigate to: **Settings (⚙️) → Display Settings (480x320 Kiosk)**

#### Controls

- **Input Range**: 0-50 pixels
- **Default Value**: 5px
- **Recommended**: 5-10px for most displays
- **Current Status**: Shows active margin value in real-time

#### Form Details

```html
<form method="POST" action="/settings/display">
  <input name="margin" type="number" min="0" max="50" step="1" value="5" />
  <button>Save Display Settings</button>
</form>
```

### 4. API Endpoint

**POST** `/settings/display`

- **Parameter**: `margin` (integer 0-50)
- **Validation**: Constrained to 0-50 range via `constrain()`
- **Action**: Saves to NVS and redirects to settings page

## Usage Instructions

### For Users

1. Navigate to the web interface (connect to ESP32 WiFi or use station IP)
2. Click the Settings icon (⚙️) in the top-right corner
3. Scroll to "Display Settings (480x320 Kiosk)"
4. Adjust the "Screen Edge Margin" value:
   - **0px**: No margin (use full screen)
   - **5px**: Light margin (recommended minimum)
   - **10px**: Medium margin (good for displays with bezels)
   - **20px+**: Large margin (for displays with thick bezels or overscan)
5. Click "Save Display Settings"
6. Return to main display to see changes applied

### Troubleshooting Common Issues

#### Content Still Cuts Off

- **Solution**: Increase margin to 10-15px
- **Check**: Some displays have irregular bezels; try asymmetric solutions if needed

#### Too Much Empty Space

- **Solution**: Decrease margin to 3-5px
- **Note**: Minimum safe value depends on your specific display

#### Changes Not Appearing

- **Solution**: Hard refresh the page (Ctrl+F5 or clear browser cache)
- **Check**: Ensure settings were saved (redirects to settings page)

## Technical Details

### Files Modified

1. **[storage.h](src/storage.h)** - Added display config declarations
2. **[storage.cpp](src/storage.cpp)** - Implemented load/save functions
3. **[web_server.cpp](src/web_server.cpp)** - Added UI and CSS implementation
4. **[main.cpp](src/main.cpp)** - Added `loadDisplayConfig()` to startup

### Code Changes Summary

#### Storage Layer

```cpp
// storage.h
extern int displayMargin;
void loadDisplayConfig();
void saveDisplayConfig();

// storage.cpp
int displayMargin = 5; // Default 5px

void loadDisplayConfig() {
    prefs.begin("display", true);
    displayMargin = prefs.getInt("margin", 5);
    prefs.end();
}

void saveDisplayConfig() {
    prefs.begin("display", false);
    prefs.putInt("margin", displayMargin);
    prefs.end();
}
```

#### Web Server Layer

```cpp
// Inject margin into CSS
String marginPx = String(displayMargin) + "px";
":root{--display-margin:" + marginPx + ";}"
"body{...;padding:var(--display-margin);}"
```

#### Settings API

```cpp
server.on("/settings/display", HTTP_POST, [](AsyncWebServerRequest *r) {
    if (r->hasParam("margin", true)) {
        int margin = r->getParam("margin", true)->value().toInt();
        displayMargin = constrain(margin, 0, 50);
        saveDisplayConfig();
    }
    r->redirect("/settings");
});
```

### Memory Usage

- **NVS Storage**: ~4 bytes (one integer)
- **RAM**: ~4 bytes (global variable)
- **Impact**: Minimal - safe for ESP32 memory constraints

## Future Enhancements (Optional)

### Potential Improvements

1. **Per-Edge Margins**: Different values for top/bottom/left/right
2. **Preset Profiles**: Quick presets for common display types
3. **Auto-Detection**: Attempt to detect safe area via JavaScript APIs
4. **Visual Preview**: Real-time preview while adjusting margin
5. **Responsive Breakpoints**: Different margins for different screen sizes

### Implementation Example (Per-Edge)

```cpp
struct DisplayMargins {
    int top;
    int right;
    int bottom;
    int left;
};
```

## Testing Checklist

- [x] Margin value persists across ESP32 reboots
- [x] Settings UI displays current margin value
- [x] CSS correctly applies margin to body element
- [x] Form validation prevents values outside 0-50 range
- [x] Page content stays within display boundaries
- [x] No errors in compilation
- [x] Settings save/load functions work correctly

## Browser Compatibility

- **Modern Browsers**: Full CSS custom property support
- **Chrome/Edge**: ✅ Supported
- **Firefox**: ✅ Supported
- **Safari**: ✅ Supported
- **Fallback**: Not needed (all target browsers support CSS variables)

## Performance Impact

- **Load Time**: +~10 bytes to HTML output
- **Rendering**: No impact (native CSS padding)
- **API Calls**: One additional POST endpoint
- **Overall**: Negligible performance impact

## Conclusion

This feature provides a simple, effective solution for preventing content overflow on kiosk displays. The user-configurable approach ensures compatibility with various display types and bezel configurations without requiring code changes or firmware updates.
