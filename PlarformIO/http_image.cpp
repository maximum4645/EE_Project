#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"

// WiFi credentials
const char* ssid = "WIFI_SSID";
const char* password = "PASSWORD";

// Server URL
const char* serverUrl = "SERVER_IP";

// AI Thinker Camera Module Pins (directly added to avoid issues)
#define PWDN_GPIO_NUM    32
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27
#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      21
#define Y4_GPIO_NUM      19
#define Y3_GPIO_NUM      18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

// Static camera configuration (global)
static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .xclk_freq_hz = 20000000,  // XCLK 20MHz
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,  // Use JPEG format
    .frame_size = FRAMESIZE_QVGA,  // Frame size
    .jpeg_quality = 12,  // Lower means better quality
    .fb_count = 1,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

#define LED_PIN 33 // ACTIVE LOW
#define LED_ON LOW   // LED is ON when pin is LOW
#define LED_OFF HIGH // LED is OFF when pin is HIGH

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);  // Set the LED pin as output
  digitalWrite(LED_PIN, LED_OFF); // Make sure the LED is off initially
  WiFi.begin(ssid, password);

  // Wait for connection to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize the camera with the static camera configuration
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Turn on the onboard LED to indicate image capture
    digitalWrite(LED_PIN, LED_ON);
    
    // Capture an image
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      digitalWrite(LED_PIN, LED_OFF);  // Turn off the LED if capture fails
      return;
    }

    Serial.println("Image captured!");

    // Send image over HTTP
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "image/jpeg");

    // Send POST request with image data
    int httpResponseCode = http.POST(fb->buf, fb->len);

    if (httpResponseCode > 0) {
      Serial.printf("Image sent. Response code: %d\n", httpResponseCode);
    } else {
      Serial.printf("Error sending image. HTTP response code: %d\n", httpResponseCode);
    }

    // End the HTTP connection
    http.end();

    // Return the frame buffer back to the driver for reuse
    esp_camera_fb_return(fb);
    
    // Turn off the onboard LED after sending the image
    digitalWrite(LED_PIN, LED_OFF);

    // Delay between captures
    delay(1000);  // Adjust this value to control the frequency
  } else {
    Serial.println("WiFi not connected");
  }
}
