- Etapa 1 
Disciplina: 
Grupo
Data: Maio de 2026

Objetivo
Configure corretamente o ambiente de desenvolvimento no Arduino IDE para permitir a programação do microcontrolador ESP8266 D1 (LOLIN Wemos D1 R2 & mini) e validar a comunicação USB/serial.

Passos Realizados
1. Adição da URL do ESP8266
Acessamos Arquivo → Preferências
No campo "URLs Adicionais de Gerenciadores de Placas" adicionamos a seguinte URL:
text  http://arduino.esp8266.com/stable/package_esp8266com_index.json

C++ Arduino / Blink Test - ESP8266 Wemos D1 #define LED_BUILTIN D4   // GPIO2 - LED onboard

void setup() { pinMode(LED_BUILTIN, OUTPUT); Serial.begin(115200); Serial.println("\n=== Teste de Configuração - ESP8266 ==="); Serial.println("LED Builtin (D4) piscando..."); }

loop vazio() {digitalWrite(LED_BUILTIN, LOW); // Liga o LED (ativo em LOW) Serial.println("LED Ligado"); atraso(800);

digitalWrite(LED_BUILTIN, ALTO); // Desliga o LED Serial.println("LED Desligado"); atraso(800); }
