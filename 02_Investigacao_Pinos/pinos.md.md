# 03 - Investigação_pinos

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
