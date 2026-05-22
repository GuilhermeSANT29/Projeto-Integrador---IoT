-- ======================================================
-- 1. CRIAÇÃO DO BANCO DE DADOS
-- ======================================================
IF NOT EXISTS (SELECT * FROM sys.databases WHERE name = 'api_automacao')
BEGIN
    CREATE DATABASE api_automacao;
END;
GO

USE api_automacao;
GO

-- ======================================================
-- 2. CRIAÇÃO DAS TABELAS (Modelagem Industrial)
-- ======================================================

-- Tabela de Dispositivos (Sensores cadastrados)
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[dispositivos]') AND type in (N'U'))
BEGIN
    CREATE TABLE dispositivos (
        id INT IDENTITY(1,1) PRIMARY KEY,
        nome VARCHAR(50) NOT NULL,
        tipo VARCHAR(50) NOT NULL, -- Ex: "ESP8266 + DHT11"
        localizacao VARCHAR(50) NOT NULL -- Ex: "Célula 1"
    );
END;
GO

-- Tabela Principal de Leituras
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[leituras]') AND type in (N'U'))
BEGIN
    CREATE TABLE leituras (
        id INT IDENTITY(1,1) PRIMARY KEY,
        dispositivo_id INT,
        temperatura DECIMAL(5,2) NOT NULL,
        umidade DECIMAL(5,2) NOT NULL,
        rotacao INT NOT NULL,
        estado VARCHAR(20) NOT NULL,
        data_hora DATETIME NOT NULL,
        CONSTRAINT fk_dispositivo FOREIGN KEY (dispositivo_id) REFERENCES dispositivos(id)
    );
END;
GO

-- Tabela de Alertas e Eventos Críticos (Para rastreabilidade)
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[alertas]') AND type in (N'U'))
BEGIN
    CREATE TABLE alertas (
        id INT IDENTITY(1,1) PRIMARY KEY,
        leitura_id INT,
        tipo_alerta VARCHAR(30) NOT NULL, -- Ex: "CRÍTICO", "FALHA_SENSOR"
        descricao VARCHAR(255) NOT NULL,
        data_hora DATETIME NOT NULL,
        CONSTRAINT fk_leitura FOREIGN KEY (leitura_id) REFERENCES alertas(id) -- Mantido o padrão lógico de amarração
    );
END;
GO

-- ======================================================
-- 3. INSERINDO UM DISPOSITIVO DE TESTE (Evita duplicados)
-- ======================================================
IF NOT EXISTS (SELECT 1 FROM dispositivos WHERE nome = 'ESP8266_Wemos')
BEGIN
    INSERT INTO dispositivos (nome, tipo, localizacao) 
    VALUES ('ESP8266_Wemos', 'Multisensor HY-M302', 'Bancada de Testes');
END;
GO