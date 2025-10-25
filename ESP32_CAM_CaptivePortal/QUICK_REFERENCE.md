# Quick Reference Guide

## Common Modifications

### 1. Change WiFi Network Name
```cpp
const char* AP_SSID = "ESP32-CAM-Portal";  // Change this
```

### 2. Add WiFi Password
```cpp
const char* AP_PASSWORD = "mypassword123";  // Add your password here
```

### 3. Change IP Address
```cpp
const IPAddress AP_IP(192, 168, 5, 1);      // Change to your preferred IP
const IPAddress AP_GATEWAY(192, 168, 5, 1); // Match the IP
```

### 4. Adjust Camera Resolution

Available sizes (smallest to largest):
```cpp
config.frame_size = FRAMESIZE_QQVGA;  // 160x120
config.frame_size = FRAMESIZE_QVGA;   // 320x240
config.frame_size = FRAMESIZE_VGA;    // 640x480
config.frame_size = FRAMESIZE_SVGA;   // 800x600    ← Default without PSRAM
config.frame_size = FRAMESIZE_XGA;    // 1024x768
config.frame_size = FRAMESIZE_SXGA;   // 1280x1024
config.frame_size = FRAMESIZE_UXGA;   // 1600x1200  ← Default with PSRAM
```

### 5. Adjust Image Quality
```cpp
config.jpeg_quality = 10;  // Range: 0-63 (lower = better quality, more memory)
```

### 6. Change Auto-Refresh Interval

In the HTML section:
```javascript
setInterval(refreshCamera, 2000);  // Change 2000 to milliseconds you want
```

Examples:
- 1 second: `1000`
- 3 seconds: `3000`
- 5 seconds: `5000`

### 7. Customize Page Title and Heading

Find in `handleRoot()` function:
```html
<title>ESP32-CAM Portal</title>              // Browser tab title
...
<h1>&#x1F4F7; ESP32-CAM Portal</h1>         // Page heading
```

### 8. Change Color Scheme

Find in the `<style>` section of `handleRoot()`:
```css
background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
```

Popular gradients:
- Ocean: `#2E3192 0%, #1BFFFF 100%`
- Sunset: `#FF512F 0%, #DD2476 100%`
- Forest: `#134E5E 0%, #71B280 100%`
- Fire: `#F00000 0%, #DC281E 100%`

### 9. Flip Camera Image

In `initCamera()` function:
```cpp
s->set_hmirror(s, 1);  // Horizontal flip: 0 = off, 1 = on
s->set_vflip(s, 1);    // Vertical flip: 0 = off, 1 = on
```

### 10. Adjust Camera Settings

In `initCamera()` function:
```cpp
s->set_brightness(s, 0);     // -2 to 2 (increase for brighter)
s->set_contrast(s, 0);       // -2 to 2 (increase for more contrast)
s->set_saturation(s, 0);     // -2 to 2 (increase for more color)
```

## Adding New Features

### Add a Download Button

Add to HTML in `handleRoot()`:
```html
<button class='refresh-btn' onclick='downloadImage()'>
  &#x1F4BE; Download Snapshot
</button>
```

Add JavaScript:
```javascript
function downloadImage() {
  var link = document.createElement('a');
  link.href = '/camera?t=' + new Date().getTime();
  link.download = 'esp32cam_' + Date.now() + '.jpg';
  link.click();
}
```

### Add Status LED Control

Add handler function before `setup()`:
```cpp
#define LED_PIN 33  // Built-in LED on ESP32-CAM

void handleLED() {
  String state = server.arg("state");
  digitalWrite(LED_PIN, state == "on" ? HIGH : LOW);
  server.send(200, "text/plain", "LED " + state);
}
```

In `setup()`:
```cpp
pinMode(LED_PIN, OUTPUT);
server.on("/led", handleLED);
```

Add button to HTML:
```html
<button class='refresh-btn' onclick='toggleLED()'>
  Toggle LED
</button>

<script>
var ledState = false;
function toggleLED() {
  ledState = !ledState;
  fetch('/led?state=' + (ledState ? 'on' : 'off'));
}
</script>
```

### Add Text Overlay on Image

This requires more advanced processing, but here's a simple approach using HTML:
```html
<div style='position: relative;'>
  <img id='camImage' class='camera-image' src='/camera'>
  <div style='position: absolute; top: 10px; left: 10px; color: white; 
              background: rgba(0,0,0,0.5); padding: 5px; border-radius: 5px;'>
    Timestamp: <span id='timestamp'></span>
  </div>
</div>

<script>
function updateTimestamp() {
  document.getElementById('timestamp').textContent = new Date().toLocaleString();
}
setInterval(updateTimestamp, 1000);
</script>
```

### Add Multiple Camera Views

If you want side-by-side or sequential views:
```html
<div style='display: flex; flex-wrap: wrap; gap: 10px;'>
  <div class='camera-container'>
    <h3>Current View</h3>
    <img class='camera-image' src='/camera?view=1'>
  </div>
  <div class='camera-container'>
    <h3>5 Seconds Ago</h3>
    <img class='camera-image' src='/camera?view=2'>
  </div>
</div>
```

