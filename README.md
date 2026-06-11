# 🚀 Projeto Integrador - IoT Grupo Os compilados

![Status](https://img.shields.io/badge/status-%20finalizado-green)
![Versão](https://img.shields.io/badge/version-1.0-blue)
![Plataforma](https://img.shields.io/badge/platform-ESP8266-green)
![Licença](https://img.shields.io/badge/license-Acadêmico-lightgrey)

---

## 👥 Integrantes

* Guilherme Santos
* Arthur Murilo
* Maria Eduarda
* Jose Henrique

---

## 📌 Descrição

This project focuses on the development of an intelligent monitoring system utilizing the ESP8266 microcontroller, integrated with a REST API, a database, and Google Sheets for real-time storage and visualization.

An Arduino equipped with an integrated shield captures ambient temperature data and transmits it to the system. Upon establishing an internet connection, the device hosts a local web server accessible via its IP address, displaying the current room temperature with a 20-second refresh interval. Historical temperature records are concurrently logged to both Google Sheets and the database. The system features an automated alarm mechanism that triggers during critical temperature anomalies. To secure access to the web interface, a user authentication system (registration and login) was implemented, with credentials stored directly within the database.

---

## 🎯 Objetivo

Criar uma solução capaz de:

* 📡 Coletar dados em tempo real (sensores)
* ⚙️ Processar informações automaticamente
* 💾 Armazenar dados em banco
* 🌐 Disponibilizar dados remotamente via API
* 📊 Integrar com Google Sheets para visualização

---

## 🧩 Etapas do Projeto

| Etapa                      | Status | Descrição            |
| -------------------------- | ------ | -------------------- |
| 01 - Configuração          | ✅      | Ambiente configurado |
| 02 - Investigação de Pinos | ✅      | Portas analisadas    |
| 03 - Modelagem do Sistema  | ✅    | Em andamento         |
| 04 - Evidências            | ✅    | Em atualização       |
| 05 - Atualizações          | 🔄     | Contínuo             |

---

## 🛠️ Tecnologias Utilizadas

* 💡 ESP8266_wemos16
* 🧰 Arduino IDE
* 🔗 Google Cloud API REST
* 📊 Google Sheets
* 🗄️ Banco de Dados

---

## 📄 Licença

Este projeto é destinado exclusivamente para fins acadêmicos.

---

## ⭐ Contribuição

Este projeto foi desenvolvido como parte de um teste Academico.
