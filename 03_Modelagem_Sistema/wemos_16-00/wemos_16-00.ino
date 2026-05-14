#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// ================= MAPEAMENTO DE PINOS =================
#define TEMP_PIN D4
#define ROT_PIN A0
#define IR_PIN D6
#define BUZZ D5
#define RGB_R D7
#define RGB_G D8
#define RGB_B D0

// ================= CONFIGURAÇÕES DE REDE E API =================
const char* ssid = "Cyber-Projeto";
const char* password = "Senai@122";
const char* apiUrl = "http://10.106.208.26:5000/data";
const char* token = "CyberProject";

// ================= LEITURA DA TEMPERATURA =================
// Ajuste a fórmula conforme o sensor de temperatura real que está no D4
float readTemperature() {
  int raw = digitalRead(TEMP_PIN);
  return raw;
}

// ================= LEITURA DA ROTAÇÃO =================
// A0 lê o valor analógico do sensor de rotação
int readRotation() {
  return analogRead(ROT_PIN);
}

// ================= CONTROLE DO RGB =================
// Se o LED for common anode, inverta HIGH/LOW
void setNormalState() {
  digitalWrite(RGB_R, LOW);
  digitalWrite(RGB_G, HIGH);
  digitalWrite(RGB_B, LOW);
  digitalWrite(BUZZ, LOW);
}

void setAlertState() {
  digitalWrite(RGB_R, HIGH);
  digitalWrite(RGB_G, HIGH);
  digitalWrite(RGB_B, LOW);
  digitalWrite(BUZZ, LOW);
}

void setCriticalState() {
  digitalWrite(RGB_R, HIGH);
  digitalWrite(RGB_G, LOW);
  digitalWrite(RGB_B, LOW);
  digitalWrite(BUZZ, HIGH);
}

void setup() {
  Serial.begin(115200);

  pinMode(TEMP_PIN, INPUT);
  pinMode(ROT_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(BUZZ, OUTPUT);
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);

  setNormalState();

  Serial.println();
  Serial.println("=====================================");
  Serial.print("A ligar à rede Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi Ligado com Sucesso!");
  Serial.print("IP do ESP8266: ");
  Serial.println(WiFi.localIP());
  Serial.println("=====================================\n");
}

void loop() {
  float temp = readTemperature();
  int rotacao = readRotation();
  bool irDetect = digitalRead(IR_PIN);

  String status = "normal";

  if (temp > 40) {
    status = "critico";
    setCriticalState();
  } else if (temp > 30 || irDetect) {
    status = "alerta";
    setAlertState();
  } else {
    status = "normal";
    setNormalState();
  }

  StaticJsonDocument<200> doc;
  doc["temp"] = temp;
  doc["rot"] = rotacao;
  doc["ir"] = irDetect;
  doc["status"] = status;

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  Serial.println("A preparar envio: " + jsonPayload);

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    http.begin(client, apiUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", token);

    int httpCode = http.POST(jsonPayload);

    if (httpCode > 0) {
      Serial.printf("Resposta da API: %d\n", httpCode);
      if (httpCode == 201) {
        Serial.println("-> Sucesso! Dados registados no sistema.");
      }
    } else {
      Serial.printf("Erro no envio: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.println("Erro: Wi-Fi desligado. A tentar reconectar...");
  }

  Serial.println("-------------------------------------");
  delay(5000);
}