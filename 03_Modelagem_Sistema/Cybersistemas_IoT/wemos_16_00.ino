#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// ================= MAPEAMENTO DE PINOS (Shield HY-M302) =================
#define TEMP_PIN A0   // Pino analógico que vai ler o Potenciômetro
#define BUZZ     14   // Pino do Buzzer
#define IR_PIN   12   // Sensor IR (Infravermelho)

// Pinos RGB
#define RGB_R    13
#define RGB_G    15
#define RGB_B    16

// ================= CONFIGURAÇÕES DE REDE E API =================
const char* ssid = "Cyber-Projeto";
const char* password = "Senai@122";
const char* apiUrl = "http://10.106.208.26:5000/data";
const char* token = "CyberProject";

// ================= LEITURA DA TEMPERATURA (SIMULADA PELO BOTÃO) =================
float readTemperature() {
  // Lê a posição do Potenciômetro (Botão azul) no pino A0
  int raw = analogRead(TEMP_PIN);
  
  // Transforma o giro do botão (0 a 1023) em uma temperatura simulada entre 15.0 °C e 50.0 °C
  // Multiplicamos por 100 para manter as casas decimais na divisão
  float celsius = map(raw, 0, 1023, 1500, 5000) / 100.0; 

  return celsius;
}

// ================= CONTROLE DE ESTADOS =================
void setNormalState() {
  digitalWrite(RGB_R, LOW);
  digitalWrite(RGB_G, HIGH); // Verde
  digitalWrite(RGB_B, LOW);
  digitalWrite(BUZZ, LOW);   // Buzzer desligado
}

void setAlertState() {
  digitalWrite(RGB_R, HIGH); // Vermelho + Verde = Amarelo/Laranja
  digitalWrite(RGB_G, HIGH);
  digitalWrite(RGB_B, LOW);
  digitalWrite(BUZZ, LOW);   // Buzzer desligado
}

void setCriticalState() {
  digitalWrite(RGB_R, HIGH); // Vermelho
  digitalWrite(RGB_G, LOW);
  digitalWrite(RGB_B, LOW);
  digitalWrite(BUZZ, HIGH);  // Buzzer LIGADO!
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  
  pinMode(IR_PIN, INPUT);
  pinMode(BUZZ, OUTPUT);
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

// ================= LOOP =================
void loop() {
  float temp = readTemperature();
  bool irDetect = digitalRead(IR_PIN);
  int rotacao = 0;
  String status = "normal";

  // Lógica de Estados baseada na temperatura simulada
  if (temp > 40.0) {
    status = "critico";
    setCriticalState();
  } else if (temp > 30.0 || irDetect) {
    status = "alerta";
    setAlertState();
  } else {
    status = "normal";
    setNormalState();
  }

  // Montagem do JSON
  StaticJsonDocument<200> doc;
  doc["temp"] = temp;
  doc["rot"] = rotacao;
  doc["ir"] = irDetect;
  doc["status"] = status;

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  // Envio para a API
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    
    http.begin(client, apiUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", token);

    int httpCode = http.POST(jsonPayload);
    Serial.printf("Temp Simulada: %.2f C | Status: %s | Resposta API: %d\n", temp, status.c_str(), httpCode);
    
    http.end();
  } else {
    Serial.println("Erro: Wi-Fi desconectado.");
  }

  delay(5000); // Aguarda 5 segundos para a próxima leitura
}