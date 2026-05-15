// ======================================================
// CYBERSISTEMAS IoT - Wemos D1 Mini
// Design integrado do Dashboard Cybersistemas v3.0
// DHT11 → Página web com tema neon/cyber
// ======================================================

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ======================================================
// BOTÕES
// ======================================================
#define BOTAO1 D0   // Conectar WiFi
#define BOTAO2 D1   // Desconectar WiFi

// ======================================================
// BUZZER
// ======================================================
#define BUZZER D5

// ======================================================
// SENSOR DHT11
// ======================================================
#define DHTPIN D2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// ======================================================
// WIFI
// ======================================================
const char* ssid     = "Cyber-Projeto";
const char* password = "Senai@122";

// ======================================================
// IP SERVIDOR PYTHON
// ======================================================
String servidor = "10.106.202.43";

// ======================================================
// NOME SENSOR
// ======================================================
String nomeSensor = "Temperatura Sala";

// ======================================================
// SERVIDOR WEB
// ======================================================
ESP8266WebServer server(80);

// ======================================================
// VARIÁVEIS
// ======================================================
bool wifiAtivo = false;
unsigned long tempoAnterior = 0;

// ======================================================
// PEGAR DATA/HORA DO SERVIDOR PYTHON
// ======================================================
String obterDataHoraServidor() {
  String dataHora = "Sem horario";
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    String url = "http://" + servidor + ":5000/hora";
    http.begin(client, url);
    int httpCode = http.GET();
    if (httpCode == 200) {
      String payload = http.getString();
      DynamicJsonDocument doc(256);
      deserializeJson(doc, payload);
      dataHora = doc["datahora"].as<String>();
    } else {
      dataHora = "Erro servidor";
    }
    http.end();
  }
  return dataHora;
}

// ======================================================
// DETERMINAR STATUS DA TEMPERATURA
// ======================================================
String determinarStatus(float temp) {
  if (temp >= 40.0) return "critico";
  if (temp >= 30.0) return "alerta";
  return "normal";
}

