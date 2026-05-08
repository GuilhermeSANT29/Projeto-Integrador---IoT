# 03 - Modelagem do Sistema

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

## Objetivo

Definir o comportamento completo do sistema antes da implementação do código, garantindo previsibilidade, organização e facilidade de manutenção durante todo o desenvolvimento.

A modelagem serve como um blueprint técnico que orienta todas as etapas posteriores do projeto.

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

## Fluxo Geral do Sistema

**Entrada → Processamento → Decisão → Ação → Envio de Dados**

1. **Entrada**: Leitura dos sensores (DHT11, Potenciômetro, IR e Botões)
2. **Processamento**: Tratamento dos valores lidos (verificação de NaN, conversões, etc.)
3. **Decisão**: Aplicação das regras de negócio definidas acima
4. **Ação**: Acionamento dos atuadores (LED RGB, Buzzer)
5. **Estruturação**: Criação do pacote de dados em JSON
6. **Envio**: Transmissão via Wi-Fi (TCP/IP) para servidor ou nuvem
 
 `fluxograma_sistema.png`

---

## Conclusão da Modelagem

A modelagem realizada nesta etapa garante que o sistema seja **lógico, seguro, previsível e escalável**. 

Com as regras de negócio bem definidas e o fluxo do sistema mapeado, conseguimos reduzir significativamente o risco de erros durante a programação e facilitar a integração futura com banco de dados, API e Google Sheets.

Esta fase consolida toda a base técnica construída nas Etapas 1 e 2, preparando o projeto para as etapas mais avançadas de comunicação e armazenamento de dados.

**Status:** Modelagem concluída e validada ✅

---
