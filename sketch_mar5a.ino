// Include libraries
#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Konfigurasi pin
#define SWITCH_PIN 13      // Switch untuk menyalakan/mematikan perangkat
#define BUTTON_PIN 12      // Push button untuk mengambil gambar

// URL server untuk mengirim gambar
const char* serverUrl = "http://your-server-url.com/upload";

// Variabel untuk menyimpan status perangkat
bool deviceOn = false;

void setup() {
    Serial.begin(115200);

    // Inisialisasi WiFi
    WiFi.begin("your_ssid", "your_password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Inisialisasi kamera
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sscb_sda = 26;
    config.pin_sscb_scl = 27;
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_SVGA;

    // Inisialisasi kamera
    if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("Camera init failed!");
        return;
    }

    // Set pin mode
    pinMode(SWITCH_PIN, INPUT_PULLUP);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void takeAndSendPicture() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }
    
    Serial.println("Sending picture to server...");
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "image/jpeg");
        int httpResponseCode = http.POST(fb->buf, fb->len);
        Serial.printf("Server response: %d\n", httpResponseCode);
        http.end();
    }
    
    esp_camera_fb_return(fb);
    delay(1000); // Delay setelah pengiriman gambar
}

void loop() {
    // Cek status switch
    deviceOn = (digitalRead(SWITCH_PIN) == LOW);
    if (!deviceOn) {
        Serial.println("Device OFF");
        return; // Jika device OFF, keluar dari loop
    }
    Serial.println("Device ON");

    // Cek status WiFi
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, reconnecting...");
        WiFi.begin("your_ssid", "your_password");
        return;
    }

    // Cek tombol dengan debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(50); // Debounce
        if (digitalRead(BUTTON_PIN) == LOW) { // Pastikan tombol masih ditekan
            Serial.println("Taking picture...");
            takeAndSendPicture();
        }
    }
}
