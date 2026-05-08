-- Leituras recentes
SELECT TOP 10 * FROM Leituras ORDER BY DataLeitura DESC;

-- Alertas não resolvidos
SELECT d.Nome, a.* FROM Alertas a JOIN Dispositivos d ON a.DispositivoId = d.Id WHERE Resolvido = 0;

-- Média temp por dispositivo
SELECT d.Nome, AVG(Temperatura) as MediaTemp FROM Leituras l JOIN Dispositivos d ON l.DispositivoId = d.Id GROUP BY d.Nome;

-- MTBF (tempo médio entre falhas)
SELECT AVG(DATEDIFF(MINUTE, DataLeitura, LEAD(DataLeitura) OVER (PARTITION BY DispositivoId ORDER BY DataLeitura))) as MTBF_Minutos
FROM Leituras;