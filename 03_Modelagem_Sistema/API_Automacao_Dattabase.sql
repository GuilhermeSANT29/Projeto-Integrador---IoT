-- 1. CRIAÇÃO DO BANCO DE DADOS
CREATE DATABASE IF NOT EXISTS api_automacao;
USE api_automacao;

-- 2. CRIAÇÃO DO USUÁRIO E PERMISSÕES (Conforme você solicitou)
CREATE USER IF NOT EXISTS 'ADM_login'@'localhost' IDENTIFIED BY 'CyberCopilots';
GRANT ALL PRIVILEGES ON api_automacao.* TO 'ADM_login'@'localhost';
FLUSH PRIVILEGES;

-- 3. CRIAÇÃO DAS TABELAS (Modelagem Industrial)

-- Tabela de Dispositivos (Sensores cadastrados)
CREATE TABLE IF NOT EXISTS dispositivos (
    id INT AUTO_INCREMENT PRIMARY KEY,
    nome VARCHAR(50) NOT NULL,
    tipo VARCHAR(50) NOT NULL, -- Ex: "ESP8266 + DHT11"
    localizacao VARCHAR(50) NOT NULL -- Ex: "Célula 1"
);

-- Tabela Principal de Leituras
CREATE TABLE IF NOT EXISTS leituras (
    id INT AUTO_INCREMENT PRIMARY KEY,
    dispositivo_id INT,
    temperatura DECIMAL(5,2) NOT NULL,
    umidade DECIMAL(5,2) NOT NULL,
    rotacao INT NOT NULL,
    estado VARCHAR(20) NOT NULL,
    data_hora DATETIME NOT NULL,
    CONSTRAINT fk_dispositivo FOREIGN KEY (dispositivo_id) REFERENCES dispositivos(id)
);

-- Tabela de Alertas e Eventos Críticos (Para rastreabilidade)
CREATE TABLE IF NOT EXISTS alertas (
    id INT AUTO_INCREMENT PRIMARY KEY,
    leitura_id INT,
    tipo_alerta VARCHAR(30) NOT NULL, -- Ex: "CRÍTICO", "FALHA_SENSOR"
    descricao VARCHAR(255) NOT NULL,
    data_hora DATETIME NOT NULL,
    CONSTRAINT fk_leitura FOREIGN KEY (leitura_id) REFERENCES leituras(id)
);

-- 4. INSERINDO UM DISPOSITIVO DE TESTE
INSERT INTO dispositivos (nome, tipo, localizacao) 
VALUES ('ESP8266_Wemos', 'Multisensor HY-M302', 'Bancada de Testes');