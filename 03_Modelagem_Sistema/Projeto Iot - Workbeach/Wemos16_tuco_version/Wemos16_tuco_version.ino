#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ======================================================
// BOTÕES
// ======================================================
#define BOTAO1 D0
#define BOTAO2 D1

// ======================================================
// BUZZER
// ======================================================
#define BUZZER D5

// ======================================================
// DHT11
// ======================================================
#define DHTPIN D2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// ======================================================
// WIFI
// ======================================================
const char* ssid = "Cyber-Projeto";
const char* password = "Senai@122";

// ======================================================
// IP DO SERVIDOR PYTHON
// ======================================================
String servidor = "10.106.208.39";

// ======================================================
// TOKEN
// ======================================================
const String AUTH_TOKEN = "CyberProject";

// ======================================================
// SENSOR
// ======================================================
String nomeSensor = "Temperatura Sala";

// ======================================================
// SERVIDOR WEB
// ======================================================
ESP8266WebServer server(80);

// ======================================================
// CONTROLE
// ======================================================
bool wifiAtivo = false;

unsigned long tempoAnterior = 0;

// ======================================================
// BUSCAR DATA/HORA
// ======================================================
String obterDataHoraServidor() {

  if (WiFi.status() != WL_CONNECTED) {
    return "Sem WiFi";
  }

  WiFiClient client;
  HTTPClient http;

  client.setTimeout(2000);

  String url = "http://" + servidor + ":5000/hora";

  Serial.println("[HORA] " + url);

  if (!http.begin(client, url)) {
    return "Erro begin";
  }

  http.setTimeout(2000);

  int httpCode = http.GET();

  if (httpCode == 200) {

    String payload = http.getString();

    DynamicJsonDocument doc(256);

    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      http.end();
      return "JSON erro";
    }

    String dataHora = doc["datahora"].as<String>();

    http.end();

    return dataHora;
  }

  Serial.printf("[HORA] ERRO HTTP: %d\n", httpCode);

  http.end();

  return "Servidor offline";
}

// ======================================================
// ENVIAR DADOS PARA API
// ======================================================
void enviarDadosParaAPI(float temperatura, float umidade) {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[API] WiFi desconectado");
    return;
  }

  String estado;

  if (isnan(temperatura)) {
    estado = "FALHA_SENSOR";
  }
  else if (temperatura >= 40) {
    estado = "CRITICO";
  }
  else if (temperatura >= 30) {
    estado = "ALERTA";
  }
  else {
    estado = "NORMAL";
  }

  DynamicJsonDocument doc(256);

  doc["temperatura"] = isnan(temperatura) ? 0.0 : temperatura;
  doc["umidade"] = isnan(umidade) ? 0.0 : umidade;
  doc["rotacao"] = 0;
  doc["estado"] = estado;

  String jsonBody;

  serializeJson(doc, jsonBody);

  WiFiClient client;
  HTTPClient http;

  client.setTimeout(3000);

  String url = "http://" + servidor + ":5000/receive_data";

  if (!http.begin(client, url)) {
    Serial.println("[API] Erro begin()");
    return;
  }

  http.setTimeout(3000);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", AUTH_TOKEN);

  Serial.println("[API] Enviando...");
  Serial.println(jsonBody);

  int httpCode = http.POST(jsonBody);

  if (httpCode > 0) {

    String resposta = http.getString();

    Serial.printf("[API] HTTP %d\n", httpCode);
    Serial.println(resposta);
  }
  else {

    Serial.printf(
      "[API] ERRO %d -> %s\n",
      httpCode,
      http.errorToString(httpCode).c_str()
    );
  }

  http.end();
}

