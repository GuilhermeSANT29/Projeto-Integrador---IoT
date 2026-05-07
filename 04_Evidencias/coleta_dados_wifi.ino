```cpp
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// Definição dos pinos
#define DHTPIN    D4
#define BUZZER    D5
#define IR_PIN    D6
#define POT_PIN   A0
#define DHTTYPE   DHT11

// Credenciais Wi-Fi
const char* ssid = "SEU_SSID";
const char* password = "SUA_SENHA";

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(BUZZER, OUTPUT);
  pinMode(IR_PIN, INPUT);
  dht.begin();

  Serial.println("\n=== ETAPA 2 - Sistema de Monitoramento IoT ===");
  Serial.println("Grupo Alpha - SENAI Guarulhos\n");

  // Conexão Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando à rede Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado com sucesso!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Leitura dos sensores
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  int potValue = analogRead(POT_PIN);
  int rotacao = map(potValue, 0, 1023, 0, 100);
  int irValue = digitalRead(IR_PIN);

  // Controle do Buzzer
  if (temperatura > 30.0) {
    digitalWrite(BUZZER, HIGH);
  } else {
    digitalWrite(BUZZER, LOW);
  }

  // Criação do JSON
  StaticJsonDocument<300> doc;
  doc["dispositivo"] = "ESP8266_GrupoAlpha";
  doc["temperatura"] = isnan(temperatura) ? -1 : temperatura;
  doc["umidade"] = isnan(umidade) ? -1 : umidade;
  doc["rotacao_percent"] = rotacao;
  doc["ir_detectado"] = (irValue == LOW);
  doc["timestamp"] = millis();
  doc["ip"] = WiFi.localIP().toString();

  String jsonString;
  serializeJson(doc, jsonString);

  // Exibição no Serial Monitor
  Serial.println("\n--- Dados Coletados ---");
  Serial.println(jsonString);

  // Envio dos dados via HTTP (TCP/IP)
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = "http://httpbin.org/post";   // Servidor de teste

    http.begin(serverPath);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(jsonString);

    if (httpResponseCode > 0) {
      Serial.print("✅ Dados enviados com sucesso! Código: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("❌ Erro no envio. Código: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  delay(5000); // Atualiza a cada 5 segundos
}
