#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"
#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>

// Parámetros de la red Wi-Fi
const char* ssid = "TuSSID";
const char* password = "TuContraseña";

// Configuración de Telegram Bot
#define BOT_TOKEN "TuToken"
#define CHAT_ID "TuChatID"

// Inicialización del cliente Wi-Fi
WiFiClientSecure client;

// Inicialización del cliente Telegram Bot
UniversalTelegramBot bot(BOT_TOKEN, client);

// Configuración de la cámara
camera_fb_t *fb = NULL;
bool cameraInitialized = false;

// Función para capturar y enviar la imagen
void captureAndSendImage() {
  if (!cameraInitialized) {
    cameraInitialized = true;
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
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
      config.frame_size = FRAMESIZE_UXGA;
      config.jpeg_quality = 10;
      config.fb_count = 2;
    } else {
      config.frame_size = FRAMESIZE_SVGA;
      config.jpeg_quality = 12;
      config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
      Serial.println("Error al inicializar la cámara");
      return;
    }
  }

  // Capturar imagen
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Error al capturar la imagen");
    return;
  }

  // Enviar imagen por Telegram
  if (client.connect("api.telegram.org", 443)) {
    String randomName = "/captura" + String(millis()) + ".jpg";
    String request = "POST /bot" + String(BOT_TOKEN) + "/sendPhoto?chat_id=" + String(CHAT_ID) + " HTTP/1.1\r\n" +
                     "Host: api.telegram.org\r\n" +
                     "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n" +
                     "Content-Length: " + String(fb->len) + "\r\n" +
                     "Connection: close\r\n\r\n";
    client.println(request);

    client.write((uint8_t*)fb->buf, fb->len);

    esp_camera_fb_return(fb);
    fb = NULL;

    Serial.println("Imagen enviada correctamente");
  } else {
    Serial.println("Error al conectar con Telegram");
  }
  client.stop();
}

void setup() {
  Serial.begin(115200);

  // Conectar a la red Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la red Wi-Fi...");
  }

  Serial.println("Conexión Wi-Fi establecida");
}

void loop() {
  // Esperar a que se presione el botón para capturar y enviar la imagen
  if (digitalRead(13) == HIGH) {
    captureAndSendImage();
    delay(5000);  // Retraso para evitar envío múltiple por un solo toque
  }

  delay(100);
}