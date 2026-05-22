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
// CORREÇÃO 1: IP DO SERVIDOR
// Execute "ipconfig" no Windows e atualize o IP abaixo
// caso tenha mudado. Procure por "Endereço IPv4".
// ======================================================
String servidor = "10.106.202.6";

// ======================================================
// CORREÇÃO 2: TOKEN DE AUTENTICAÇÃO
// Deve ser IDÊNTICO ao AUTH_TOKEN da API Python
// ======================================================
const String AUTH_TOKEN = "CyberProject";

// ======================================================
// NOME SENSOR
// ======================================================
String nomeSensor = "Temperatura Sala";

// ======================================================
// SERVIDOR WEB LOCAL DO ESP
// ======================================================
ESP8266WebServer server(80);

// ======================================================
// VARIÁVEIS DE CONTROLE
// ======================================================
bool wifiAtivo       = false;
unsigned long tempoAnterior = 0;

// ======================================================
// FUNÇÃO: BUSCAR DATA/HORA DA API
// CORREÇÃO 3: Rota /hora agora existe na API corrigida
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
      // MELHORIA: exibe o erro real no monitor serial
      Serial.printf("[HORA] Erro HTTP: %d — %s\n", httpCode, http.errorToString(httpCode).c_str());
      dataHora = "Erro servidor";
    }

    http.end();
  }

  return dataHora;
}

// ======================================================
// ENVIO DOS DADOS DO SENSOR PARA A API
// ======================================================
void enviarDadosParaAPI(float temperatura, float umidade) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[API] WiFi desconectado. Envio cancelado.");
    return;
  }

  // Determina o estado com base na temperatura
  String estado;
  if (isnan(temperatura)) {
    estado = "FALHA_SENSOR";
  } else if (temperatura >= 40.0) {
    estado = "CRITICO";
  } else if (temperatura >= 30.0) {
    estado = "ALERTA";
  } else {
    estado = "NORMAL";
  }

  // Monta o JSON
  DynamicJsonDocument doc(256);
  doc["temperatura"] = isnan(temperatura) ? 0.0 : temperatura;
  doc["umidade"]     = isnan(umidade)     ? 0.0 : umidade;
  doc["rotacao"]     = 0;       // Ajuste se tiver sensor de rotação
  doc["estado"]      = estado;

  String jsonBody;
  serializeJson(doc, jsonBody);

  WiFiClient client;
  HTTPClient http;

  String url = "http://" + servidor + ":5000/receive_data";
  http.begin(client, url);

  // CORREÇÃO 5: Adiciona os headers obrigatórios
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", AUTH_TOKEN);

  Serial.println("[API] Enviando dados para: " + url);
  Serial.println("[API] Payload: " + jsonBody);

  int httpCode = http.POST(jsonBody);

  if (httpCode > 0) {
    String resposta = http.getString();
    Serial.printf("[API] Resposta HTTP: %d\n", httpCode);
    Serial.println("[API] Corpo: " + resposta);
  } else {
    // MELHORIA: exibe o erro real (resolve o "HTTP -1")
    Serial.printf("[API] ERRO HTTP: %d — %s\n", httpCode, http.errorToString(httpCode).c_str());
  }

  http.end();
}

