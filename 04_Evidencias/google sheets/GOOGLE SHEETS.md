## 🎯 Objetivo

Investigar como os dados coletados pelo sistema IoT com :contentReference[oaicite:0]{index=0} podem ser enviados e armazenados em planilhas online do :contentReference[oaicite:1]{index=1}.

---

## 🔗 Como ocorre a integração

A integração entre o ESP8266 e o Google Sheets não é direta.  
É necessário utilizar um intermediário chamado **API**.

---

## 🔁 Fluxo da informação
ESP8266
↓ (TCP/IP via Wi-Fi)
API (Google Apps Script)
↓
Google Sheets
↓
Visualização dos dados

---

## ⚙️ Tecnologias possíveis

Durante a pesquisa, foram identificadas as seguintes abordagens:

---

### ✔️ 1. Google Apps Script (RECOMENDADO)

Cria uma API dentro do próprio Google Sheets.

Funcionalidades:
- Recebe requisições HTTP (GET ou POST)
- Processa dados enviados pelo ESP8266
- Insere automaticamente na planilha

✔ Vantagens:
- Gratuito
- Fácil implementação
- Integração direta com Google Sheets

---

### ✔️ 2. Python + Google Sheets API

Utiliza bibliotecas como `gspread` para comunicação com a API oficial do Google.

✔ Vantagens:
- Maior controle sobre os dados
- Escalável para sistemas maiores

❌ Desvantagens:
- Configuração mais complexa
- Necessita autenticação OAuth

---

### ✔️ 3. Webhooks / APIs externas

O ESP8266 envia dados para um servidor intermediário, que repassa para o Google Sheets.

✔ Vantagens:
- Alta flexibilidade
- Utilizado em sistemas IoT industriais reais

---

## 🔐 Autenticação

Para integração com Google Sheets, podem ser utilizados:

- Chave de API (simples)
- Conta de serviço (Google Cloud)
- Web App público (Google Apps Script)

---

## 📡 Método escolhido para o projeto

A equipe optou por utilizar:

- ✔ Google Apps Script como API
- ✔ Comunicação HTTP via ESP8266
- ✔ Envio de dados em formato JSON ou URL

---

## 📦 Exemplo de envio de dados

O ESP8266 pode enviar dados de duas formas:

### 🔹 Via URL (GET)
https://script.google.com/macros/s/XXXX/exec?sensor=IR&valor=1
---

### 🔹 Via JSON

```json id="jsongs1"
{
  "sensor": "IR",
  "valor": 1,
  "estado": "ATIVO"
}
