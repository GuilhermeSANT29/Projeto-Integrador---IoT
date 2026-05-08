#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define ROT_PIN D5
#define IR_PIN D6
#define BTN1 D1
#define RGB_R D7  // Ajuste pins HY-M302
#define RGB_G D8
#define RGB_B D0
#define BUZZ D9

const char* ssid = "Cyber-projeto";
const char* password = "C@necTad0!";
const char* apiUrl = "http://SEU_IP:5000/data";  // Substitua pelo IP da API
const char* token = "seutoken123";

float temp, hum;
int rotacao = 0;
bool irDetect = false;
String status = "normal";

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(ROT_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(BTN1, INPUT);
  pinMode(RGB_R, OUTPUT); pinMode(RGB_G, OUTPUT); pinMode(RGB_B, OUTPUT);
  pinMode(BUZZ, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("WiFi OK");
}

void loop() {
  // Leitura sensores
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  rotacao = pulseIn(ROT_PIN, HIGH);
  irDetect = digitalRead(IR_PIN);
  
  // Lógica local
  if (temp > 30 || hum > 80 || irDetect) {
    status = "alerta"; rgbAlerta(); buzz(500, 200);
  } else if (temp > 40) {
    status = "critico"; rgbCritico(); buzz(1000, 500);
  } else {
    status = "normal"; rgbNormal();
  }
  
  // JSON e envio
  StaticJsonDocument<200> doc;
  doc["temp"] = temp; doc["hum"] = hum; doc["rot"] = rotacao;
  doc["ir"] = irDetect; doc["status"] = status;
  String json;
  serializeJson(doc, json);
  
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(apiUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", token);
    int httpCode = http.POST(json);
    if (httpCode == 200) Serial.println("Enviado OK");
    http.end();
  }
  delay(5000);  // A cada 5s
}

void rgbNormal() { analogWrite(RGB_R, 0); analogWrite(RGB_G, 255); analogWrite(RGB_B, 0); }
void rgbAlerta() { analogWrite(RGB_R, 255); analogWrite(RGB_G, 255); analogWrite(RGB_B, 0); }
void rgbCritico() { analogWrite(RGB_R, 255); analogWrite(RGB_G, 0); analogWrite(RGB_B, 0); }
void buzz(int freq, int dur) { tone(BUZZ, freq, dur); }