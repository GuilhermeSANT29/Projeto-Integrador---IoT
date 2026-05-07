# 04 - Etapa 2: Desenvolvimento da Coleta de Dados e Comunicação TCP/IP

**Grupo Alpha** | **Data:** Maio de 2026

## Objetivo da Etapa
Implementar a leitura dos sensores do Shield HY-M302, estruturar os dados e preparar a comunicação via Wi-Fi (TCP/IP).

## Componentes Utilizados (Shield HY-M302)

| Componente          | Pino     | Função                          | Status     |
|---------------------|----------|---------------------------------|------------|
| DHT11               | D4       | Temperatura e Umidade           | Implementado |
| Buzzer              | D5       | Alerta sonoro                   | Implementado |
| Receptor IR         | D6       | Controle remoto                 | Implementado |
| LED RGB             | D4 / PWM | Indicação visual de status      | Implementado |
| Potenciômetro       | A0       | Leitura de rotação              | Implementado |

## Código Principal - Coleta de Dados (`coleta_dados.ino`)

```cpp```

#include <DHT.h>
#include <ArduinoJson.h>

#define DHTPIN D4
#define DHTTYPE DHT11
#define BUZZER D5
#define IR_PIN D6
#define POT_PIN A0

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(BUZZER, OUTPUT);
  pinMode(IR_PIN, INPUT);
  
  Serial.println("\n=== Sistema de Monitoramento IoT - Etapa 2 ===");
}

void loop() {
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  int rotacao = analogRead(POT_PIN);
  int irValue = digitalRead(IR_PIN);

  // Controle simples do Buzzer
  if (temperatura > 30) {
    digitalWrite(BUZZER, HIGH);
  } else {
    digitalWrite(BUZZER, LOW);
  }

  // Estruturação dos dados em JSON
  StaticJsonDocument<200> doc;
  doc["temperatura"] = temperatura;
  doc["umidade"] = umidade;
  doc["rotacao"] = map(rotacao, 0, 1023, 0, 100); // Converte para %
  doc["ir_detectado"] = (irValue == LOW);
  doc["timestamp"] = millis();

  String jsonString;
  serializeJson(doc, jsonString);

  Serial.println("Dados Coletados:");
  Serial.println(jsonString);
  Serial.println("------------------------");

  delay(2000);
}
