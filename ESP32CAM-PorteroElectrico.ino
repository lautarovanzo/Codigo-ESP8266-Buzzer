#include <WiFi.h>
#include <WiFiClient.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"
#include <PubSubClient.h>

// Configuración de la cámara
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// Configuración de Wi-Fi
const char* ssid = "nombre_red_wifi";
const char* password = "contraseña_wifi";

// Configuración del bot de Telegram
#define BOT_TOKEN "token_del_bot"
#define CHAT_ID "ID_del_chat"

// Configuración del servidor MQTT
const char* mqtt_server = "dirección_ip_broker";
const char* mqtt_username = "usuario_broker";
const char* mqtt_password = "contraseña_broker";
const char* mqtt_topic = "timbre";

WiFiClient wifiClient;
UniversalTelegramBot bot(BOT_TOKEN, wifiClient);
WiFiClientSecure client;
PubSubClient mqttClient(client);

bool photoTaken = false;

void setup() {
  Serial.begin(115200);
  pinMode(4, INPUT_PULLUP); // Pin del pulsador

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
  config.jpeg_quality = 10;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error al inicializar la cámara. Código de error: 0x%x", err);
    return;
  }

  // Conexión a Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a Wi-Fi...");
  }

  Serial.println("Conexión establecida.");
  Serial.print("Dirección IP asignada: ");
  Serial.println(WiFi.localIP());

  // Conexión al broker MQTT
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);
  if (mqttClient.connect("ESP32CAM", mqtt_username, mqtt_password)) {
    mqttClient.subscribe(mqtt_topic);
  } else {
    Serial.println("Error al conectar al broker MQTT.");
  }
}

void loop() {
  if (digitalRead(4) == LOW && !photoTaken) {
    takePhoto();
  }

  if (mqttClient.connected()) {
    mqttClient.loop();
  }
}

void takePhoto() {
  camera_fb_t* fb = NULL;
  fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Error al capturar la imagen.");
    return;
  }

  String photoName = "/photo.jpg";

  fs::FS &fs = SPIFFS;
  Serial.printf("Guardando foto en: %s\n", photoName.c_str());

  File photoFile = fs.open(photoName.c_str(), FILE_WRITE);
  if (!photoFile) {
    Serial.println("Error al abrir el archivo.");
  } else {
    photoFile.write(fb->buf, fb->len);
    Serial.println("Foto guardada.");
    photoFile.close();
    photoTaken = true;
    sendTelegramMessage();
  }

  esp_camera_fb_return(fb);
}

void sendTelegramMessage() {
  bot.sendMessage(CHAT_ID, "¡Hay alguien en la puerta!");
  bot.sendPhoto(CHAT_ID, SPIFFS, "/photo.jpg");
}
