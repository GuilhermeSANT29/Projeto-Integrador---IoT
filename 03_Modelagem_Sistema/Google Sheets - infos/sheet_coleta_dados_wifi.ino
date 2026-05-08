#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

// =================== DHT11 ===================

#define DHTPIN D4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// =================== BOTÃO ===================

#define BUTTON D3

// =================== WIFI ===================

const char* ssid = "os_compilados";
const char* password = "dudinha boboca";

// URL DO WEB APP
const char* serverURL = "https://docs.google.com/spreadsheets/d/19nM9DBSt4Us8D6WIgvMR_EWAKRm2vzoFjrEar4eMZt0/edit?usp=sharing";

// =================== SETUP ===================

void setup() {

  Serial.begin(115200);

  dht.begin();

  pinMode(BUTTON, INPUT_PULLUP);

  // ================= WIFI =================

  WiFi.begin(ssid, password);

  Serial.print("Conectando");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado!");
}

// =================== LOOP ===================

void loop() {

  float temperatura = dht.readTemperature();

  float umidade = dht.readHumidity();

  int botao = digitalRead(BUTTON);

  // ================= JSON =================

  String json = "{";

  json += "\"temperatura\":";
  json += temperatura;
  json += ",";

  json += "\"umidade\":";
  json += umidade;
  json += ",";

  json += "\"botao\":";
  json += botao;

  json += "}";

  // ================= SERIAL =================

  Serial.println(json);

  // ================= ENVIO =================

  if (WiFi.status() == WL_CONNECTED) {

    WiFiClient client;

    HTTPClient http;

    http.begin(client, serverURL);

    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST(json);

    Serial.print("Codigo HTTP: ");

    Serial.println(httpCode);

    http.end();
  }

  delay(5000);
}
