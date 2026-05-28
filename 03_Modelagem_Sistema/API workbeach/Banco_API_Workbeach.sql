-- =========================================================
-- CRIAR BANCO
-- =========================================================
CREATE DATABASE IF NOT EXISTS api_automacao;

USE api_automacao;

-- =========================================================
-- CRIAR USUÁRIO MYSQL
-- =========================================================
CREATE USER IF NOT EXISTS 'ADM_login'@'localhost'
IDENTIFIED BY 'CyberCopilots';

-- =========================================================
-- PERMISSÕES
-- =========================================================
GRANT ALL PRIVILEGES
ON api_automacao.*
TO 'ADM_login'@'localhost';

FLUSH PRIVILEGES;

-- =========================================================
-- TABELA DE USUÁRIOS
-- =========================================================
CREATE TABLE IF NOT EXISTS usuarios (
    
    id INT AUTO_INCREMENT PRIMARY KEY,
    
    username VARCHAR(100) NOT NULL UNIQUE,
    
    password_hash TEXT NOT NULL,
    
    criado_em TIMESTAMP DEFAULT CURRENT_TIMESTAMP

);

-- =========================================================
-- TABELA DISPOSITIVOS
-- =========================================================
CREATE TABLE IF NOT EXISTS dispositivos (

    id INT AUTO_INCREMENT PRIMARY KEY,

    nome VARCHAR(100) NOT NULL,

    ip_dispositivo VARCHAR(100),

    status_dispositivo VARCHAR(50) DEFAULT 'ONLINE',

    criado_em TIMESTAMP DEFAULT CURRENT_TIMESTAMP

);

-- =========================================================
-- INSERE DISPOSITIVO PADRÃO
-- =========================================================
INSERT INTO dispositivos
(
    nome,
    ip_dispositivo,
    status_dispositivo
)
VALUES
(
    'ESP8266 DHT11',
    '192.168.0.108',
    'ONLINE'
);

-- =========================================================
-- TABELA LEITURAS
-- =========================================================
CREATE TABLE IF NOT EXISTS leituras (

    id INT AUTO_INCREMENT PRIMARY KEY,

    dispositivo_id INT NOT NULL,

    temperatura DECIMAL(5,2),

    umidade DECIMAL(5,2),

    rotacao INT,

    estado VARCHAR(50),

    data_hora DATETIME DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT fk_dispositivo
    FOREIGN KEY (dispositivo_id)
    REFERENCES dispositivos(id)

);

-- =========================================================
-- TABELA ALERTAS
-- =========================================================
CREATE TABLE IF NOT EXISTS alertas (

    id INT AUTO_INCREMENT PRIMARY KEY,

    leitura_id INT NOT NULL,

    tipo_alerta VARCHAR(50),

    descricao TEXT,

    data_hora DATETIME DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT fk_leitura
    FOREIGN KEY (leitura_id)
    REFERENCES leituras(id)

);

-- =========================================================
-- VERIFICAR TABELAS
-- =========================================================
SHOW TABLES;

-- =========================================================
-- VERIFICAR USUÁRIOS
-- =========================================================
SELECT * FROM usuarios;

-- =========================================================
-- VERIFICAR LEITURAS
-- =========================================================
SELECT * FROM leituras;

-- =========================================================
-- VERIFICAR ALERTAS
-- =========================================================
SELECT * FROM alertas;

-- =========================================================
-- VERIFICAR DISPOSITIVOS
-- =========================================================
SELECT * FROM dispositivos;