// ======================================================
// PÁGINA WEB
// ======================================================
void paginaInicial() {

  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  String dataHora = obterDataHoraServidor();

  String tempStr =
    isnan(temperatura)
    ? "--"
    : String(temperatura, 1);

  String humStr =
    isnan(umidade)
    ? "--"
    : String((int)umidade);

  String status;

  if (isnan(temperatura)) {
    status = "ERRO";
  }
  else if (temperatura >= 40) {
    status = "CRITICO";
  }
  else if (temperatura >= 30) {
    status = "ALERTA";
  }
  else {
    status = "NORMAL";
  }

  String html = R"====(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta http-equiv="refresh" content="10">
<meta name="viewport" content="width=device-width, initial-scale=1">

<title>CyberProject IoT</title>

<style>

body{
  background:#0b0f19;
  color:#00ff9d;
  font-family:Arial;
  text-align:center;
  padding:20px;
}

.card{
  background:#111827;
  border:1px solid #00ff9d;
  border-radius:20px;
  padding:25px;
  margin:auto;
  max-width:500px;
  box-shadow:0 0 20px rgba(0,255,157,.2);
}

h1{
  color:#00c8ff;
}

.valor{
  font-size:45px;
  margin:20px 0;
}

.info{
  margin-top:20px;
  color:#ccc;
}

.status{
  margin-top:20px;
  font-size:25px;
  font-weight:bold;
}

.ok{
  color:#00ff9d;
}

.alerta{
  color:orange;
}

.critico{
  color:red;
}

</style>
</head>

<body>

<div class="card">

<h1>CYBERPROJECT IOT</h1>

<div class="valor">
🌡 Temperatura: TEMP °C
</div>

<div class="valor">
💧 Umidade: HUM %
</div>

<div class="status CLASSSTATUS">
STATUS
</div>

<div class="info">
📅 DATAHORA
</div>

<div class="info">
🌐 IP ESP: IPESP
</div>

<div class="info">
🖥 API: SERVIDOR:5000
</div>

</div>

</body>
</html>
)====";

  html.replace("TEMP", tempStr);
  html.replace("HUM", humStr);
  html.replace("DATAHORA", dataHora);
  html.replace("IPESP", WiFi.localIP().toString());
  html.replace("SERVIDOR", servidor);

  if (status == "NORMAL") {
    html.replace("STATUS", "✅ SISTEMA NORMAL");
    html.replace("CLASSSTATUS", "ok");
  }
  else if (status == "ALERTA") {
    html.replace("STATUS", "⚠ ALERTA");
    html.replace("CLASSSTATUS", "alerta");
  }
  else {
    html.replace("STATUS", "🚨 CRITICO");
    html.replace("CLASSSTATUS", "critico");
  }

  server.send(200, "text/html", html);
}

// ======================================================
// SETUP
// ======================================================
void setup() {

  Serial.begin(115200);

  pinMode(BOTAO1, INPUT_PULLUP);
  pinMode(BOTAO2, INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);

  dht.begin();

  Serial.println("");
  Serial.println("================================");
  Serial.println("CYBERPROJECT INICIADO");
  Serial.println("PRESSIONE D0 PARA CONECTAR WIFI");
  Serial.println("================================");
}

// ======================================================
// LOOP
// ======================================================
void loop() {

  // ======================================================
  // BOTÃO CONECTAR WIFI
  // ======================================================
  if (digitalRead(BOTAO1) == LOW && !wifiAtivo) {

    Serial.println("");
    Serial.println("Conectando WiFi...");

    WiFi.begin(ssid, password);

    unsigned long inicio = millis();

    bool conectado = false;

    while (millis() - inicio < 15000) {

      yield();

      if (WiFi.status() == WL_CONNECTED) {
        conectado = true;
        break;
      }

      delay(500);

      Serial.print(".");
    }

    Serial.println("");

    if (!conectado) {

      Serial.println("ERRO AO CONECTAR");

      WiFi.disconnect();

      return;
    }

    Serial.println("WIFI CONECTADO");
    Serial.print("IP ESP: ");
    Serial.println(WiFi.localIP());

    server.on("/", paginaInicial);

    server.begin();

    Serial.println("SERVIDOR WEB ONLINE");
    Serial.print("ACESSE: http://");
    Serial.println(WiFi.localIP());

    wifiAtivo = true;

    delay(1000);
  }

  // ======================================================
  // BOTÃO DESCONECTAR
  // ======================================================
  if (digitalRead(BOTAO2) == LOW && wifiAtivo) {

    Serial.println("Desconectando WiFi...");

    WiFi.disconnect();

    server.stop();

    wifiAtivo = false;

    Serial.println("WiFi desconectado");

    delay(1000);
  }

  // ======================================================
  // ENVIO PARA API
  // ======================================================
  if (millis() - tempoAnterior > 20000) {

    float temperatura = dht.readTemperature();
    float umidade = dht.readHumidity();

    Serial.println("");
    Serial.println("======================");
    Serial.print("TEMP: ");
    Serial.println(temperatura);

    Serial.print("UMID: ");
    Serial.println(umidade);

    Serial.println("======================");

    if (!isnan(temperatura) && temperatura < 27) {

      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
    }

    if (wifiAtivo) {
      enviarDadosParaAPI(temperatura, umidade);
    }

    tempoAnterior = millis();
  }

  // ======================================================
  // WEB SERVER
  // ======================================================
  if (wifiAtivo) {
    server.handleClient();
  }
}