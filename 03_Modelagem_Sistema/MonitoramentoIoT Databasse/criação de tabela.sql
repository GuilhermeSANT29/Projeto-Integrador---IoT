CREATE TABLE sensors (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temp DECIMAL(5,2),
    hum DECIMAL(5,2),
    rpm INT,
    ir INT,
    status VARCHAR(20),
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);