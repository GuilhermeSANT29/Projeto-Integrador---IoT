# 03 - Modelagem do Sistema

## Objetivo
Definir o comportamento completo do sistema antes da implementação do código, garantindo previsibilidade e organização.

## Componentes do Shield HY-M302 Utilizados

| Componente          | Pino     | Tipo de Sinal     | Função Principal |
|---------------------|----------|-------------------|------------------|
| DHT11               | D4       | Digital           | Temperatura e Umidade |
| Buzzer              | D5       | Digital (Saída)   | Alerta sonoro |
| Receptor IR         | D6       | Digital (Entrada) | Controle remoto |
| LED RGB             | D4 / PWM | Saída PWM         | Indicação visual de status |
| Potenciômetro       | A0       | Analógico         | Leitura de rotação / ajuste |

## Regras de Funcionamento

| Condição                          | Estado               | Ação Principal |
|-----------------------------------|----------------------|----------------|
| Temperatura > limite definido     | Alerta Térmico       | Ativar buzzer + LED Vermelho |
| Umidade < limite definido         | Ambiente Seco        | Sinalizar + registrar leitura |
| Botão pressionado                 | Comando Manual       | Alterar estado do sistema |
| Comando IR recebido               | Controle Remoto      | Executar ação correspondente |
| Rotação / Potenciômetro fora do padrão | Falha Operacional | Ativar alerta visual e sonoro |

## Fluxo Geral do Sistema

**Entrada → Processamento → Decisão → Ação → Envio de Dados**

1. Leitura dos sensores (DHT11, IR, Potenciômetro, Botões)
2. Processamento das informações
3. Aplicação das regras de negócio
4. Acionamento dos atuadores (LED RGB, Buzzer)
5. Estruturação dos dados em JSON
6. Envio via Wi-Fi (próxima etapa)

*(Insira aqui a imagem do fluxograma presente no PDF)*

**Nome sugerido da imagem:** `fluxograma_sistema.png`

## Conclusão da Modelagem
A modelagem realizada nesta etapa garante que o sistema seja **lógico, seguro e escalável**, servindo como base sólida para as etapas seguintes (coleta de dados, comunicação TCP/IP, banco de dados e integração com Google Sheets).

**Status:** Modelagem concluída e validada ✅
