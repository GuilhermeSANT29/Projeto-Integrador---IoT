/*
  ETAPA 2 - SISTEMA IOT FINAL
  ESP8266 + Sensores + Google Sheets API
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

// =================== PINOS ===================
#define DHTPIN     D4
#define DHTTYPE    DHT11

#define BUZZER     D5
#define IR_PIN     D6
#define POT_PIN    A0

#define BUTTON1    D2
#define BUTTON2    D3

#define LED_R      D1
#define LED_G      D7
#define LED_B      D0   // corrigido (evita problema no boot)

// =================== WIFI ===================
const char* ssid = "os_compilados";
const char* password = "dudinha boboca";

// =================== GOOGLE SCRIPT (OBRIGATÓRIO ALTERAR) ===================
const char* serverName = "https://script.google.com/macros/s/SEU_ID_AQUI/exec";

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);
  pinMode(IR_PIN, INPUT_PULLUP);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  dht.begin();
  setRGB(0, 0, 0);

  conectarWiFi();
}

// =================== LOOP ===================
void loop() {

  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  int rotacao = map(analogRead(POT_PIN), 0, 1023, 0, 100);

  bool ir = digitalRead(IR_PIN) == LOW;
  bool b1 = digitalRead(BUTTON1) == LOW;
  bool b2 = digitalRead(BUTTON2) == LOW;

  // =================== LÓGICA ===================
  if (temperatura > 30 || b1) {
    digitalWrite(BUZZER, HIGH);
    setRGB(255, 0, 0);
  }
  else if (umidade < 40) {
    digitalWrite(BUZZER, LOW);
    setRGB(0, 0, 255);
  }
  else {
    digitalWrite(BUZZER, LOW);
    setRGB(0, 255, 0);
  }

  // =================== ENVIO ===================
  enviarDados(temperatura, umidade, rotacao, ir, b1, b2);

  delay(5000);
}

// =================== FUNÇÃO ENVIO ===================
void enviarDados(float temp, float umi, int rot, bool ir, bool b1, bool b2) {

  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
    return;
  }

  HTTPClient http;

  String url = String(serverName) +
    "?temperatura=" + String(temp) +
    "&umidade=" + String(umi) +
    "&rotacao=" + String(rot) +
    "&ir=" + String(ir) +
    "&b1=" + String(b1) +
    "&b2=" + String(b2);

  http.begin(url);

  int httpCode = http.GET();

  Serial.print("Envio HTTP: ");
  Serial.println(httpCode);

  http.end();
}

// =================== WIFI ===================
void conectarWiFi() {
  WiFi.begin(ssid, password);

  Serial.print("Conectando WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Conectado!");
  Serial.println(WiFi.localIP());
}

// =================== LED RGB ===================
void setRGB(int r, int g, int b) {
  analogWrite(LED_R, 255 - r);
  analogWrite(LED_G, 255 - g);
  analogWrite(LED_B, 255 - b);
}
