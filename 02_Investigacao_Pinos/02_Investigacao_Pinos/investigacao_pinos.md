# 02 - Investigação das Portas Funcionais (ESP8266 + Shield HY-M302)

## Introdução
Realizada uma investigação técnica detalhada dos pinos do ESP8266 para identificar quais podem ser utilizados com segurança no projeto, considerando as restrições de boot e as conexões do Shield HY-M302.

## Tabela de Pinos Verificados e Utilizados

| Pino Placa | GPIO   | Componente      | Tipo              | Status      | Observação |
|------------|--------|-----------------|-------------------|-------------|----------|
| D4         | GPIO2  | LED RGB         | Saída Digital/PWM | Validado    | Compartilha linha com LED onboard |
| D5         | GPIO14 | Buzzer          | Saída Digital     | Validado    | Ideal para alertas sonoros |
| D6         | GPIO12 | Receptor IR     | Entrada Digital   | Validado    | Recepção de comandos remotos |

**Pinos Recomendados para o Projeto:** D1, D2, D5, D6, D7  
**Pinos com Restrições (evitar):** D3, D4, D8, TX, RX  
**Pino Analógico:** A0 (Potenciômetro / Rotação)

## Respostas Técnicas (Obrigatórias)

**1. Por que nem todos os pinos podem ser usados?**  
Alguns pinos do ESP8266 são chamados de **Strapping Pins**. Eles são utilizados durante o processo de boot para definir o modo de inicialização do microcontrolador. Se estiverem em nível lógico incorreto no momento da energização, a placa não inicializa corretamente.

**2. O que pode acontecer se um pino inadequado for utilizado?**  
- Falha no boot  
- Loop infinito de reinicialização  
- Impossibilidade de upload do código  
- Travamento do sistema

**3. Como isso impacta um sistema real de automação industrial?**  
Pode gerar paradas não programadas, perda de dados de monitoramento, acionamento incorreto de atuadores, falhas de comunicação e, em casos críticos, comprometer a segurança do processo industrial.

## Fontes Consultadas
- Documentação oficial Espressif Systems
- Random Nerd Tutorials – ESP8266 Pinout
- Datasheet Shield HY-M302
- Fóruns Arduino e GitHub

## Evidências
*(Colocar na pasta `prints/`)*  
- Capturas de pesquisas  
- Fotos dos testes práticos com o Shield  
- Prints do Monitor Serial

**Status:** Investigação concluída e documentada ✅
