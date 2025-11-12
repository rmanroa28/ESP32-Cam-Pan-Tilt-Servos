#include "esp_camera.h"
#include <WiFi.h>
#include <ESP32Servo.h>

// Replace with your Wi-Fi credentials
const char* ssid = "eduroam";
const char* password = "pass";

// Servo setup
Servo servoPan;
Servo servoTilt;
int panPos = 90;
int tiltPos = 90;
int panPin = 12;
int tiltPin = 13;

// Web server
WiFiServer server(80);

// Camera configuration (AI-Thinker board)
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

void startCamera() {
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
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  esp_camera_init(&config);
}

void setup() {
  Serial.begin(115200);
  servoPan.attach(panPin);
  servoTilt.attach(tiltPin);
  servoPan.write(panPos);
  servoTilt.write(tiltPos);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());

  startCamera();
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Control servos via URL, e.g., /move?pan=120&tilt=80
  if (request.indexOf("/move") != -1) {
    int panIndex = request.indexOf("pan=");
    int tiltIndex = request.indexOf("tilt=");
    if (panIndex != -1) panPos = request.substring(panIndex + 4, request.indexOf("&", panIndex)).toInt();
    if (tiltIndex != -1) tiltPos = request.substring(tiltIndex + 5).toInt();
    servoPan.write(panPos);
    servoTilt.write(tiltPos);
  }

  // Serve basic page
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println();
  client.println("<html><body>");
  client.println("<h1>ESP32-CAM Pan/Tilt Control</h1>");
  client.println("<img src='/stream' width='320'><br>");
  client.println("<button onclick=\"fetch('/move?pan=0&tilt=90')\">Left</button>");
  client.println("<button onclick=\"fetch('/move?pan=180&tilt=90')\">Right</button>");
  client.println("<button onclick=\"fetch('/move?pan=90&tilt=0')\">Up</button>");
  client.println("<button onclick=\"fetch('/move?pan=90&tilt=180')\">Down</button>");
  client.println("</body></html>");
  client.stop();
}
