#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ========================================
// PINOS
// ========================================
#define BOTAO_WIFI_ON   D0
#define BOTAO_WIFI_OFF  D1
#define BUZZER          D5
#define DHTPIN          D2
#define DHTTYPE         DHT11

// ========================================
// WIFI
// ========================================
const char* WIFI_SSID = "Cyber-Projeto";
const char* WIFI_PASS = "Senai@122";

// ========================================
// API PYTHON
// ========================================
String servidor = "192.168.0.117";
const String AUTH_TOKEN = "CyberProject";

// ========================================
// SENSOR
// ========================================
String nomeSensor = "Temperatura Sala";

// ========================================
// OBJETOS
// ========================================
DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);

// ========================================
// CONTROLE
// ========================================
bool wifiAtivo = false;
unsigned long ultimoEnvio = 0;

// ========================================
// FUNÇÃO: STATUS TEMPERATURA
// ========================================
String obterEstado(float temperatura) {

  if (isnan(temperatura))
    return "FALHA_SENSOR";

  if (temperatura >= 40)
    return "CRITICO";

  if (temperatura >= 30)
    return "ALERTA";

  return "NORMAL";
}

// ========================================
// FUNÇÃO: DATA/HORA API
// ========================================
String obterDataHoraServidor() {

  if (WiFi.status() != WL_CONNECTED)
    return "--";

  WiFiClient client;
  HTTPClient http;

  client.setTimeout(2000);

  String url = "http://" + servidor + ":5000/hora";

  if (!http.begin(client, url))
    return "--";

  http.setTimeout(2000);

  int httpCode = http.GET();

  if (httpCode == 200) {

    String payload = http.getString();

    DynamicJsonDocument doc(256);

    if (!deserializeJson(doc, payload)) {

      String dataHora = doc["datahora"].as<String>();

      http.end();

      return dataHora;
    }
  }

  http.end();

  return "--";
}

// ========================================
// FUNÇÃO: ENVIAR DADOS API
// ========================================
void enviarDadosAPI(float temperatura, float umidade) {

  if (WiFi.status() != WL_CONNECTED)
    return;

  String estado = obterEstado(temperatura);

  DynamicJsonDocument doc(256);

  doc["temperatura"] = isnan(temperatura) ? 0.0 : temperatura;
  doc["umidade"]     = isnan(umidade) ? 0.0 : umidade;
  doc["rotacao"]     = 0;
  doc["estado"]      = estado;

  String jsonBody;

  serializeJson(doc, jsonBody);

  WiFiClient client;
  HTTPClient http;

  client.setTimeout(3000);

  String url = "http://" + servidor + ":5000/receive_data";

  if (!http.begin(client, url))
    return;

  http.setTimeout(3000);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", AUTH_TOKEN);

  int httpCode = http.POST(jsonBody);

  Serial.println("========================");
  Serial.println("ENVIO API");
  Serial.print("HTTP: ");
  Serial.println(httpCode);
  Serial.println("========================");

  http.end();
}

// ========================================
// CONECTAR WIFI
// ========================================
void conectarWiFi() {

  Serial.println("Conectando WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long inicio = millis();

  while (millis() - inicio < 15000) {

    yield();

    if (WiFi.status() == WL_CONNECTED) {

      wifiAtivo = true;

      Serial.println("");
      Serial.println("WiFi conectado!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());

      server.begin();

      Serial.print("Acesse: http://");
      Serial.println(WiFi.localIP());

      return;
    }

    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("ERRO: WiFi nao conectou.");

  WiFi.disconnect();
}

// ========================================
// DESCONECTAR WIFI
// ========================================
void desconectarWiFi() {

  Serial.println("Desconectando WiFi...");

  WiFi.disconnect();

  server.stop();

  wifiAtivo = false;

  Serial.println("WiFi desconectado.");
}

// ========================================
// SETUP
// ========================================
void setup() {

  Serial.begin(115200);

  pinMode(BOTAO_WIFI_ON, INPUT_PULLUP);
  pinMode(BOTAO_WIFI_OFF, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  dht.begin();

  Serial.println("========================");
  Serial.println("SISTEMA INICIADO");
  Serial.println("BOTAO D0 = CONECTAR");
  Serial.println("BOTAO D1 = DESCONECTAR");
  Serial.println("========================");
}

// ========================================
// LOOP
// ========================================
void loop() {

  // ====================================
  // CONECTAR WIFI
  // ====================================
  if (digitalRead(BOTAO_WIFI_ON) == LOW && !wifiAtivo) {

    conectarWiFi();

    delay(1000);
  }

  // ====================================
  // DESCONECTAR WIFI
  // ====================================
  if (digitalRead(BOTAO_WIFI_OFF) == LOW && wifiAtivo) {

    desconectarWiFi();

    delay(1000);
  }

  // ====================================
  // LEITURA SENSOR
  // ====================================
  if (millis() - ultimoEnvio > 20000) {

    float temperatura = dht.readTemperature();
    float umidade     = dht.readHumidity();

    String estado = obterEstado(temperatura);

    Serial.println("========================");
    Serial.print("Sensor: ");
    Serial.println(nomeSensor);

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" C");

    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.println(" %");

    Serial.print("Estado: ");
    Serial.println(estado);

    Serial.print("Horario API: ");
    Serial.println(obterDataHoraServidor());

    Serial.println("========================");

    // ALERTA SONORO
    if (!isnan(temperatura) && temperatura < 27) {

      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
    }

    // ENVIO API
    if (wifiAtivo) {

      enviarDadosAPI(temperatura, umidade);
    }

    ultimoEnvio = millis();
  }

  // ====================================
  // WEB SERVER
  // ====================================
  if (wifiAtivo) {

    server.handleClient();
  }
}