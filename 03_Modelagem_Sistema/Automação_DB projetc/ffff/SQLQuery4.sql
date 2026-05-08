-- 1. Criação do Banco
IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = 'MonitoramentoIoT')
CREATE DATABASE MonitoramentoIoT;
GO
USE MonitoramentoIoT;
GO

-- 2. Tabelas (DER)
CREATE TABLE Dispositivos (
    Id INT IDENTITY(1,1) PRIMARY KEY,
    Nome VARCHAR(50) NOT NULL UNIQUE,
    Descricao VARCHAR(200),
    IP VARCHAR(15),
    MacAddress VARCHAR(17),
    CriadoEm DATETIME2 DEFAULT GETDATE(),
    Ativo BIT DEFAULT 1
);
GO

CREATE TABLE Leituras (
    Id BIGINT IDENTITY(1,1) PRIMARY KEY,
    DispositivoId INT NOT NULL FOREIGN KEY REFERENCES Dispositivos(Id),
    DataLeitura DATETIME2 DEFAULT GETDATE(),
    Temperatura DECIMAL(5,2),
    Umidade DECIMAL(5,2),
    Rotacao INT,
    IRDetectado BIT,
    Status VARCHAR(20) CHECK (Status IN ('normal', 'alerta', 'critico')),
    Qualidade INT DEFAULT 100 CHECK (Qualidade BETWEEN 0 AND 100)
);
GO

CREATE TABLE Alertas (
    Id INT IDENTITY(1,1) PRIMARY KEY,
    DispositivoId INT NOT NULL FOREIGN KEY REFERENCES Dispositivos(Id),
    LeituraId BIGINT FOREIGN KEY REFERENCES Leituras(Id),
    TipoAlerta VARCHAR(50),
    Severidade VARCHAR(20) CHECK (Severidade IN ('baixa', 'media', 'alta', 'critica')),
    DataAlerta DATETIME2 DEFAULT GETDATE(),
    Resolvido BIT DEFAULT 0,
    Descricao VARCHAR(500)
);
GO

CREATE TABLE Eventos (
    Id INT IDENTITY(1,1) PRIMARY KEY,
    DispositivoId INT NOT NULL FOREIGN KEY REFERENCES Dispositivos(Id),
    TipoEvento VARCHAR(50),
    DataEvento DATETIME2 DEFAULT GETDATE(),
    Detalhes VARCHAR(500)
);
GO

-- 3. Índices para Performance (IoT high volume)
CREATE INDEX IX_Leituras_Data ON Leituras(DataLeitura);
CREATE INDEX IX_Leituras_Dispositivo ON Leituras(DispositivoId);
CREATE INDEX IX_Alertas_Data ON Alertas(DataAlerta);
GO

