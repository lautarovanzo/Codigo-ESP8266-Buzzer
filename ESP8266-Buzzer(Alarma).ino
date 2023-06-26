#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configuración de Wi-Fi
const char* ssid = "nombre_red_wifi";
const char* password = "contraseña_wifi";

// Configuración del servidor MQTT
const char* mqtt_server = "dirección_ip_broker";
const char* mqtt_username = "usuario_broker";
const char* mqtt_password = "contraseña_broker";
const char* mqtt_topic = "timbre";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

const int buzzerPin = D1; // Pin del buzzer

void setup() {
  Serial.begin(115200);

  pinMode(buzzerPin, OUTPUT);

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
  if (mqttClient.connect("ESP8266", mqtt_username, mqtt_password)) {
    mqttClient.subscribe(mqtt_topic);
  } else {
    Serial.println("Error al conectar al broker MQTT.");
  }
}

void loop() {
  if (mqttClient.connected()) {
    mqttClient.loop();
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Realizar una acción cuando se reciba un mensaje MQTT
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (message == "ring") {
    activateBuzzer();
  }
}

void activateBuzzer() {
  digitalWrite(buzzerPin, HIGH); // Activar el buzzer
  delay(1000); // Mantener el buzzer activo durante 1 segundo
  digitalWrite(buzzerPin, LOW); // Desactivar el buzzer
}
