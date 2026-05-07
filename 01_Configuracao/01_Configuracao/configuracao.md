# 01 - Configuração do Ambiente de Desenvolvimento

## Objetivo
Configurar corretamente a Arduino IDE para programar o microcontrolador **ESP8266 D1** (LOLIN Wemos D1 R2 & mini) e validar a comunicação com a placa.

## Passos Executados

1. Acessado **Arquivo → Preferências** e adicionada a URL oficial:
http://arduino.esp8266.com/stable/package_esp8266com_index.json

2. Acessado **Ferramentas → Placa → Gerenciador de Placas** e instalado o pacote **ESP8266** (versão mais recente).

3. Selecionada a placa correta: **LOLIN(WEMOS) D1 R2 & mini**

4. Selecionada a porta COM correspondente ao dispositivo.

## Validação Prática
- Código **Blink** carregado com sucesso.
- LED onboard (D4) piscando corretamente.
- Comunicação serial via Monitor Serial funcionando.

**Resultado:** Ambiente totalmente configurado e operacional.

## Evidências

- `prints/preferencias_url.png`
- `prints/gerenciador_placas.png`
- `prints/placa_selecionada.png`
- `prints/upload_blink_sucesso.png`
- `prints/serial_monitor.png`

**Status:** Concluído com sucesso ✅