### Add Connection Counter

Add global variable:
```cpp
int connectionCount = 0;
```

Modify `handleRoot()`:
```cpp
void handleRoot() {
  connectionCount++;
  String html = "<!DOCTYPE html>...";
  // Add to HTML:
  // "<p><strong>Total Connections:</strong> " + String(connectionCount) + "</p>"
  server.send(200, "text/html", html);
}
```

### Save Images to SD Card

Add at top:
```cpp
#include "SD_MMC.h"
bool sdCardAvailable = false;
```

In `setup()`:
```cpp
// Initialize SD card
if(!SD_MMC.begin()) {
  Serial.println("SD Card Mount Failed");
} else {
  sdCardAvailable = true;
  Serial.println("SD Card initialized");
}
```

Add handler:
```cpp
void handleSave() {
  if (!sdCardAvailable) {
    server.send(503, "text/plain", "SD card not available");
    return;
  }
  
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    server.send(503, "text/plain", "Camera capture failed");
    return;
  }
  
  String filename = "/img_" + String(millis()) + ".jpg";
  File file = SD_MMC.open(filename, FILE_WRITE);
  if (file) {
    file.write(fb->buf, fb->len);
    file.close();
    server.send(200, "text/plain", "Saved: " + filename);
  } else {
    server.send(500, "text/plain", "Failed to save");
  }
  
  esp_camera_fb_return(fb);
}
```

In `setup()`:
```cpp
server.on("/save", handleSave);
```

## Serial Monitor Commands

While monitoring serial output (115200 baud), you'll see:
```
=== ESP32-CAM Captive Portal ===
Initializing camera...
Camera initialized successfully
Starting WiFi Access Point...
Access Point started: ESP32-CAM-Portal
IP Address: 192.168.4.1
DNS server started
HTTP server started

=== Setup Complete ===
Connect to WiFi: ESP32-CAM-Portal
Then open any website to see the portal
```

If you see errors:
- `Camera init failed`: Check camera connections
- `Camera capture failed`: Camera may be in use or disconnected

## Performance Tuning

### For Faster Response
```cpp
config.frame_size = FRAMESIZE_QVGA;  // Smaller image
config.jpeg_quality = 20;            // Lower quality
setInterval(refreshCamera, 5000);    // Less frequent updates
```

### For Better Quality
```cpp
config.frame_size = FRAMESIZE_UXGA;  // Larger image
config.jpeg_quality = 5;             // Higher quality
setInterval(refreshCamera, 3000);    // Slightly less frequent
```

### For More Clients
```cpp
config.fb_count = 2;  // Use 2 frame buffers if you have PSRAM
```

## Testing Checklist

- [ ] Upload sketch successfully
- [ ] See serial output showing "Setup Complete"
- [ ] Find WiFi network with correct SSID
- [ ] Connect to WiFi (no password if configured as open)
- [ ] Captive portal opens automatically (or navigate to 192.168.4.1)
- [ ] See landing page with gradient background
- [ ] Camera image loads
- [ ] Image refreshes automatically every 2 seconds
- [ ] Manual refresh button works
- [ ] Test on multiple devices (phone, laptop)

## Common Issues & Quick Fixes

| Issue | Quick Fix |
|-------|-----------|
| Can't upload sketch | Connect GPIO0 to GND before upload |
| Camera image is upside down | Set `s->set_vflip(s, 1);` |
| Image too dark | Increase `s->set_brightness(s, 1);` |
| Image updates too slow | Reduce refresh interval to 1000ms |
| Out of memory errors | Reduce frame size to SVGA or VGA |
| WiFi not visible | Check power supply (need 5V, >500mA) |
| Portal doesn't open | Manually go to 192.168.4.1 |

## Pin Reference (AI-Thinker ESP32-CAM)

```
     ╔════════════════════════════════╗
     ║                                ║
     ║         [OV2640 Camera]        ║
     ║                                ║
     ╠════════════════════════════════╣
     ║  GPIO 0   ┊  GPIO 32  (PWDN)  ║
     ║  GND      ┊  GND               ║
     ║  5V       ┊  GPIO 33  (LED)    ║
     ║  GPIO 4   ┊  GPIO 1   (TX)     ║
     ║  ...      ┊  GPIO 3   (RX)     ║
     ╚════════════════════════════════╝
         [SD Card Slot on bottom]
```

**Important Pins:**
- GPIO 0: Hold LOW during upload
- GPIO 33: Built-in LED (active HIGH)
- GPIO 4: Flash LED
- TX/RX: Serial communication (115200 baud)

## Resources

- **ESP32-CAM Datasheet**: Search "AI-Thinker ESP32-CAM"
- **OV2640 Camera**: Search "OV2640 datasheet"
- **ESP32 Arduino Core**: https://github.com/espressif/arduino-esp32
- **ESP32 Camera Library**: https://github.com/espressif/esp32-camera

## Need Help?

1. Check serial monitor output at 115200 baud
2. Verify power supply is adequate (5V, >500mA)
3. Confirm camera cable is properly seated
4. Try pressing reset button
5. Re-upload sketch with GPIO0 to GND
