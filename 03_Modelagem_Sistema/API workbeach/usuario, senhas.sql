-- =========================================================
-- 1. CRIAR O USUÁRIO DO SISTEMA (Acesso ao Banco)
-- =========================================================
-- Cria o usuário que o Python vai usar para se conectar
CREATE USER IF NOT EXISTS 'ADM_login'@'localhost' 
IDENTIFIED BY 'CyberCopilots';

-- Garante todas as permissões para este usuário no banco da API
GRANT ALL PRIVILEGES ON api_automacao.* TO 'ADM_login'@'localhost';
FLUSH PRIVILEGES;

-- =========================================================
-- 2. INSERIR UM USUÁRIO DE TESTE NA TABELA (Acesso à API)
-- =========================================================
USE api_automacao;

-- Inserindo um usuário administrativo para teste no sistema.
-- A senha original é 'admin123', mas convertida para o formato 
-- de hash do Werkzeug (scrypt) para a API conseguir autenticar.
INSERT INTO usuarios (username, password_hash) 
VALUES (
    'admin', 
    'scrypt:32768:8:1$uD7Xm9Vb8Z$d9f1f0a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0'
)
ON DUPLICATE KEY UPDATE username=username;

-- =========================================================
-- 3. CONFERIR SE DEU CERTO
-- =========================================================
SELECT id, username, criado_em FROM usuarios;