// ======================================================
// PÁGINA WEB — DESIGN CYBERSISTEMAS v3.0
// ======================================================
void paginaInicial() {
  float temperatura   = dht.readTemperature();
  float umidade       = dht.readHumidity();
  String dataHora     = obterDataHoraServidor();
  String status       = isnan(temperatura) ? "erro" : determinarStatus(temperatura);
  String tempStr      = isnan(temperatura) ? "--" : String(temperatura, 1);
  String humStr       = isnan(umidade)     ? "--" : String((int)umidade);
  String statusColor  = (status == "critico") ? "#ff003c" : (status == "alerta") ? "#ff9500" : "#00ff41";
  String statusText   = (status == "critico") ? "CRÍTICO" : (status == "alerta") ? "ALERTA" : "NORMAL";
  String statusBorder = (status == "critico") ? "#ff003c" : (status == "alerta") ? "#ff9500" : "#00ff41";
  String ipStr        = WiFi.localIP().toString();

  String html = F("<!DOCTYPE html>"
    "<html lang='pt-br'>"
    "<head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<meta http-equiv='refresh' content='5'>"
    "<title>Cybersistemas IOT Dashboard</title>"
    "<link href='https://fonts.googleapis.com/css2?family=Fira+Code:wght@400;500;600;700&display=swap' rel='stylesheet'>"
    "<style>"
    ":root{"
      "--neon-blue:#00f2ff;"
      "--neon-green:#00ff41;"
      "--neon-red:#ff003c;"
      "--neon-orange:#ff9500;"
      "--bg-dark:#0a0a0f;"
      "--bg-card:rgba(20,20,30,0.98);"
      "--glow-green:0 0 15px rgba(0,255,65,0.4);"
      "--glow-blue:0 0 15px rgba(0,242,255,0.4);"
    "}"
    "body{"
      "font-family:'Fira Code',monospace;"
      "background:linear-gradient(135deg,#0a0a0f 0%,#1a1a2e 50%,#0a0a0f 100%);"
      "color:#e5e5e5;"
      "min-height:100vh;"
      "margin:0;"
      "padding:0;"
    "}"
    // HEADER
    ".header-main{"
      "background:linear-gradient(90deg,rgba(0,255,65,0.1) 0%,transparent 50%,rgba(0,242,255,0.1) 100%);"
      "border-bottom:1px solid rgba(0,255,65,0.3);"
      "text-align:center;"
      "padding:3rem 1.5rem 4rem;"
    "}"
    ".header-title{"
      "font-size:clamp(2rem,8vw,5rem);"
      "font-weight:700;"
      "background:linear-gradient(90deg,var(--neon-blue),var(--neon-green));"
      "-webkit-background-clip:text;"
      "-webkit-text-fill-color:transparent;"
      "background-clip:text;"
      "margin:0 0 1rem;"
      "letter-spacing:-0.02em;"
    "}"
    ".header-sub{"
      "font-size:clamp(0.8rem,2vw,1.1rem);"
      "opacity:0.85;"
      "max-width:600px;"
      "margin:0 auto 2rem;"
      "line-height:1.6;"
    "}"
    ".live-dot{"
      "display:inline-block;"
      "width:12px;height:12px;"
      "background:var(--neon-green);"
      "border-radius:50%;"
      "margin-right:8px;"
      "animation:ping 1.5s cubic-bezier(0,0,0.2,1) infinite;"
      "vertical-align:middle;"
    "}"
    "@keyframes ping{"
      "0%{transform:scale(1);opacity:1;}"
      "75%,100%{transform:scale(2.2);opacity:0;}"
    "}"
    // STATUS BAR
    ".status-bar{"
      "display:inline-flex;"
      "align-items:center;"
      "gap:0.5rem;"
      "font-size:0.9rem;"
      "opacity:0.9;"
      "text-transform:uppercase;"
      "letter-spacing:0.05em;"
    "}"
    // MAIN
    "main{"
      "max-width:900px;"
      "margin:0 auto;"
      "padding:3rem 1.5rem 4rem;"
    "}"
    // SECTION TITLE
    ".section-title{"
      "font-size:clamp(1.4rem,4vw,2.2rem);"
      "font-weight:700;"
      "text-align:center;"
      "margin-bottom:2.5rem;"
      "background:linear-gradient(90deg,var(--neon-blue),var(--neon-green));"
      "-webkit-background-clip:text;"
      "-webkit-text-fill-color:transparent;"
      "background-clip:text;"
      "letter-spacing:-0.01em;"
    "}"
    // SENSOR CARDS GRID
    ".cards-grid{"
      "display:grid;"
      "grid-template-columns:repeat(auto-fit,minmax(280px,1fr));"
      "gap:2rem;"
      "margin-bottom:2rem;"
    "}"
    ".sensor-card{"
      "background:var(--bg-card);"
      "border:2px solid rgba(0,255,65,0.25);"
      "border-radius:24px;"
      "padding:2.5rem 2rem;"
      "text-align:center;"
      "position:relative;"
      "overflow:hidden;"
      "transition:all 0.4s cubic-bezier(0.25,0.8,0.25,1);"
    "}"
    ".sensor-card::before{"
      "content:'';"
      "position:absolute;top:0;left:-100%;width:100%;height:100%;"
      "background:linear-gradient(90deg,transparent,rgba(0,255,65,0.05),transparent);"
      "transition:left 0.6s;"
    "}"
    ".sensor-card:hover::before{left:100%;}"
    ".sensor-card:hover{"
      "border-color:var(--neon-green);"
      "transform:translateY(-8px);"
      "box-shadow:var(--glow-green),0 25px 50px rgba(0,0,0,0.5);"
    "}"
    ".sensor-badge{"
      "position:absolute;top:1.2rem;right:1.2rem;"
      "padding:0.35rem 0.85rem;"
      "background:linear-gradient(45deg,var(--neon-blue),rgba(0,242,255,0.8));"
      "color:#000;font-size:0.7rem;font-weight:700;"
      "border-radius:20px;"
      "box-shadow:0 4px 15px rgba(0,242,255,0.3);"
    "}"
    ".card-label{"
      "font-size:1rem;font-weight:600;"
      "text-transform:uppercase;letter-spacing:0.08em;"
      "opacity:0.85;margin-bottom:0.5rem;"
    "}"
    ".card-sub{"
      "font-size:0.75rem;opacity:0.6;letter-spacing:0.04em;margin-bottom:1.2rem;"
    "}"
    ".value-main{"
      "font-size:clamp(3.5rem,12vw,6rem);"
      "font-weight:700;"
      "line-height:1;"
      "color:var(--neon-blue);"
      "text-shadow:var(--glow-blue);"
    "}"
    // STATUS CARD
    ".status-card{"
      "background:var(--bg-card);"
      "border-radius:24px;"
      "padding:3rem 2rem;"
      "text-align:center;"
      "position:relative;"
      "overflow:hidden;"
      "margin-bottom:2rem;"
      "transition:border-color 0.4s;"
    "}"
    ".status-val{"
      "font-size:clamp(2rem,6vw,3.5rem);"
      "font-weight:900;"
      "letter-spacing:0.04em;"
      "margin-bottom:0.75rem;"
    "}"
    ".status-msg{"
      "font-size:1rem;opacity:0.8;line-height:1.6;"
    "}"
    // INFO GRID
    ".info-grid{"
      "display:grid;"
      "grid-template-columns:repeat(auto-fit,minmax(180px,1fr));"
      "gap:1.25rem;"
      "margin-top:2.5rem;"
    "}"
    ".info-item{"
      "background:rgba(0,255,65,0.06);"
      "border:1px solid rgba(0,255,65,0.2);"
      "border-radius:16px;"
      "padding:1.25rem 1rem;"
      "text-align:center;"
    "}"
    ".info-label{"
      "font-size:0.7rem;text-transform:uppercase;letter-spacing:0.08em;opacity:0.6;margin-bottom:0.5rem;"
    "}"
    ".info-val{"
      "font-size:1.1rem;font-weight:700;color:var(--neon-green);"
    "}"
    ".info-val.blue{color:var(--neon-blue);}"
    ".info-val.orange{color:var(--neon-orange);}"
    // FOOTER
    "footer{"
      "text-align:center;padding:2rem 1.5rem;"
      "border-top:1px solid rgba(0,255,65,0.15);"
      "font-size:0.8rem;opacity:0.5;"
    "}"
    "</style></head>"
    "<body>"
  );

  // HEADER
  html += F("<header class='header-main'>"
    "<h1 class='header-title'>CYBERSISTEMAS IOT</h1>"
    "<p class='header-sub'>"
      "Dashboard Profissional &bull; Wemos D1 Mini &bull; DHT11 &bull; Python Flask API &bull; "
      "<span style='color:var(--neon-green);font-weight:600;'>Google Sheets Integrado</span>"
    "</p>"
    "<div class='status-bar'>"
      "<span class='live-dot'></span>"
      "Sensor ativo &bull; Atualiza a cada 5s"
    "</div>"
  "</header>");

  // MAIN
  html += F("<main>");
  html += F("<h2 class='section-title'>Sensores em Tempo Real</h2>");

  // CARDS: TEMPERATURA + UMIDADE
  html += F("<div class='cards-grid'>");

  // Temperatura
  html += "<div class='sensor-card'>";
  html += "<div class='sensor-badge'>TEMP-01</div>";
  html += "<div class='card-label'>Temperatura</div>";
  html += "<div class='card-sub'>DHT11 &bull; Wemos D1 Mini &bull; ";
  html += nomeSensor;
  html += "</div>";
  html += "<div class='value-main'>";
  html += tempStr;
  html += "&deg;C</div>";
  html += "</div>";

  // Umidade
  html += "<div class='sensor-card'>";
  html += "<div class='sensor-badge'>HUM-01</div>";
  html += "<div class='card-label'>Umidade</div>";
  html += "<div class='card-sub'>DHT11 &bull; Wemos D1 Mini</div>";
  html += "<div class='value-main'>";
  html += humStr;
  html += "%</div>";
  html += "</div>";

  html += F("</div>"); // /cards-grid

  // STATUS CARD
  html += "<div class='status-card' style='border:2px solid ";
  html += statusBorder;
  html += ";'>";
  html += "<div class='sensor-badge' style='background:linear-gradient(45deg,";
  html += statusColor;
  html += ",rgba(255,255,255,0.5));color:#000;'>";
  html += statusText;
  html += "</div>";
  html += "<div class='status-val' style='color:";
  html += statusColor;
  html += ";'>";
  if (status == "critico") {
    html += "⚠ TEMPERATURA CRÍTICA";
  } else if (status == "alerta") {
    html += "⚡ TEMPERATURA ELEVADA";
  } else {
    html += "✓ SISTEMA OK";
  }
  html += "</div>";
  html += "<div class='status-msg'>";
  if (status == "critico") {
    html += "Temperatura acima de 40°C &mdash; Intervenção necessária imediata";
  } else if (status == "alerta") {
    html += "Temperatura entre 30°C e 40°C &mdash; Atenção recomendada";
  } else {
    html += "Monitoração ativa &bull; Wemos conectado &bull; Todos os sensores nominais";
  }
  html += "</div>";

  // INFO GRID dentro do status
  html += F("<div class='info-grid'>"
    "<div class='info-item'>"
      "<div class='info-label'>API Server</div>"
      "<div class='info-val blue'>10.106.202.20:5000</div>"
    "</div>"
    "<div class='info-item'>"
      "<div class='info-label'>Database</div>"
      "<div class='info-val blue'>SQL SERVER</div>"
    "</div>"
    "<div class='info-item'>"
      "<div class='info-label'>Sheets Sync</div>"
      "<div class='info-val orange'>LIVE</div>"
    "</div>"
  );

  html += "<div class='info-item'><div class='info-label'>Data / Hora</div><div class='info-val'>";
  html += dataHora;
  html += "</div></div>";

  html += "<div class='info-item'><div class='info-label'>IP do ESP</div><div class='info-val blue'>";
  html += ipStr;
  html += "</div></div>";

  html += "<div class='info-item'><div class='info-label'>WiFi Status</div><div class='info-val'>";
  html += wifiAtivo ? "Conectado" : "Desconectado";
  html += "</div></div>";

  html += F("</div>"); // /info-grid
  html += F("</div>"); // /status-card

  html += F("</main>");

  // FOOTER
  html += F("<footer>CYBERSISTEMAS &copy; 2025 &bull; Wemos D1 Mini + DHT11 + Python API + SQL Server + Google Sheets</footer>");

  html += F("</body></html>");

  server.send(200, "text/html; charset=UTF-8", html);
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

  Serial.println("========================");
  Serial.println("CYBERSISTEMAS IOT");
  Serial.println("SISTEMA INICIADO");
  Serial.println("Pressione BOTAO1 para conectar WiFi");
  Serial.println("========================");
}