// ======================================================
// PÁGINA WEB — DESIGN CYBERSISTEMAS (sem alterações)
// ======================================================
void paginaInicial() {
  float temperatura = dht.readTemperature();
  float umidade     = dht.readHumidity();
  String dataHora   = obterDataHoraServidor();

  String status      = isnan(temperatura) ? "erro" : (temperatura >= 40.0 ? "critico" : (temperatura >= 30.0 ? "alerta" : "normal"));
  String tempStr     = isnan(temperatura) ? "--" : String(temperatura, 1);
  String humStr      = isnan(umidade)     ? "--" : String((int)umidade);
  String statusColor = (status == "critico") ? "#ff003c" : (status == "alerta") ? "#ff9500" : "#00ff41";
  String statusText  = (status == "critico") ? "CR&Iacute;TICO" : (status == "alerta") ? "ALERTA" : "NORMAL";
  String ipStr       = WiFi.localIP().toString();

  String html = "<!DOCTYPE html>";
  html += "<html lang='pt-br'>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<title>Cybersistemas IOT Dashboard</title>";
  html += "<link href='https://fonts.googleapis.com/css2?family=Fira+Code:wght@400;500;600;700&display=swap' rel='stylesheet'>";

  html += "<style>";
  html += ":root{";
  html += "  --neon-blue:#00f2ff;";
  html += "  --neon-green:#00ff41;";
  html += "  --neon-red:#ff003c;";
  html += "  --neon-orange:#ff9500;";
  html += "  --bg-dark:#0a0a0f;";
  html += "  --bg-card:rgba(20,20,30,0.98);";
  html += "  --glow-green:0 0 15px rgba(0,255,65,0.4);";
  html += "  --glow-blue:0 0 15px rgba(0,242,255,0.4);";
  html += "}";
  html += "body{font-family:'Fira Code',monospace;background:linear-gradient(135deg,#0a0a0f 0%,#1a1a2e 50%,#0a0a0f 100%);color:#e5e5e5;min-height:100vh;margin:0;padding:0;}";
  html += ".header-main{background:linear-gradient(90deg,rgba(0,255,65,0.1) 0%,transparent 50%,rgba(0,242,255,0.1) 100%);border-bottom:1px solid rgba(0,255,65,0.3);text-align:center;padding:3rem 1.5rem 4rem;}";
  html += ".header-title{font-size:clamp(2rem,8vw,5rem);font-weight:700;background:linear-gradient(90deg,var(--neon-blue),var(--neon-green));-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text;margin:0 0 1rem;letter-spacing:-0.02em;}";
  html += ".header-sub{font-size:clamp(0.8rem,2vw,1.1rem);opacity:0.85;max-width:600px;margin:0 auto 2rem;line-height:1.6;}";
  html += ".live-dot{display:inline-block;width:12px;height:12px;background:var(--neon-green);border-radius:50%;margin-right:8px;animation:ping 1.5s cubic-bezier(0,0,0.2,1) infinite;vertical-align:middle;}";
  html += "@keyframes ping{0%{transform:scale(1);opacity:1;}75%,100%{transform:scale(2.2);opacity:0;}}";
  html += ".status-bar{display:inline-flex;align-items:center;gap:0.5rem;font-size:0.9rem;opacity:0.9;text-transform:uppercase;letter-spacing:0.05em;}";
  html += "main{max-width:900px;margin:0 auto;padding:3rem 1.5rem 4rem;}";
  html += ".section-title{font-size:clamp(1.4rem,4vw,2.2rem);font-weight:700;text-align:center;margin-bottom:2.5rem;background:linear-gradient(90deg,var(--neon-blue),var(--neon-green));-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text;letter-spacing:-0.01em;}";
  html += ".cards-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:2rem;margin-bottom:2rem;}";
  html += ".sensor-card{background:var(--bg-card);border:2px solid rgba(0,255,65,0.25);border-radius:24px;padding:2.5rem 2rem;text-align:center;position:relative;overflow:hidden;transition:all 0.4s cubic-bezier(0.25,0.8,0.25,1);}";
  html += ".sensor-card::before{content:'';position:absolute;top:0;left:-100%;width:100%;height:100%;background:linear-gradient(90deg,transparent,rgba(0,255,65,0.05),transparent);transition:left 0.6s;}";
  html += ".sensor-card:hover::before{left:100%;}";
  html += ".sensor-card:hover{border-color:var(--neon-green);transform:translateY(-8px);box-shadow:var(--glow-green),0 25px 50px rgba(0,0,0,0.5);}";
  html += ".sensor-badge{position:absolute;top:1.2rem;right:1.2rem;padding:0.35rem 0.85rem;background:linear-gradient(45deg,var(--neon-blue),rgba(0,242,255,0.8));color:#000;font-size:0.7rem;font-weight:700;border-radius:20px;box-shadow:0 4px 15px rgba(0,242,255,0.3);}";
  html += ".card-label{font-size:1rem;font-weight:600;text-transform:uppercase;letter-spacing:0.08em;opacity:0.85;margin-bottom:0.5rem;}";
  html += ".card-sub{font-size:0.75rem;opacity:0.6;letter-spacing:0.04em;margin-bottom:1.2rem;}";
  html += ".value-main{font-size:clamp(3.5rem,12vw,6rem);font-weight:700;line-height:1;color:var(--neon-blue);text-shadow:var(--glow-blue);}";
  html += ".status-card{background:var(--bg-card);border-radius:24px;padding:3rem 2rem;text-align:center;position:relative;overflow:hidden;margin-bottom:2rem;transition:border-color 0.4s;}";
  html += ".status-val{font-size:clamp(2rem,6vw,3.5rem);font-weight:900;letter-spacing:0.04em;margin-bottom:0.75rem;}";
  html += ".status-msg{font-size:1rem;opacity:0.8;line-height:1.6;}";
  html += ".info-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:1.25rem;margin-top:2.5rem;}";
  html += ".info-item{background:rgba(0,255,65,0.06);border:1px solid rgba(0,255,65,0.2);border-radius:16px;padding:1.25rem 1rem;text-align:center;}";
  html += ".info-label{font-size:0.7rem;text-transform:uppercase;letter-spacing:0.08em;opacity:0.6;margin-bottom:0.5rem;}";
  html += ".info-val{font-size:1.1rem;font-weight:700;color:var(--neon-green);}";
  html += ".info-val.blue{color:var(--neon-blue);}";
  html += ".info-val.orange{color:var(--neon-orange);}";
  html += "footer{text-align:center;padding:2rem 1.5rem;border-top:1px solid rgba(0,255,65,0.15);font-size:0.8rem;opacity:0.5;}";
  html += "</style>";
  html += "</head><body>";

  html += "<header class='header-main'>";
  html += "  <h1 class='header-title'>CYBERSISTEMAS IOT</h1>";
  html += "  <p class='header-sub'>Dashboard Profissional &bull; Wemos D1 Mini &bull; DHT11 &bull; Python Flask API &bull; <span style='color:var(--neon-green);font-weight:600;'>Google Sheets Integrado</span></p>";
  html += "  <div class='status-bar'><span class='live-dot'></span>Sensor ativo &bull; Atualiza a cada 5s</div>";
  html += "</header>";

  html += "<main>";
  html += "  <h2 class='section-title'>Sensores em Tempo Real</h2>";
  html += "  <div class='cards-grid'>";

  html += "    <div class='sensor-card'>";
  html += "      <div class='sensor-badge'>TEMP-01</div>";
  html += "      <div class='card-label'>Temperatura</div>";
  html += "      <div class='card-sub'>DHT11 &bull; Wemos &bull; " + nomeSensor + "</div>";
  html += "      <div class='value-main'>" + tempStr + "&deg;C</div>";
  html += "    </div>";

  html += "    <div class='sensor-card'>";
  html += "      <div class='sensor-badge'>HUM-01</div>";
  html += "      <div class='card-label'>Umidade</div>";
  html += "      <div class='card-sub'>DHT11 &bull; Ambient Dynamic</div>";
  html += "      <div class='value-main'>" + humStr + "%</div>";
  html += "    </div>";
  html += "  </div>";

  html += "  <div class='status-card' style='border:2px solid " + statusColor + ";'>";
  html += "    <div class='sensor-badge' style='background:linear-gradient(45deg," + statusColor + ",rgba(255,255,255,0.5));color:#000;'>" + statusText + "</div>";
  html += "    <div class='status-val' style='color:" + statusColor + ";'>";
  if (status == "critico")     html += "&#9888; TEMPERATURA CR&Iacute;TICA";
  else if (status == "alerta") html += "&#9889; TEMPERATURA ELEVADA";
  else                         html += "&#10003; SISTEMA OK";
  html += "    </div>";
  html += "    <div class='status-msg'>";
  if (status == "critico")     html += "Temperatura acima de 40&deg;C &mdash; Interven&ccedil;&atilde;o necess&aacute;ria imediata";
  else if (status == "alerta") html += "Temperatura entre 30&deg;C e 40&deg;C &mdash; Aten&ccedil;&atilde;o recomendada";
  else                         html += "Monitora&ccedil;&atilde;o ativa &bull; Wemos conectado &bull; Todos os sensores nominais";
  html += "    </div>";

  html += "    <div class='info-grid'>";
  html += "      <div class='info-item'><div class='info-label'>API Server</div><div class='info-val blue'>" + servidor + ":5000</div></div>";
  html += "      <div class='info-item'><div class='info-label'>Database</div><div class='info-val blue'>MySQL</div></div>";
  html += "      <div class='info-item'><div class='info-label'>Sheets Sync</div><div class='info-val orange'>LIVE</div></div>";
  html += "      <div class='info-item'><div class='info-label'>Data / Hora</div><div class='info-val'>" + dataHora + "</div></div>";
  html += "      <div class='info-item'><div class='info-label'>IP do ESP</div><div class='info-val blue'>" + ipStr + "</div></div>";
  html += "      <div class='info-item'><div class='info-label'>WiFi Status</div><div class='info-val'>" + String(wifiAtivo ? "Conectado" : "Desconectado") + "</div></div>";
  html += "    </div>";
  html += "  </div>";
  html += "</main>";
  html += "<footer>CYBERSISTEMAS &copy; 2026 &bull; Wemos D1 Mini + DHT11 + Python API + MySQL + Google Sheets</footer>";
  html += "</body></html>";

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
  Serial.println("SISTEMA INICIADO");
  Serial.println("Pressione BOTAO1 para conectar WiFi");
  Serial.println("========================");
}

