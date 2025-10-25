/*
 * ESP32-CAM AI-Thinker Captive Portal with Live Camera Snapshot
 * 
 * This sketch creates an offline captive portal on the ESP32-CAM board.
 * When users connect to the open WiFi access point, they are redirected
 * to a landing page that displays a live camera snapshot.
 * 
 * Hardware: ESP32-CAM AI-Thinker module
 * 
 * Features:
 * - Open SoftAP (no password required)
 * - DNS server for captive portal redirection
 * - HTTP web server serving all content offline
 * - Live camera snapshot embedded in landing page
 * - Comprehensive comments for easy extension
 */

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include "esp_camera.h"

// ============================================================================
// Configuration Section
// ============================================================================

// Access Point Configuration
const char* AP_SSID = "ESP32-CAM-Portal";      // WiFi network name
const char* AP_PASSWORD = "";                   // Empty = open network
const IPAddress AP_IP(192, 168, 4, 1);         // Access point IP address
const IPAddress AP_GATEWAY(192, 168, 4, 1);    // Gateway IP
const IPAddress AP_SUBNET(255, 255, 255, 0);   // Subnet mask

// DNS Server Configuration (for captive portal)
const byte DNS_PORT = 53;                       // DNS port
DNSServer dnsServer;                            // DNS server instance

// Web Server Configuration
WebServer server(80);                           // HTTP server on port 80

// ============================================================================
// AI-Thinker ESP32-CAM Pin Configuration
// ============================================================================

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ============================================================================
// Camera Initialization Function
// ============================================================================

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Frame size and quality configuration
  // Use lower resolution for faster refresh and lower memory usage
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;    // 1600x1200
    config.jpeg_quality = 10;              // 0-63, lower means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;    // 800x600
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;
  }
  
  // Additional sensor configuration for better image quality
  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 0);     // -2 to 2
    s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, etc.)
    s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled
    s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
    s->set_aec2(s, 0);           // 0 = disable , 1 = enable
    s->set_ae_level(s, 0);       // -2 to 2
    s->set_aec_value(s, 300);    // 0 to 1200
    s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);       // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
    s->set_bpc(s, 0);            // 0 = disable , 1 = enable
    s->set_wpc(s, 1);            // 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
    s->set_lenc(s, 1);           // 0 = disable , 1 = enable
    s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
    s->set_vflip(s, 0);          // 0 = disable , 1 = enable
    s->set_dcw(s, 1);            // 0 = disable , 1 = enable
    s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
  }
  
  Serial.println("Camera initialized successfully");
  return true;
}

// ============================================================================
// HTTP Request Handlers
// ============================================================================

/**
 * Landing Page Handler
 * Serves the main HTML page with embedded camera snapshot
 * All content is inline (no external resources) for offline operation
 */