// ======================================================
// LOOP
// ======================================================
void loop() {

  // LEITURA SENSOR a cada 2s
  if (millis() - tempoAnterior > 2000) {
    float temperatura = dht.readTemperature();
    float umidade     = dht.readHumidity();
    String dataHora   = obterDataHoraServidor();

    if (!isnan(temperatura)) {
      Serial.println("========================");
      Serial.print("SENSOR: ");
      Serial.println(nomeSensor);
      Serial.print("TEMPERATURA: ");
      Serial.print(temperatura);
      Serial.println(" °C");
      Serial.print("UMIDADE: ");
      Serial.print(umidade);
      Serial.println(" %");
      Serial.print("DATA/HORA: ");
      Serial.println(dataHora);
      Serial.print("STATUS: ");
      Serial.println(determinarStatus(temperatura));
      Serial.println("========================");

      // ALERTA SONORO se temperatura crítica (>= 40°C)
      if (temperatura >= 40.0) {
        tone(BUZZER, 1000, 500);
        delay(600);
        tone(BUZZER, 1500, 300);
      // BEEP simples se temperatura normal (< 27°C) — comportamento original
      } else if (temperatura < 27.0) {
        digitalWrite(BUZZER, HIGH);
        delay(200);
        digitalWrite(BUZZER, LOW);
      }
    }

    tempoAnterior = millis();
  }

  // BOTÃO 1 → CONECTAR WIFI
  if (digitalRead(BOTAO1) == LOW && !wifiAtivo) {
    Serial.println("========================");
    Serial.println("Conectando WiFi...");

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi conectado!");
    Serial.print("IP do ESP: ");
    Serial.println(WiFi.localIP());

    server.on("/", paginaInicial);
    server.begin();

    Serial.println("========================");
    Serial.println("Servidor WEB iniciado!");
    Serial.print("Acesse: http://");
    Serial.println(WiFi.localIP());
    Serial.println("========================");

    wifiAtivo = true;
    delay(1000);
  }

  // BOTÃO 2 → DESCONECTAR WIFI
  if (digitalRead(BOTAO2) == LOW && wifiAtivo) {
    Serial.println("========================");
    Serial.println("Desconectando WiFi...");
    WiFi.disconnect();
    server.stop();
    wifiAtivo = false;
    Serial.println("WiFi desconectado!");
    Serial.println("========================");
    delay(1000);
  }

  // SERVIDOR WEB
  if (wifiAtivo) {
    server.handleClient();
  }
}
