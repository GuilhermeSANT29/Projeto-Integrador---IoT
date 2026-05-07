/*
  ETAPA 2 - Versão Final
  Grupo Alpha - SENAI Hermenegildo Campos de Almeida
  ESP8266 + Shield HY-M302
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
#define LED_R      D1     // Vermelho
#define LED_G      D7     // Verde
#define LED_B      D8     // Azul

#define DHTTYPE    DHT11

// =================== CONFIGURAÇÃO Wi-Fi ===================
const char* ssid = "os_compilados";
const char* password = "dudinha boboca";

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Configuração dos pinos
  pinMode(BUZZER, OUTPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  dht.begin();

  // Desliga LEDs RGB inicialmente
  setRGB(0, 0, 0);

  Serial.println("\n=== ETAPA 2 - Sistema IoT Grupo Alpha ===");
  Serial.println("Iniciando coleta de dados e comunicação...\n");

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

  // Controle dos atuadores
  if (temperatura > 30.0 || botao1) {
    digitalWrite(BUZZER, HIGH);
    setRGB(255, 0, 0);        // Vermelho = Alerta
  } 
  else if (umidade < 40.0) {
    setRGB(0, 0, 255);        // Azul = Ambiente Seco
    digitalWrite(BUZZER, LOW);
  } 
  else {
    setRGB(0, 255, 0);        // Verde = Normal
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

  // Envio dos dados
  enviarDadosHTTP(jsonString);

  delay(4000); // Atualiza a cada 4 segundos
}

// =================== FUNÇÕES AUXILIARES ===================
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
    Serial.println("\n✅ Wi-Fi Conectado com sucesso!");
    Serial.print("IP: "); 
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Falha ao conectar no Wi-Fi!");
  }
}

void enviarDadosHTTP(String json) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ Sem conexão. Tentando reconectar...");
    conectarWiFi();
    return;
  }

  HTTPClient http;
  String serverPath = "http://httpbin.org/post";   // Servidor para teste

  http.begin(serverPath);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(json);

  if (httpCode > 0) {
    Serial.print("✅ Dados enviados! Código: ");
    Serial.println(httpCode);
  } else {
    Serial.print("❌ Erro ao enviar. Código: ");
    Serial.println(httpCode);
  }

  http.end();
}
