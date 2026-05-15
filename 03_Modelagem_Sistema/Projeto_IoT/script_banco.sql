-- ==============================================================================
-- PROJETO INTEGRADOR - SISTEMA INTELIGENTE DE MONITORAMENTO (IoT)
-- Criação Completa do Banco de Dados para MICROSOFT SQL SERVER (SSMS)
-- ==============================================================================

IF NOT EXISTS (SELECT * FROM sys.databases WHERE name = 'MonitoramentoIoT')
BEGIN
    CREATE DATABASE MonitoramentoIoT;
END
GO

USE MonitoramentoIoT;
GO

-- Tabela 1: Dispositivos
IF NOT EXISTS (SELECT * FROM sys.tables WHERE name = 'Dispositivos')
BEGIN
    CREATE TABLE Dispositivos (
        Id INT IDENTITY(1,1) PRIMARY KEY,
        Nome VARCHAR(50) NOT NULL UNIQUE,
        Descricao VARCHAR(200),
        IP VARCHAR(15),
        MacAddress VARCHAR(17),
        CriadoEm DATETIME DEFAULT GETDATE(),
        Ativo BIT DEFAULT 1
    );
END
GO

-- Tabela 2: Leituras dos Sensores
IF NOT EXISTS (SELECT * FROM sys.tables WHERE name = 'Leituras')
BEGIN
    CREATE TABLE Leituras (
        Id BIGINT IDENTITY(1,1) PRIMARY KEY,
        DispositivoId INT NOT NULL,
        DataLeitura DATETIME DEFAULT GETDATE(),
        Temperatura DECIMAL(5,2),
        Umidade DECIMAL(5,2),
        Rotacao INT,
        IRDetectado BIT,          
        Status VARCHAR(20) CHECK (Status IN ('normal', 'alerta', 'critico')),
        Qualidade INT DEFAULT 100,
        FOREIGN KEY (DispositivoId) REFERENCES Dispositivos(Id)
    );
END
GO

-- Configuração de Acesso para a API Python
IF NOT EXISTS (SELECT * FROM sys.server_principals WHERE name = 'api_iot_login')
BEGIN
    CREATE LOGIN api_iot_login WITH PASSWORD = 'SenhaForte@123', DEFAULT_DATABASE = MonitoramentoIoT;
END
GO

IF NOT EXISTS (SELECT * FROM sys.database_principals WHERE name = 'api_iot_login')
BEGIN
    CREATE USER api_iot_login FOR LOGIN api_iot_login;
    ALTER ROLE db_owner ADD MEMBER api_iot_login;
END
GO

-- Inserção do Wemos Inicial
SET IDENTITY_INSERT Dispositivos ON;
IF NOT EXISTS (SELECT 1 FROM Dispositivos WHERE Id = 1)
BEGIN
    INSERT INTO Dispositivos (Id, Nome, Descricao, Ativo) 
    VALUES (1, 'Wemos D1 - Celula 1', 'Placa ESP8266 Principal da Celula de Teste', 1);
END
SET IDENTITY_INSERT Dispositivos OFF;
GO