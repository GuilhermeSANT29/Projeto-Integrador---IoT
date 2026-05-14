#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// ================= MAPEAMENTO DE PINOS =================
#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define ROT_PIN D5
#define IR_PIN D6
#define RGB_R D7
#define RGB_G D8
#define RGB_B D0
#define BUZZ D1 // Pino do Buzzer ajustado

// ================= CONFIGURAÇÕES DE REDE E API =================
// CREDENCIAIS ATUALIZADAS
const char* ssid = "Cyber-Projeto";
const char* password = "Senai@122";

// IP da máquina onde a API Python está a correr
const char* apiUrl = "http://10.106.208.26:5000/data"; 
const char* token = "CyberProject";

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Configuração dos pinos
  pinMode(ROT_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(RGB_R, OUTPUT); 
  pinMode(RGB_G, OUTPUT); 
  pinMode(RGB_B, OUTPUT);
  pinMode(BUZZ, OUTPUT);
  
  // Ligação ao Wi-Fi
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
  // 1. Leitura de Sensores
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
  int rotacao = pulseIn(ROT_PIN, HIGH);
  bool irDetect = digitalRead(IR_PIN);
  String status = "normal";
  
  // Tratamento de erro do sensor DHT
  if (isnan(hum) || isnan(temp)) {
    Serial.println("Falha na leitura do DHT! Verifique as ligações.");
    delay(2000);
    return;
  }
  
  // 2. Lógica Local (Alertas baseados na temperatura/umidade/presença)
  if (temp > 40) {
    status = "critico"; 
    // Acende Vermelho e liga Buzzer
    digitalWrite(RGB_R, HIGH); digitalWrite(RGB_G, LOW); digitalWrite(RGB_B, LOW);
    digitalWrite(BUZZ, HIGH);
  } else if (temp > 30 || hum > 80 || irDetect) {
    status = "alerta"; 
    // Acende Amarelo/Laranja e desliga Buzzer
    digitalWrite(RGB_R, HIGH); digitalWrite(RGB_G, HIGH); digitalWrite(RGB_B, LOW);
    digitalWrite(BUZZ, LOW);
  } else {
    status = "normal"; 
    // Acende Verde e desliga Buzzer
    digitalWrite(RGB_R, LOW); digitalWrite(RGB_G, HIGH); digitalWrite(RGB_B, LOW);
    digitalWrite(BUZZ, LOW);
  }
  
  // 3. Montagem do pacote JSON
  StaticJsonDocument<200> doc;
  doc["temp"] = temp; 
  doc["hum"] = hum; 
  doc["rot"] = rotacao;
  doc["ir"] = irDetect; 
  doc["status"] = status;
  
  String jsonPayload;
  serializeJson(doc, jsonPayload);
  Serial.println("A preparar envio: " + jsonPayload);
  
  // 4. Envio para a API via POST
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    
    Serial.print("A conectar a: ");
    Serial.println(apiUrl);
    
    http.begin(client, apiUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", token); // Token de Segurança da API
    
    int httpCode = http.POST(jsonPayload);
    
    if (httpCode > 0) {
      Serial.printf("Resposta da API: %d\n", httpCode); // 201 significa Sucesso (Created)
      if (httpCode == 201) {
        Serial.println("-> Sucesso! Dados registados no sistema.");
      }
    } else {
      Serial.printf("Erro no envio: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end(); // Fecha a ligação
  } else {
    Serial.println("Erro: Wi-Fi desligado. A tentar reconectar...");
  }
  
  Serial.println("-------------------------------------");
  delay(5000); // Aguarda 5 segundos antes da próxima leitura
}