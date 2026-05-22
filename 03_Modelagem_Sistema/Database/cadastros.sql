USE api_automacao;
GO

IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[usuarios]') AND type in (N'U'))
BEGIN
    CREATE TABLE usuarios (
        id INT IDENTITY(1,1) PRIMARY KEY,
        username VARCHAR(50) NOT NULL UNIQUE,
        password_hash VARCHAR(255) NOT NULL, -- Guarda a password encriptada (Segurança)
        data_criacao DATETIME DEFAULT GETDATE()
    );
    PRINT '? Tabela [usuarios] criada com sucesso.';
END;
GO