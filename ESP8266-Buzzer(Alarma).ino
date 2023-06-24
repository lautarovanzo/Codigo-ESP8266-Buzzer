#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

// Parámetros de la red Wi-Fi
const char* ssid = "TuSSID";
const char* password = "TuContraseña";

// Configuración del cliente MQTT
const char* mqttServer = "mqtt.example.com";
const int mqttPort = 1883;
const char* mqttUser = "TuUsuario";
const char* mqttPassword = "TuContraseña";

// Configuración del buzzer
const int buzzerPin = D1;

// Inicialización del cliente Wi-Fi
WiFiClient client;

// Inicialización del cliente MQTT
Adafruit_MQTT_Client mqtt(&client, mqttServer, mqttPort, mqttUser, mqttPassword);

// Canal MQTT para recibir mensajes
Adafruit_MQTT_Subscribe alarmChannel = Adafruit_MQTT_Subscribe(&mqtt, "home/alarm");

void setup() {
  Serial.begin(115200);

  // Conectar a la red Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la red Wi-Fi...");
  }

  Serial.println("Conexión Wi-Fi establecida");

  // Configurar MQTT
  mqtt.subscribe(&alarmChannel);
}

void loop() {
  // Reconectar si se pierde la conexión MQTT
  if (!mqtt.connected()) {
    Serial.println("Conexión MQTT perdida. Intentando reconexión...");
    reconnectMQTT();
  }

  // Manejar mensajes MQTT
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(1000))) {
    if (subscription == &alarmChannel) {
      // Activar el buzzer
      digitalWrite(buzzerPin, HIGH);
      delay(500);
      digitalWrite(buzzerPin, LOW);
    }
  }
}

void reconnectMQTT() {
  while (!mqtt.connected()) {
    if (mqtt.connect()) {
      Serial.println("Conexión MQTT exitosa");
      mqtt.subscribe(&alarmChannel);
    } else {
      Serial.print("Error al conectar con el servidor MQTT. Estado: ");
      Serial.println(mqtt.connectError());
      delay(5000);
    }
  }
}