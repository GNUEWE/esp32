# ESP32-CAM Captive Portal with Live Camera Feed

A minimal ESP32-CAM AI-Thinker project that creates an offline captive portal with a live camera snapshot embedded in the landing page.

## Features

- **Open WiFi Access Point**: No password required, easy connection
- **Captive Portal**: Automatically redirects users to the landing page
- **Live Camera Feed**: Real-time camera snapshot that auto-refreshes every 2 seconds
- **Fully Offline**: All resources served locally, no external dependencies
- **Beautiful UI**: Gradient background with modern, responsive design
- **Well Commented**: Easy to understand and extend

## Hardware Requirements

- ESP32-CAM AI-Thinker module
- USB-to-Serial adapter (for programming)
- Power supply (5V)

## Pin Configuration

This sketch uses the standard AI-Thinker ESP32-CAM pin configuration:

| Function | GPIO Pin |
|----------|----------|
| PWDN     | 32       |
| XCLK     | 0        |
| SIOD     | 26       |
| SIOC     | 27       |
| Y9       | 35       |
| Y8       | 34       |
| Y7       | 39       |
| Y6       | 36       |
| Y5       | 21       |
| Y4       | 19       |
| Y3       | 18       |
| Y2       | 5        |
| VSYNC    | 25       |
| HREF     | 23       |
| PCLK     | 22       |

## Installation

### Arduino IDE Setup

1. **Install ESP32 Board Support**:
   - Open Arduino IDE
   - Go to `File` → `Preferences`
   - Add this URL to "Additional Boards Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to `Tools` → `Board` → `Boards Manager`
   - Search for "esp32" and install "esp32 by Espressif Systems"

2. **Select Board**:
   - Go to `Tools` → `Board` → `ESP32 Arduino`
   - Select "AI Thinker ESP32-CAM"

3. **Configure Upload Settings**:
   - `Tools` → `Upload Speed`: 115200
   - `Tools` → `Flash Frequency`: 80MHz
   - `Tools` → `Flash Mode`: QIO
   - `Tools` → `Partition Scheme`: "Huge APP (3MB No OTA/1MB SPIFFS)"
   - `Tools` → `Core Debug Level`: "None"
   - `Tools` → `Port`: Select your USB-to-Serial adapter port

### Upload Sketch

1. Open `ESP32_CAM_CaptivePortal.ino` in Arduino IDE
2. Connect your ESP32-CAM using a USB-to-Serial adapter
3. Put the board in programming mode:
   - Connect GPIO0 to GND
   - Press the reset button
4. Click the Upload button
5. After upload completes:
   - Disconnect GPIO0 from GND
   - Press the reset button to start the program

## Usage

1. **Power on the ESP32-CAM**
   - The board will start automatically after upload
   - Wait ~5 seconds for initialization

2. **Connect to WiFi**
   - Look for WiFi network: `ESP32-CAM-Portal`
   - Connect (no password required)

3. **View the Portal**
   - On most devices, the captive portal page will open automatically
   - If not, open a browser and navigate to any website (e.g., `http://example.com`)
   - You'll be redirected to the ESP32-CAM portal at `192.168.4.1`

4. **View Live Camera Feed**
   - The landing page displays a live camera snapshot
   - Image refreshes automatically every 2 seconds
   - Click "Refresh Now" button for manual refresh

## Configuration

You can customize the following settings in the sketch:

### Access Point Settings
```cpp
const char* AP_SSID = "ESP32-CAM-Portal";      // Change WiFi name
const char* AP_PASSWORD = "";                   // Add password if needed
const IPAddress AP_IP(192, 168, 4, 1);         // Change IP address
```

### Camera Settings
```cpp
config.frame_size = FRAMESIZE_UXGA;    // Change resolution
config.jpeg_quality = 10;              // Adjust quality (0-63, lower = better)
```

Available frame sizes:
- `FRAMESIZE_QQVGA` (160x120)
- `FRAMESIZE_QVGA` (320x240)
- `FRAMESIZE_VGA` (640x480)
- `FRAMESIZE_SVGA` (800x600)
- `FRAMESIZE_XGA` (1024x768)
- `FRAMESIZE_SXGA` (1280x1024)
- `FRAMESIZE_UXGA` (1600x1200)

### Auto-Refresh Interval
In the HTML, change the interval (in milliseconds):
```javascript
setInterval(refreshCamera, 2000);  // Change 2000 to your preferred interval
```

## Extending the Project

The code is structured with clear sections for easy modification:

### Adding New Pages
```cpp
void handleNewPage() {
  String html = "<!DOCTYPE html>...";
  server.send(200, "text/html", html);
}

// In setup():
server.on("/newpage", handleNewPage);
```

### Adding Camera Controls
You can add buttons to control camera settings like brightness, contrast, etc.:
```cpp
void handleBrightness() {
  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, 1);  // Increase brightness
  server.send(200, "text/plain", "Brightness adjusted");
}
```

### Adding Data Storage
Capture and save images to SD card or SPIFFS for later retrieval.

### Adding Authentication
Change `AP_PASSWORD` to add WiFi password protection.

## Troubleshooting

### Camera Initialization Failed
- Check all camera pin connections
- Ensure proper power supply (5V, sufficient current)
- Try pressing the reset button
- Check serial monitor for error messages

### Can't Upload Sketch
- Ensure GPIO0 is connected to GND during upload
- Check USB-to-Serial adapter connections
- Try a lower upload speed (e.g., 115200 instead of 921600)

### Captive Portal Not Showing
- Some devices/browsers may not auto-trigger captive portal
- Manually navigate to `192.168.4.1` in browser
- Try accessing `http://example.com` which will redirect

### Camera Image Not Loading
- Wait a few seconds after connecting
- Check browser console for errors
- Try manual refresh button
- Check serial monitor for camera errors

### Memory Issues
- Reduce frame size to SVGA or lower
- Increase JPEG quality value (lower quality, less memory)
- Use partition scheme with more APP space

## Serial Monitor Output

Expected output when running:
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

## License

This project is provided as-is for educational and development purposes.

## Credits

Created for ESP32-CAM AI-Thinker hardware using Arduino framework.
