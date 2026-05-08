USE master;
GO

CREATE LOGIN api_iot_login
WITH PASSWORD = 'SenhaForte@123',
     CHECK_POLICY = ON,
     CHECK_EXPIRATION = ON;
GO

USE MonitoramentoIoT;
GO

CREATE USER api_iot_user FOR LOGIN api_iot_login;
GO

ALTER ROLE db_datareader ADD MEMBER api_iot_user;
ALTER ROLE db_datawriter ADD MEMBER api_iot_user;
GO