#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#define TEMP_PIN A0
#define BUZZ    D5
#define IR_PIN  D6

#define RGB_R   D1
#define RGB_G   D2
#define RGB_B   D7

const char* ssid = "Cyber-Projeto";
const char* password = "Senai@122";
const char* apiUrl = "http://10.106.202.20:5000/data";
const char* token = "CyberProject";

float readTemperature() {
  int raw = analogRead(TEMP_PIN);
  float celsius = map(raw, 0, 1023, 1500, 5000) / 100.0;
  return celsius;
}

void playMarioTheme() {
  int melody[] = {
    659, 659, 0, 659, 0, 523, 659, 0, 784, 0,
    392, 0, 523, 0, 392, 0, 330, 0, 440, 0,
    494, 0, 466, 440, 0, 392, 659, 784
  };

  int noteDurations[] = {
    150, 150, 75, 150, 75, 150, 150, 75, 150, 75,
    150, 75, 150, 75, 150, 75, 150, 75, 150, 75,
    150, 75, 150, 150, 75, 150, 150, 150
  };

  for (int i = 0; i < 28; i++) {
    if (melody[i] == 0) {
      noTone(BUZZ);
    } else {
      tone(BUZZ, melody[i]);
    }
    delay(noteDurations[i]);
    noTone(BUZZ);
    delay(30);
  }
}

void setNormalState() {
  digitalWrite(RGB_R, LOW);
  digitalWrite(RGB_G, HIGH);
  digitalWrite(RGB_B, LOW);
  noTone(BUZZ);
  digitalWrite(BUZZ, LOW);
}

void setAlertState() {
  digitalWrite(RGB_R, HIGH);
  digitalWrite(RGB_G, HIGH);
  digitalWrite(RGB_B, LOW);
  digitalWrite(BUZZ, HIGH);
  delay(100);
  digitalWrite(BUZZ, LOW);
}

void setCriticalState() {
  digitalWrite(RGB_R, HIGH);
  digitalWrite(RGB_G, LOW);
  digitalWrite(RGB_B, LOW);
  playMarioTheme();
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZ, OUTPUT);
  pinMode(IR_PIN, INPUT_PULLUP);
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);

  setNormalState();

  Serial.println("\n=====================================");
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi Conectado!");
  Serial.println("=====================================\n");
}

void loop() {
  float temp = readTemperature();
  bool irDetect = digitalRead(IR_PIN);
  int rotacao = 0;
  String status = "normal";

  bool objetoDetetado = (irDetect == LOW);

  if (temp > 40.0) {
    status = "critico";
    setCriticalState();
  } else if (temp > 30.0 || objetoDetetado) {
    status = "alerta";
    setAlertState();
  } else {
    status = "normal";
    setNormalState();
  }

  StaticJsonDocument<256> doc;
  doc["temp"] = temp;
  doc["hum"] = 60.0;
  doc["rot"] = rotacao;
  doc["ir"] = objetoDetetado ? 1 : 0;
  doc["status"] = status;

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    http.begin(client, apiUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", token);

    Serial.print("A enviar dados: ");
    Serial.println(jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.print("Resposta da API (HTTP Code): ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Erro no envio do POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Erro: Wi-Fi desconectado!");
  }

  delay(5000);
}