void handleRoot() {
  String html = "<!DOCTYPE html>"
    "<html>"
    "<head>"
      "<meta charset='UTF-8'>"
      "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
      "<title>ESP32-CAM Portal</title>"
      "<style>"
        "body {"
          "font-family: Arial, sans-serif;"
          "margin: 0;"
          "padding: 20px;"
          "background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
          "color: white;"
          "text-align: center;"
        "}"
        ".container {"
          "max-width: 800px;"
          "margin: 0 auto;"
          "background: rgba(255, 255, 255, 0.1);"
          "border-radius: 15px;"
          "padding: 30px;"
          "box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37);"
          "backdrop-filter: blur(4px);"
          "border: 1px solid rgba(255, 255, 255, 0.18);"
        "}"
        "h1 {"
          "margin: 0 0 10px 0;"
          "font-size: 2.5em;"
        "}"
        ".subtitle {"
          "font-size: 1.2em;"
          "opacity: 0.9;"
          "margin-bottom: 30px;"
        "}"
        ".camera-container {"
          "background: white;"
          "border-radius: 10px;"
          "padding: 10px;"
          "margin: 20px 0;"
          "box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);"
        "}"
        ".camera-image {"
          "width: 100%;"
          "height: auto;"
          "border-radius: 5px;"
          "display: block;"
        "}"
        ".info {"
          "margin-top: 30px;"
          "font-size: 0.9em;"
          "opacity: 0.8;"
        "}"
        ".refresh-btn {"
          "background: rgba(255, 255, 255, 0.3);"
          "border: 2px solid white;"
          "color: white;"
          "padding: 12px 30px;"
          "font-size: 1em;"
          "border-radius: 25px;"
          "cursor: pointer;"
          "margin-top: 20px;"
          "transition: all 0.3s ease;"
        "}"
        ".refresh-btn:hover {"
          "background: rgba(255, 255, 255, 0.5);"
          "transform: scale(1.05);"
        "}"
      "</style>"
      "<script>"
        // Auto-refresh camera image every 2 seconds
        "function refreshCamera() {"
          "var img = document.getElementById('camImage');"
          "img.src = '/camera?t=' + new Date().getTime();"
        "}"
        "function startAutoRefresh() {"
          "refreshCamera();"
          "setInterval(refreshCamera, 2000);"  // Refresh every 2 seconds
        "}"
      "</script>"
    "</head>"
    "<body onload='startAutoRefresh()'>"
      "<div class='container'>"
        "<h1>&#x1F4F7; ESP32-CAM Portal</h1>"
        "<div class='subtitle'>Live Camera Feed</div>"
        "<div class='camera-container'>"
          "<img id='camImage' class='camera-image' src='/camera' alt='Camera Feed'>"
        "</div>"
        "<button class='refresh-btn' onclick='refreshCamera()'>&#x1F504; Refresh Now</button>"
        "<div class='info'>"
          "<p><strong>Status:</strong> Connected to ESP32-CAM</p>"
          "<p><strong>IP Address:</strong> 192.168.4.1</p>"
          "<p>Camera feed updates automatically every 2 seconds</p>"
        "</div>"
      "</div>"
    "</body>"
    "</html>";
  
  server.send(200, "text/html", html);
}

/**
 * Camera Snapshot Handler
 * Captures and returns a JPEG image from the camera
 * This endpoint is called by the landing page to display the live feed
 */
void handleCamera() {
  camera_fb_t * fb = NULL;
  
  // Capture frame from camera
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    server.send(503, "text/plain", "Camera capture failed");
    return;
  }
  
  // Send JPEG image
  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Disposition", "inline; filename=capture.jpg");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
  
  server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);
  
  // Return frame buffer to be reused
  esp_camera_fb_return(fb);
}

/**
 * 404 Not Found Handler
 * For captive portal functionality, redirect all unknown URLs to the landing page
 * This ensures that users are always directed to our portal page
 */
void handleNotFound() {
  // Redirect to root for captive portal behavior
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

// ============================================================================
// Setup Function - Runs once at startup
// ============================================================================

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  Serial.println("\n\n=== ESP32-CAM Captive Portal ===");
  
  // Initialize camera
  Serial.println("Initializing camera...");
  if (!initCamera()) {
    Serial.println("Camera initialization failed!");
    Serial.println("Please check camera connections and restart");
    while(1) {
      delay(1000); // Halt execution if camera fails
    }
  }
  
  // Configure and start WiFi Access Point
  Serial.println("Starting WiFi Access Point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  Serial.print("Access Point started: ");
  Serial.println(AP_SSID);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
  
  // Start DNS server for captive portal
  // Redirect all DNS requests to this device's IP
  dnsServer.start(DNS_PORT, "*", AP_IP);
  Serial.println("DNS server started");
  
  // Configure HTTP server routes
  server.on("/", handleRoot);              // Landing page
  server.on("/camera", handleCamera);      // Camera snapshot
  server.onNotFound(handleNotFound);       // Catch-all for captive portal
  
  // Start HTTP server
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("\n=== Setup Complete ===");
  Serial.println("Connect to WiFi: " + String(AP_SSID));
  Serial.println("Then open any website to see the portal\n");
}

// ============================================================================
// Loop Function - Runs continuously
// ============================================================================

void loop() {
  // Process DNS requests (for captive portal redirection)
  dnsServer.processNextRequest();
  
  // Process HTTP requests
  server.handleClient();
  
  // Small delay to prevent watchdog timer issues
  delay(1);
}
