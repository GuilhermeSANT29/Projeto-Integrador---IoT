/*
  ETAPA 2 - Versão Final com Google Sheets
  Grupo Alpha - SENAI Hermenegildo Campos de Almeida
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// =================== PINOS ===================
#define DHTPIN     D4
#define BUZZER     D5
#define IR_PIN     D6
#define POT_PIN    A0
#define BUTTON1    D2
#define BUTTON2    D3
#define LED_R      D1
#define LED_G      D7
#define LED_B      D8

#define DHTTYPE    DHT11

// =================== CONFIGURAÇÃO Wi-Fi ===================
const char* ssid = "os_compilados";
const char* password = "dudinha boboca";

// =================== GOOGLE SHEETS ===================
// Substitua pela sua URL do Web App
const char* googleSheetsURL = "https://script.google.com/macros/s/SEU_WEB_APP_URL_AQUI/exec";

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUZZER, OUTPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  dht.begin();
  setRGB(0, 0, 0);

  Serial.println("\n=== ETAPA 2 - Sistema IoT com Google Sheets ===");
  Serial.println("Grupo Alpha - SENAI\n");

  conectarWiFi();
}

void loop() {
  // Leitura dos sensores
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  int rotacao = map(analogRead(POT_PIN), 0, 1023, 0, 100);
  bool irDetectado = digitalRead(IR_PIN) == LOW;
  bool botao1 = digitalRead(BUTTON1) == LOW;
  bool botao2 = digitalRead(BUTTON2) == LOW;

  // Controle de Atuadores
  if (temperatura > 30.0 || botao1) {
    digitalWrite(BUZZER, HIGH);
    setRGB(255, 0, 0);        // Vermelho
  } 
  else if (umidade < 40.0) {
    setRGB(0, 0, 255);        // Azul
    digitalWrite(BUZZER, LOW);
  } 
  else {
    setRGB(0, 255, 0);        // Verde
    digitalWrite(BUZZER, LOW);
  }

  // Criação do JSON
  StaticJsonDocument<400> doc;
  doc["dispositivo"] = "ESP8266_GrupoAlpha";
  doc["temperatura"] = isnan(temperatura) ? -1 : temperatura;
  doc["umidade"] = isnan(umidade) ? -1 : umidade;
  doc["rotacao"] = rotacao;
  doc["ir_detectado"] = irDetectado;
  doc["botao1"] = botao1;
  doc["botao2"] = botao2;
  doc["timestamp"] = millis();
  doc["ip"] = WiFi.localIP().toString();

  String jsonString;
  serializeJson(doc, jsonString);

  Serial.println("\n--- Dados Coletados ---");
  Serial.println(jsonString);

  // Envio para Google Sheets
  enviarParaGoogleSheets(jsonString);

  delay(5000); // 5 segundos
}

// =================== FUNÇÕES ===================
void setRGB(int r, int g, int b) {
  analogWrite(LED_R, 255 - r);
  analogWrite(LED_G, 255 - g);
  analogWrite(LED_B, 255 - b);
}

void conectarWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  
  unsigned long timeout = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - timeout < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Wi-Fi Conectado!");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Falha na conexão Wi-Fi!");
  }
}

void enviarParaGoogleSheets(String json) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ Sem Wi-Fi. Reconectando...");
    conectarWiFi();
    return;
  }

  HTTPClient http;
  http.begin(googleSheetsURL);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(json);

  if (httpCode > 0) {
    Serial.print("✅ Enviado para Google Sheets! Código: ");
    Serial.println(httpCode);
  } else {
    Serial.print("❌ Erro ao enviar para Google Sheets. Código: ");
    Serial.println(httpCode);
  }

  http.end();
}
