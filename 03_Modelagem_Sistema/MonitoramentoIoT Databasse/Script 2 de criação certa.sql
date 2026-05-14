-- ==============================================================================
-- PROJETO INTEGRADOR - SISTEMA INTELIGENTE DE MONITORAMENTO (IoT)
-- Criação Completa do Banco de Dados para MYSQL WORKBENCH
-- Equipe: Os Compilados
-- ==============================================================================

-- 1. CRIAÇÃO DO BANCO DE DADOS
CREATE DATABASE IF NOT EXISTS MonitoramentoIoT;
USE MonitoramentoIoT;

-- ==============================================================================
-- 2. CRIAÇÃO DAS TABELAS (DER)
-- ==============================================================================

-- Tabela 2.1: Dispositivos (ESP8266 / Wemos D1)
CREATE TABLE IF NOT EXISTS Dispositivos (
    Id INT AUTO_INCREMENT PRIMARY KEY,
    Nome VARCHAR(50) NOT NULL UNIQUE,
    Descricao VARCHAR(200),
    IP VARCHAR(15),
    MacAddress VARCHAR(17),
    CriadoEm DATETIME DEFAULT CURRENT_TIMESTAMP,
    Ativo BOOLEAN DEFAULT 1
);

-- Tabela 2.2: Leituras dos Sensores
CREATE TABLE IF NOT EXISTS Leituras (
    Id BIGINT AUTO_INCREMENT PRIMARY KEY,
    DispositivoId INT NOT NULL,
    DataLeitura DATETIME DEFAULT CURRENT_TIMESTAMP,
    Temperatura DECIMAL(5,2),
    Umidade DECIMAL(5,2),
    Rotacao INT,
    IRDetectado BOOLEAN,
    Status VARCHAR(20) CHECK (Status IN ('normal', 'alerta', 'critico')),
    Qualidade INT DEFAULT 100 CHECK (Qualidade BETWEEN 0 AND 100),
    FOREIGN KEY (DispositivoId) REFERENCES Dispositivos(Id)
);

-- Tabela 2.3: Histórico de Alertas e Eventos
CREATE TABLE IF NOT EXISTS Alertas (
    Id INT AUTO_INCREMENT PRIMARY KEY,
    DispositivoId INT NOT NULL,
    TipoAlerta VARCHAR(50),
    Severidade VARCHAR(20) CHECK (Severidade IN ('baixa', 'media', 'alta', 'critica')),
    DataAlerta DATETIME DEFAULT CURRENT_TIMESTAMP,
    Resolvido BOOLEAN DEFAULT 0,
    Descricao VARCHAR(500),
    FOREIGN KEY (DispositivoId) REFERENCES Dispositivos(Id)
);

-- ==============================================================================
-- 3. INSERÇÃO DO DISPOSITIVO PADRÃO (PASSO CRÍTICO)
-- ==============================================================================
-- Usamos INSERT IGNORE para não dar erro se já existir
INSERT IGNORE INTO Dispositivos (Id, Nome, Descricao, Ativo) 
VALUES (1, 'Wemos D1 - Célula 1', 'Placa ESP8266 Principal da Célula de Teste', 1);

-- ==============================================================================
-- 4. CONFIGURAÇÃO DE SEGURANÇA E ACESSO DA API
-- ==============================================================================
-- Cria o usuário para a API (se não existir)
CREATE USER IF NOT EXISTS 'api_iot_login'@'localhost' IDENTIFIED BY 'SenhaForte@123';

-- Dá permissões totais apenas no banco MonitoramentoIoT
GRANT ALL PRIVILEGES ON MonitoramentoIoT.* TO 'api_iot_login'@'localhost';

-- Atualiza os privilégios
FLUSH PRIVILEGES;