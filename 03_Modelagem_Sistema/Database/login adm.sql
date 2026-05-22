-- ======================================================
-- 1. SELECIONAR O MASTER PARA CRIAR O LOGIN NO SERVIDOR
-- ======================================================
USE master;
GO

-- Cria o Login se ele não existir
IF NOT EXISTS (SELECT * FROM sys.server_principals WHERE name = 'ADM_login')
BEGIN
    CREATE LOGIN ADM_login WITH PASSWORD = 'CyberCopilots123*', CHECK_EXPIRATION = OFF, CHECK_POLICY = OFF;
    PRINT '? Login ADM_login criado no servidor.';
END;
GO

-- ======================================================
-- 2. ASSOCIAR O LOGIN COMO UTILIZADOR DA API_AUTOMACAO
-- ======================================================
USE api_automacao;
GO

-- Cria o utilizador associado ao login dentro desta base de dados
IF NOT EXISTS (SELECT * FROM sys.database_principals WHERE name = 'ADM_login')
BEGIN
    CREATE USER ADM_login FOR LOGIN ADM_login;
    
    -- Atribui a permissão de Dono do Banco (db_owner) para gerir tabelas e dados
    ALTER ROLE db_owner ADD MEMBER ADM_login;
    
    PRINT '? Utilizador ADM_login associado à base de dados com permissões db_owner.';
END;
GO