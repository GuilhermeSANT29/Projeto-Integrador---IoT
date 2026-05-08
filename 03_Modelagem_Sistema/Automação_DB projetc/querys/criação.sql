-- 1. Criação do Banco de Dados
CREATE DATABASE IF NOT EXISTS automacao_industrial;
USE automacao_industrial;

-- 2. Tabela de Sensores (Cadastro dos componentes)
CREATE TABLE IF NOT EXISTS sensores (
    id_sensor INT AUTO_INCREMENT PRIMARY KEY,
    nome VARCHAR(50) NOT NULL,       -- Ex: 'DHT11', 'Sensor Rotação'
    tipo VARCHAR(30),                -- Ex: 'Temperatura', 'Digital'
    unidade_medida VARCHAR(10)       -- Ex: '°C', 'RPM', '%'
);

-- 3. Tabela de Leituras (Dados contínuos enviados pelo ESP8266)
CREATE TABLE IF NOT EXISTS leituras (
    id_leitura INT AUTO_INCREMENT PRIMARY KEY,
    id_sensor INT,
    valor FLOAT NOT NULL,
    data_hora TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (id_sensor) REFERENCES sensores(id_sensor)
);

-- 4. Tabela de Eventos e Alertas (Logs do sistema)
CREATE TABLE IF NOT EXISTS eventos (
    id_evento INT AUTO_INCREMENT PRIMARY KEY,
    descricao VARCHAR(100),          -- Ex: 'Botão pressionado', 'Temp Crítica'
    estado_sistema VARCHAR(20),      -- Ex: 'Alerta', 'Normal'
    data_hora TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);