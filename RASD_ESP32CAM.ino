/*
  ESP32-CAM Ai-Thinker
  Sends a startup message on boot, and sends a photo when a signal is received on GPIO 2
  Full working code (no external files)
*/

#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "time.h"

// ==== WIFI ====
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ==== TELEGRAM ====
String BOT_TOKEN = "YOUR_BOT_TOKEN";
String CHAT_ID   = "YOUR_CHAT_ID";

// ==== NTP SERVER ====
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 10800; // GMT+3 (Saudi Arabia)
const int   daylightOffset_sec = 0;

// ==== LOCATION ====
String LOCATION = "King Abdulaziz University, Jeddah";

// ==== CAMERA PINS (AI-THINKER) ====
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

// ==== TRIGGER PIN ====
#define TRIGGER_PIN 2

// ====== GET CURRENT TIME AND DATE ======
String getCurrentTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "N/A";
  }
  
  char timeStr[10];
  strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
  return String(timeStr);
}

String getCurrentDate() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "N/A";
  }
  
  char dateStr[20];
  strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
  return String(dateStr);
}

// ====== CREATE VIOLATION MESSAGE ======
String createViolationMessage() {
  String message = "🚨 Violation Recorded\n\n";
  message += "Violation:\n";
  message += "Using disabled parking spaces without authorization.\n\n";
  message += "Time: " + getCurrentTime() + "\n";
  message += "Date: " + getCurrentDate() + "\n";
  message += "Location: " + LOCATION;
  
  return message;
}

// ====== SEND TEXT MESSAGE ======
bool sendTelegramMessage(String text) {
  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("❌ Failed to connect to Telegram");
    return false;
  }

  String encodedText = "";
  for (unsigned int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);
    if (c == '\n') {
      encodedText += "%0A";
    } else if (c == ' ') {
      encodedText += "%20";
    } else if (c == ':') {
      encodedText += "%3A";
    } else if (c == '-') {
      encodedText += "%2D";
    } else if (c == '.') {
      encodedText += "%2E";
    } else if (c == ',') {
      encodedText += "%2C";
    } else {
      encodedText += c;
    }
  }

  String url = "/bot" + BOT_TOKEN + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + encodedText + "&parse_mode=Markdown";

  client.println("GET " + url + " HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("Connection: close");
  client.println();

  unsigned long timeout = millis() + 5000;
  while (!client.available() && millis() < timeout) {
    delay(10);
  }

  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line.indexOf("HTTP/1.1 200 OK") != -1) {
      Serial.println("✅ Message sent successfully");
      return true;
    }
  }

  return false;
}

// ====== SEND PHOTO WITH CAPTION ======
bool sendPhotoWithCaption() {
  WiFiClientSecure client;
  client.setInsecure();

  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("❌ Camera capture failed");
    return false;
  }

  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("❌ Connection to Telegram failed");
    esp_camera_fb_return(fb);
    return false;
  }

  String caption = createViolationMessage();
  
  // Safely encode the caption
  String encodedCaption = "";
  for (unsigned int i = 0; i < caption.length(); i++) {
    char c = caption.charAt(i);
    if (c == '\n') {
      encodedCaption += "%0A";
    } else if (c == ' ') {
      encodedCaption += "%20";
    } else if (c == ':') {
      encodedCaption += "%3A";
    } else if (c == '-') {
      encodedCaption += "%2D";
    } else if (c == '.') {
      encodedCaption += "%2E";
    } else if (c == ',') {
      encodedCaption += "%2C";
    } else if (c == '*') {
      encodedCaption += "%2A";
    } else {
      encodedCaption += c;
    }
  }

  String boundary = "----ESP32CAM";
  String head = "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
  head += CHAT_ID;
  head += "\r\n--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"photo\"; filename=\"violation.jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";

  String middle = "\r\n--" + boundary + "\r\n";
  middle += "Content-Disposition: form-data; name=\"caption\"\r\n\r\n";
  middle += caption;
  
  String tail = "\r\n--" + boundary + "--\r\n";

  uint32_t totalLen = head.length() + fb->len + middle.length() + tail.length();

  client.printf("POST /bot%s/sendPhoto HTTP/1.1\r\n", BOT_TOKEN.c_str());
  client.println("Host: api.telegram.org");
  client.printf("Content-Type: multipart/form-data; boundary=%s\r\n", boundary.c_str());
  client.printf("Content-Length: %u\r\n", totalLen);
  client.println();
  client.print(head);

  client.write(fb->buf, fb->len);
  client.print(middle);
  client.print(tail);

  esp_camera_fb_return(fb);

  unsigned long timeout = millis() + 10000;
  while (!client.available() && millis() < timeout) {
    delay(10);
  }

  bool success = false;
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line.indexOf("HTTP/1.1 200 OK") != -1) {
      Serial.println("✅ Photo with caption sent successfully");
      success = true;
    }
  }

  return success;
}

// ====== CAMERA INIT ======
void setupCamera() {
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

  config.frame_size = FRAMESIZE_VGA; 
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Camera init failed 0x%x\n", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_vflip(s, 1);
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  delay(2000);

  // Set pin 2 as input
  pinMode(TRIGGER_PIN, INPUT_PULLDOWN);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✔️ WiFi Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Initialize NTP time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Wait to get the time
  struct tm timeinfo;
  Serial.print("Waiting for NTP time sync...");
  int retries = 0;
  while (!getLocalTime(&timeinfo) && retries < 10) {
    Serial.print(".");
    delay(1000);
    retries++;
  }
  
  if (retries >= 10) {
    Serial.println("\n⚠️ Could not get NTP time, using default");
  } else {
    Serial.println("\n✔️ Time synchronized!");
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.print("Current time: ");
    Serial.println(timeStr);
  }

  setupCamera();

  // Send startup message
  String startupMsg = "🚓 Parking Violation System\n\n";
  startupMsg += "📡 System is Online and Connected!\n";
  startupMsg += "📍 Location: " + LOCATION + "\n";
  startupMsg += "🕒 Time: " + getCurrentTime() + "\n";
  startupMsg += "📅 Date: " + getCurrentDate() + "\n\n";
  startupMsg += "✅ Ready to detect violations...";
  
  sendTelegramMessage(startupMsg);
}

// ====== LOOP ======
void loop() {
  // Check for a signal on pin 2 (HIGH)
  if (digitalRead(TRIGGER_PIN) == HIGH) {
    Serial.println("🚨 Violation detected! Recording evidence...");
    
    // Send the photo with caption
    if (sendPhotoWithCaption()) {
      Serial.println("✅ Violation report sent with photo and details");
    } else {
      Serial.println("❌ Failed to send violation report");
      // Try sending a text message as a fallback
      String errorMsg = "⚠️ System Alert\n";
      errorMsg += "Failed to send photo report!\n";
      errorMsg += "Violation details:\n";
      errorMsg += createViolationMessage();
      sendTelegramMessage(errorMsg);
    }
    
    // Wait until the signal ends (to avoid multiple captures)
    while (digitalRead(TRIGGER_PIN) == HIGH) {
      delay(100);
    }
    
    // Delay to prevent bouncing and multiple captures
    delay(2000);
  }
  
  delay(100); // Short delay to reduce processor load
}
