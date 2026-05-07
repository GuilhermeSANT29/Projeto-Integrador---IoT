# Etapa 2 — Desenvolvimento da Coleta de Dados e Comunicação TCP/IP

**Grupo**  
**Curso:** Técnico em Cibersistemas para Automação – SENAI Hermenegildo Campos de Almeida  
**Docente:** William Belarmino da Silva  
**Data:** Maio de 2026

**Integrantes:**  
- Guilherme Santos –  
- Arthur Murilo –   
- Maria Eduarda – 
- José Henrique – 

---

## 1. Objetivo da Etapa

Desenvolver a coleta de dados dos sensores do Shield HY-M302, estruturar as informações e implementar a comunicação via rede TCP/IP utilizando o ESP8266.

Esta etapa representa a transição do planejamento teórico (Etapa 1) para um sistema funcional, capaz de coletar, processar e transmitir dados em tempo real.

---

## 2. Componentes Utilizados

| Componente          | Pino   | Tipo de Sinal      | Função no Projeto                     |
|---------------------|--------|--------------------|---------------------------------------|
| DHT11               | D4     | Digital            | Leitura de Temperatura e Umidade      |
| Buzzer              | D5     | Saída Digital      | Alerta sonoro em situações críticas   |
| Receptor IR         | D6     | Entrada Digital    | Detecção de comandos remotos          |
| Potenciômetro       | A0     | Entrada Analógica  | Leitura de rotação / ajuste           |
| LED RGB (onboard)   | D4     | Saída PWM          | Indicação visual de status            |

---
