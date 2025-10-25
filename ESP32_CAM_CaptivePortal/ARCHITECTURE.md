# ESP32-CAM Captive Portal - Architecture Overview

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         ESP32-CAM Device                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌───────────────┐        ┌──────────────┐                    │
│  │   Camera      │        │   WiFi       │                    │
│  │   Module      │        │   SoftAP     │                    │
│  │   (OV2640)    │        │   Mode       │                    │
│  └───────┬───────┘        └──────┬───────┘                    │
│          │                       │                             │
│          │  JPEG frames          │  192.168.4.1               │
│          │                       │                             │
│  ┌───────▼────────────────────────▼───────┐                   │
│  │         Main Program (Arduino)         │                   │
│  │                                         │                   │
│  │  ┌─────────────┐   ┌────────────────┐  │                  │
│  │  │ DNS Server  │   │  HTTP Server   │  │                  │
│  │  │  Port 53    │   │   Port 80      │  │                  │
│  │  │             │   │                │  │                  │
│  │  │ Redirects   │   │ Routes:        │  │                  │
│  │  │ all domains │   │  /        →    │  │                  │
│  │  │ to          │   │  /camera  →    │  │                  │
│  │  │ 192.168.4.1 │   │  /*       →    │  │                  │
│  │  └─────────────┘   └────────────────┘  │                  │
│  └─────────────────────────────────────────┘                   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
                            │
                            │ WiFi Connection
                            │ SSID: ESP32-CAM-Portal
                            │ (Open - No Password)
                            │
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                       Client Device                             │
│                  (Phone, Tablet, Laptop)                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. Connect to WiFi → "ESP32-CAM-Portal"                       │
│  2. Browser opens any URL → DNS redirects to 192.168.4.1      │
│  3. HTTP GET / → Receives HTML landing page                    │
│  4. Browser loads → JavaScript requests /camera every 2s       │
│  5. Displays live camera feed                                   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Component Breakdown

### 1. Camera Module (OV2640)
- **Purpose**: Captures images for the live feed
- **Configuration**: 
  - Resolution: UXGA (1600x1200) with PSRAM, SVGA (800x600) without
  - Format: JPEG
  - Quality: 10-12 (lower number = higher quality)
- **Pin Assignment**: Standard AI-Thinker ESP32-CAM pinout

### 2. WiFi SoftAP (Software Access Point)
- **Purpose**: Creates a WiFi network for clients to connect
- **Configuration**:
  - SSID: "ESP32-CAM-Portal"
  - Password: None (Open network)
  - IP: 192.168.4.1
  - Subnet: 255.255.255.0

### 3. DNS Server
- **Purpose**: Implements captive portal by redirecting all DNS queries
- **Functionality**:
  - Listens on port 53
  - Wildcard domain matching (`*`)
  - Returns device IP (192.168.4.1) for all queries
- **Result**: Any website client tries to visit redirects to our portal

### 4. HTTP Web Server
- **Purpose**: Serves web pages and camera images
- **Routes**:
  - `GET /` → Landing page with embedded camera feed
  - `GET /camera` → JPEG snapshot from camera
  - `GET /*` → Redirects to landing page (404 handler)

### 5. Landing Page
- **HTML/CSS/JavaScript**: All inline (no external resources)
- **Features**:
  - Responsive design with gradient background
  - Auto-refreshing camera image (2 second intervals)
  - Manual refresh button
  - Status information display

## Data Flow

### Initial Connection Flow
```
Client                          ESP32-CAM
  │                                │
  │  1. WiFi Scan                 │
  │─────────────────────────────>│
  │  2. SSID: ESP32-CAM-Portal   │
  │<─────────────────────────────│
  │                                │
  │  3. Connect (no auth)         │
  │─────────────────────────────>│
  │  4. DHCP IP Assignment        │
  │<─────────────────────────────│
  │  (192.168.4.2)                │
```

### Captive Portal Redirection Flow
```
Client Browser                  ESP32-CAM
  │                                │
  │  1. DNS Query: example.com    │
  │─────────────────────────────>│
  │  2. DNS Response: 192.168.4.1 │
  │<─────────────────────────────│
  │                                │
  │  3. HTTP GET http://example   │
  │─────────────────────────────>│
  │  4. 302 Redirect to /         │
  │<─────────────────────────────│
  │                                │
  │  5. HTTP GET /                │
  │─────────────────────────────>│
  │  6. HTML Landing Page         │
  │<─────────────────────────────│
```

### Camera Feed Flow
```
Browser JavaScript              ESP32-CAM
  │                                │
  │  1. HTTP GET /camera?t=xxx    │
  │─────────────────────────────>│
  │                                │
  │                           2. Capture Frame
  │                           esp_camera_fb_get()
  │                                │
  │  3. JPEG Image (binary)       │
  │<─────────────────────────────│
  │                                │
  │  4. Display in <img> tag      │
  │                                │
  │  ... Wait 2 seconds ...       │
  │                                │
  │  5. HTTP GET /camera?t=yyy    │
  │─────────────────────────────>│
  │                                │
  │  (Cycle repeats)              │
```

## Code Structure

### File: ESP32_CAM_CaptivePortal.ino

```
┌─────────────────────────────────────────┐
│ 1. Header & Documentation               │
│    - Project description                │
│    - Feature list                       │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│ 2. Library Includes                     │
│    - WiFi.h                             │
│    - DNSServer.h                        │
│    - WebServer.h                        │
│    - esp_camera.h                       │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│ 3. Configuration Section                │
│    - AP credentials & network config    │
│    - DNS server config                  │
│    - Web server instantiation           │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│ 4. Pin Definitions                      │
│    - AI-Thinker ESP32-CAM pinout        │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│ 5. initCamera() Function                │
│    - Camera configuration               │
│    - Sensor initialization              │
│    - Image quality settings             │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│ 6. HTTP Request Handlers                │
│    - handleRoot()       (Landing page)  │
│    - handleCamera()     (JPEG snapshot) │
│    - handleNotFound()   (Redirect)      │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│ 7. setup() Function                     │
│    - Serial initialization              │
│    - Camera initialization              │
│    - WiFi AP start                      │
│    - DNS server start                   │
│    - HTTP server configuration          │
│    - HTTP server start                  │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│ 8. loop() Function                      │
│    - Process DNS requests               │
│    - Handle HTTP requests               │
│    - Watchdog delay                     │
└─────────────────────────────────────────┘
```

## Memory Considerations

### PSRAM Detection
- **With PSRAM**: Higher resolution (UXGA), 2 frame buffers
- **Without PSRAM**: Lower resolution (SVGA), 1 frame buffer

### Frame Buffer Management
```c
// Capture
camera_fb_t * fb = esp_camera_fb_get();

// Use frame buffer
server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);

// IMPORTANT: Return buffer to pool
esp_camera_fb_return(fb);
```

## Extending the System

### Add New Pages
```c
void handleNewPage() {
  String html = "<!DOCTYPE html>...";
  server.send(200, "text/html", html);
}

// In setup():
server.on("/newpage", handleNewPage);
```

### Add Camera Controls
```c
void handleBrightness() {
  sensor_t * s = esp_camera_sensor_get();
  int level = server.arg("level").toInt();
  s->set_brightness(s, level);
  server.send(200, "text/plain", "OK");
}

// In setup():
server.on("/brightness", handleBrightness);
```

### Add Data Logging
```c
#include "SD_MMC.h"

void saveImage() {
  camera_fb_t * fb = esp_camera_fb_get();
  String path = "/image_" + String(millis()) + ".jpg";
  File file = SD_MMC.open(path, FILE_WRITE);
  file.write(fb->buf, fb->len);
  file.close();
  esp_camera_fb_return(fb);
}
```

## Performance Characteristics

- **Startup Time**: ~3-5 seconds
- **Camera Capture Time**: ~100-300ms per frame
- **Image Refresh Rate**: 2 seconds (configurable)
- **Concurrent Clients**: 4-5 (ESP32 limitation)
- **Memory Usage**: 
  - Sketch: ~500KB
  - Runtime: ~100KB + frame buffers
  - Frame buffer: ~100KB (SVGA) to ~500KB (UXGA)

## Security Considerations

⚠️ **Important**: This is designed for demonstration/development purposes.

- **Open WiFi**: No authentication required
- **No HTTPS**: All traffic is unencrypted
- **No Authentication**: Web server has no login
- **Limited Range**: WiFi range limits access

### Production Recommendations
1. Add WiFi password: `const char* AP_PASSWORD = "your_password";`
2. Implement HTTPS with certificates
3. Add web authentication (HTTP Basic Auth or form-based)
4. Limit client connections
5. Add rate limiting for camera endpoint

## Troubleshooting Decision Tree

```
┌─────────────────────────┐
│   Can't find WiFi?      │
└───────────┬─────────────┘
            │
    No  ────┴──── Yes
    │              │
    ▼              ▼
Power?      ┌──────────────┐
Check       │ Connected?   │
5V supply   └──────┬───────┘
            │      │
        No ─┴─ Yes │
        │          ▼
        │   ┌─────────────────┐
        │   │  Portal shows?  │
        │   └────────┬────────┘
        │      No ───┴─── Yes
        ▼            │         │
    Wait 30s    Try      ▼
    Press         http://   Working!
    Reset       192.168.4.1
```

## Summary

This ESP32-CAM captive portal project provides a complete, standalone solution for creating an offline WiFi portal with live camera streaming. The architecture is modular and well-documented, making it easy to extend with additional features like data logging, camera controls, or custom UI elements.