// ======================================================
// LOOP
// ======================================================
void loop() {

  // ======================================================
  // LEITURA E ENVIO A CADA 20 SEGUNDOS
  // CORREÇÃO 4: agora chama enviarDadosParaAPI()
  // ======================================================
  if (millis() - tempoAnterior > 20000) {

    float temperatura = dht.readTemperature();
    float umidade     = dht.readHumidity();
    String dataHora   = obterDataHoraServidor();

    Serial.println("========================");
    Serial.print("SENSOR: ");       Serial.println(nomeSensor);
    Serial.print("TEMPERATURA: ");  Serial.print(temperatura);  Serial.println(" °C");
    Serial.print("UMIDADE: ");      Serial.print(umidade);      Serial.println(" %");
    Serial.print("DATA/HORA: ");    Serial.println(dataHora);
    Serial.println("========================");

    // Alerta sonoro se temperatura baixa
    if (!isnan(temperatura) && temperatura < 27) {
      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
    }

    // Envia os dados para a API (correção principal)
    if (wifiAtivo) {
      enviarDadosParaAPI(temperatura, umidade);
    }

    tempoAnterior = millis();
  }

  // ======================================================
  // BOTÃO 1 → CONECTAR WIFI
  // CORREÇÃO 6: timeout de 15 segundos no while de conexão
  // ======================================================
  if (digitalRead(BOTAO1) == LOW && !wifiAtivo) {

    Serial.println("========================");
    Serial.println("Conectando WiFi...");

    WiFi.begin(ssid, password);

    unsigned long inicio = millis();
    bool conectou = false;

    while (millis() - inicio < 15000) {   // timeout: 15 segundos
      if (WiFi.status() == WL_CONNECTED) {
        conectou = true;
        break;
      }
      delay(500);
      Serial.print(".");
    }

    Serial.println("");

    if (!conectou) {
      Serial.println("ERRO: WiFi nao conectou em 15s. Verifique a rede.");
      Serial.println("========================");
      WiFi.disconnect();
      delay(1000);
      return;   // sai sem travar
    }

    Serial.println("WiFi conectado!");
    Serial.print("IP do ESP: ");
    Serial.println(WiFi.localIP());

    server.on("/", paginaInicial);
    server.begin();

    Serial.println("Servidor WEB iniciado!");
    Serial.print("Acesse: http://");
    Serial.println(WiFi.localIP());
    Serial.println("========================");

    wifiAtivo = true;
    delay(1000);
  }

  // ======================================================
  // BOTÃO 2 → DESCONECTAR WIFI
  // ======================================================
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

  // ======================================================
  // SERVIDOR WEB LOCAL
  // ======================================================
  if (wifiAtivo) {
    server.handleClient();
  }
}
