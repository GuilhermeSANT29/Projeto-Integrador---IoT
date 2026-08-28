# Projeto Integrador — IoT Monitoring System

**Status:** Finalizado · **Versão:** 1.0 · **Plataforma:** ESP8266

Sistema de monitoramento de temperatura desenvolvido como projeto acadêmico, integrando **IoT, REST API, banco de dados, autenticação e visualização de dados**.

## Visão geral

O sistema coleta dados de temperatura por meio de um ESP8266 e disponibiliza as informações para armazenamento, consulta e visualização. Os registros históricos são enviados para o banco de dados e para o Google Sheets. O sistema também possui autenticação para acesso à interface web e um mecanismo de alerta para condições críticas.

## Arquitetura

```text
Sensor / ESP8266
       │
       ▼
Coleta de temperatura
       │
       ▼
REST API / Integração
       │
       ├──────────────► Banco de Dados
       │
       └──────────────► Google Sheets
       │
       ▼
Interface Web + Autenticação
       │
       ▼
Alertas / Monitoramento
```

## Funcionalidades

- Coleta de temperatura em tempo real
- Conectividade de rede pelo ESP8266
- Servidor web local para visualização
- Integração com REST API
- Persistência de dados em banco
- Registro histórico no Google Sheets
- Autenticação de usuários
- Mecanismo de alerta para anomalias de temperatura

## Tecnologias

- ESP8266 / Wemos
- Arduino IDE
- REST API
- Google Cloud API
- Google Sheets
- Banco de dados / SQL
- Sistema de autenticação

## Organização do repositório

A estrutura original foi preservada e organizada por etapa do desenvolvimento:

| Diretório | Finalidade |
| --- | --- |
| `01_Configuracao/` | Configuração inicial do ambiente e hardware |
| `02_Investigacao_Pinos/` | Investigação e testes dos pinos |
| `03_Modelagem_Sistema/` | Modelagem e definição da solução |
| `04_Evidencias/` | Evidências de funcionamento e testes |
| `05_Atualizacoes/` | Registro de atualizações do projeto |

## Equipe

- Guilherme Santos
- Arthur Murilo
- Maria Eduarda
- Jose Henrique

## Contexto

Projeto desenvolvido para fins acadêmicos. Além do objetivo educacional, o sistema demonstra integração entre **hardware, redes, APIs, dados e autenticação**, competências relacionadas ao meu foco em Software Engineering e Security Engineering.

## Licença

Projeto acadêmico. Consulte o arquivo `LICENSE` para os termos aplicáveis.
