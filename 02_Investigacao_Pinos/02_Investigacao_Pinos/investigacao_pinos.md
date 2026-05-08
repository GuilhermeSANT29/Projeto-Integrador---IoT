# 02 - Investigação das Portas Funcionais (ESP8266 + Shield HY-M302)

**Grupo Os Compilados**  
**Curso:** Técnico em Cibersistemas para Automação – SENAI Hermenegildo Campos de Almeida  
**Docente:** William Belarmino da Silva  
**Data:** Maio de 2026

**Integrantes:**  
- Guilherme Santos – documentador 
- Arthur Murilo – programador
- Maria Eduarda – analista
- José Henrique – testador

---

## Introdução

Realizamos uma investigação técnica detalhada dos pinos do microcontrolador **ESP8266 D1** (LOLIN Wemos D1 R2 & mini), considerando as limitações hardware do próprio módulo e as conexões específicas do **Shield HY-M302**.

O objetivo foi identificar quais pinos podem ser utilizados com segurança, evitando problemas de boot e conflitos de funcionamento.

---

## Tabela de Pinos Verificados e Utilizados

| Pino Placa | GPIO   | Componente          | Tipo de Sinal       | Status     | Observação |
|------------|--------|---------------------|---------------------|------------|----------|
| D4         | GPIO2  | LED RGB + LED Onboard | Saída Digital/PWM | Validado   | Compartilha linha com LED interno |
| D5         | GPIO14 | Buzzer              | Saída Digital       | Validado   | Ideal para alertas sonoros |
| D6         | GPIO12 | Receptor IR         | Entrada Digital     | Validado   | Detecção de comandos remotos |
| A0         | A0     | Potenciômetro       | Entrada Analógica   | Validado   | Leitura de rotação (0-1023) |

## Componentes do Shield HY-M302 Utilizados

| Componente          | Pino     | Tipo de Sinal       | Função Principal |
|---------------------|----------|---------------------|------------------|
| DHT11               | D4       | Digital             | Leitura de Temperatura e Umidade |
| Buzzer              | D5       | Saída Digital       | Alerta sonoro em condições críticas |
| Receptor IR         | D6       | Entrada Digital     | Recebimento de comandos remotos |
| LED RGB             | D4 / PWM | Saída PWM           | Indicação visual de status do sistema |
| Potenciômetro       | A0       | Entrada Analógica   | Leitura de rotação / ajuste de variáveis |

---

## Regras de Funcionamento

| Condição                                | Estado                | Ação Principal |
|-----------------------------------------|-----------------------|--------------|
| Temperatura > 30°C                      | Alerta Térmico        | Ativar buzzer + LED Vermelho |
| Umidade < 40%                           | Ambiente Seco         | Sinalizar + registrar leitura |
| Botão pressionado                       | Comando Manual        | Alterar estado do sistema |
| Comando IR recebido                     | Controle Remoto       | Executar ação correspondente ao comando |
| Rotação / Potenciômetro fora do padrão  | Falha Operacional     | Ativar alerta visual e sonoro |
---

### Recomendações de Uso

- **Pinos Recomendados (Alta prioridade):** D1, D2, D5, D6, D7  
- **Pinos com Restrições (usar com cuidado):** D4, A0  
- **Pinos a Evitar no Projeto:** D3, D8, TX (GPIO1), RX (GPIO3)

---

## Respostas

**1. Por que nem todos os pinos podem ser usados?**  
Nem todos os pinos do ESP8266 podem ser utilizados livremente porque alguns são **Strapping Pins** (pinos de configuração). Eles são lidos pelo microcontrolador durante o processo de inicialização (*boot*). Dependendo do nível lógico (HIGH ou LOW) desses pinos no momento da energização, o ESP8266 define o modo de operação (normal, download, etc.).

**2. O que pode acontecer se um pino inadequado for utilizado?**  
- Falha completa no boot da placa  
- Loop infinito de reinicialização  
- Impossibilidade de realizar upload de código  
- Travamento ou comportamento instável do sistema  
- Danos em componentes conectados

**3. Como isso impacta um sistema real de automação industrial?**  
Em um ambiente industrial, o uso incorreto de pinos pode causar:
- Paradas não programadas da máquina
- Perda de dados de monitoramento em tempo real
- Acionamento incorreto de atuadores (válvulas, motores, alarmes)
- Falhas de comunicação com sistemas supervisórios
- Riscos à segurança operacional e integridade dos equipamentos

---

## Fontes Consultadas

- Documentação oficial Espressif Systems (ESP8266 Datasheet)
- Random Nerd Tutorials – ESP8266 Pinout Guide
- Datasheet do Shield HY-M302
- Fóruns Arduino.cc e GitHub
- Espressif Technical Reference
