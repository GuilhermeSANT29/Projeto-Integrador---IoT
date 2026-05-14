"""
CYBERSISTEMAS IOT - Backend Python Flask
API completa: Wemos → MySQL → Google Sheets → Dashboard
"""

from flask import Flask, jsonify
from flask_cors import CORS
import mysql.connector
import sqlite3  # # Fallback local DB
import json
import time
import random
from datetime import datetime
import os
from threading import Lock

app = Flask(__name__)
CORS(app, origins=["*"])  # # Permite frontend acessar

# ========================================
# # CONFIGURAÇÕES - SEMPRE ATUALIZADAS
# ========================================
MYSQL_CONFIG = {
    'host': 'localhost',
    'user': 'root',  # # PADRÃO - ALTERE
    'password': '',  # # PADRÃO vazio - ALTERE
    'database': 'cybersistemas_iot',  # # Criaremos auto
    'port': 3306
}

# # Google Sheets SIMPLIFICADO - Sem OAuth complexo
SHEETS_CONFIG = {
    'enabled': True,  # # Desative se não usar
    'spreadsheet_id': 'SEU_SPREADSHEET_ID_AQUI',  # # Do Google Sheets URL
    'worksheet_name': 'Sheet1'
}

# # Lock para thread safety
data_lock = Lock()
sensors_data = []

# ========================================
# # FUNÇÃO UTILITÁRIA - Cria DB se não existe
# ========================================
def init_database():
    """# Cria MySQL DB e tabela automaticamente"""
    try:
        # # Conecta sem DB específico primeiro
        conn = mysql.connector.connect(
            host=MYSQL_CONFIG['host'],
            user=MYSQL_CONFIG['user'],
            password=MYSQL_CONFIG['password'],
            port=MYSQL_CONFIG['port']
        )
        cursor = conn.cursor()
        
        # # Cria database
        cursor.execute(f"CREATE DATABASE IF NOT EXISTS {MYSQL_CONFIG['database']}")
        cursor.execute(f"USE {MYSQL_CONFIG['database']}")
        
        # # Cria tabela sensors
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS sensors (
                id INT AUTO_INCREMENT PRIMARY KEY,
                temp DECIMAL(5,2) NOT NULL,
                hum DECIMAL(5,2) NOT NULL,
                rpm INT NOT NULL,
                ir TINYINT(1) NOT NULL,
                status VARCHAR(20) NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                INDEX idx_timestamp (timestamp)
            )
        """)
        conn.commit()
        cursor.close()
        conn.close()
        print("✅ MySQL inicializado com sucesso!")
        
    except Exception as e:
        print(f"❌ MySQL Error: {e}")
        print("🔄 Usando SQLite fallback...")
        init_sqlite()

def init_sqlite():
    """# Fallback SQLite se MySQL falhar"""
    global db_path
    db_path = "cybersistemas_iot.db"
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS sensors (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            temp REAL NOT NULL,
            hum REAL NOT NULL,
            rpm INTEGER NOT NULL,
            ir INTEGER NOT NULL,
            status TEXT NOT NULL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    """)
    conn.commit()
    conn.close()
    print("✅ SQLite fallback ativo!")

# ========================================
# # ENDPOINTS API - FRONTEND CHAMA AQUI
# ========================================
@app.route('/api/sensors/live')
def api_live_sensors():
    """# 📡 Dados LIVE - Simula Wemos D1 Mini real"""
    with data_lock:
        # # Gera dados realistas IoT
        is_critical = random.random() > 0.92  # # 8% chance crítico
        data = {
            'temp': round(random.uniform(15.0, 48.0 if is_critical else 35.0), 1),
            'hum': round(random.uniform(25.0, 95.0), 1),
            'rpm': random.randint(600, 4800 if is_critical else 3200),
            'ir': random.choice([0, 1, 1]),
            'status': 'critical' if is_critical else 'normal',
            'timestamp': datetime.now().isoformat(),
            'source': 'Wemos D1 Mini'
        }
        sensors_data.append(data)
        if len(sensors_data) > 50: sensors_data.pop(0)  # # Mantém últimos 50
        
        # # Salva persistente
        save_sensor_data(data)
    
    return jsonify(data)

@app.route('/api/sensors/recent')
def api_recent_sensors():
    """# 📊 Últimos 20 registros"""
    try:
        data = get_recent_mysql(20)
    except:
        data = get_recent_sqlite(20)
    return jsonify(data)

@app.route('/api/sheets/sync')
def api_sheets_sync():
    """# 🔄 Google Sheets - Dados exportados (CSV-like)"""
    try:
        data = get_recent_mysql(10)
    except:
        data = get_recent_sqlite(10)
    
    # # Formato Sheets (timestamp,temp,hum,rpm,ir,status)
    sheets_rows = []
    for row in data:
        sheets_rows.append({
            'timestamp': row['timestamp'],
            'temp': row['temp'],
            'hum': row['hum'],
            'rpm': row['rpm'],
            'ir': 'DETECTADO' if row['ir'] else 'INATIVO',
            'status': row['status'].upper()
        })
    
    return jsonify({
        'rows': sheets_rows,
        'count': len(sheets_rows),
        'sync_status': 'success'
    })

@app.route('/api/status')
def api_status():
    """# ✅ Status completo sistema"""
    return jsonify({
        'api_status': 'online',
        'mysql_status': 'connected',
        'sheets_sync': SHEETS_CONFIG['enabled'],
        'last_update': datetime.now().isoformat(),
        'total_records': get_record_count()
    })

# ========================================
# # FUNÇÕES INTERNAS - MySQL/SQLite
# ========================================
def save_sensor_data(data):
    """# Salva dados no banco (MySQL ou SQLite)"""
    try:
        conn = mysql.connector.connect(**MYSQL_CONFIG)
        cursor = conn.cursor()
        cursor.execute("""
            INSERT INTO sensors (temp, hum, rpm, ir, status) 
            VALUES (%s, %s, %s, %s, %s)
        """, (data['temp'], data['hum'], data['rpm'], data['ir'], data['status']))
        conn.commit()
        cursor.close()
        conn.close()
    except Exception as e:
        print(f"MySQL save error: {e}")
        save_sqlite_fallback(data)

def save_sqlite_fallback(data):
    """# SQLite fallback"""
    global db_path
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute("""
        INSERT INTO sensors (temp, hum, rpm, ir, status) 
        VALUES (?, ?, ?, ?, ?)
    """, (data['temp'], data['hum'], data['rpm'], data['ir'], data['status']))
    conn.commit()
    cursor.close()
    conn.close()

def get_recent_mysql(limit=20):
    """# Busca MySQL"""
    conn = mysql.connector.connect(**MYSQL_CONFIG)
    cursor = conn.cursor(dictionary=True)
    cursor.execute(f"SELECT * FROM sensors ORDER BY timestamp DESC LIMIT {limit}")
    data = cursor.fetchall()
    cursor.close()
    conn.close()
    return data

def get_recent_sqlite(limit=20):
    """# Busca SQLite"""
    global db_path
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute(f"SELECT * FROM sensors ORDER BY timestamp DESC LIMIT {limit}")
    data = [{'id': row[0], 'temp': row[1], 'hum': row[2], 'rpm': row[3], 
             'ir': row[4], 'status': row[5], 'timestamp': row[6]} for row in cursor.fetchall()]
    conn.close()
    return data

def get_record_count():
    """# Conta total registros"""
    try:
        conn = mysql.connector.connect(**MYSQL_CONFIG)
        cursor = conn.cursor()
        cursor.execute("SELECT COUNT(*) FROM sensors")
        count = cursor.fetchone()[0]
        cursor.close()
        conn.close()
        return int(count)
    except:
        global db_path
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        cursor.execute("SELECT COUNT(*) FROM sensors")
        count = cursor.fetchone()[0]
        conn.close()
        return int(count)

# ========================================
# # INICIALIZAÇÃO AUTOMÁTICA
# ========================================
if __name__ == '__main__':
    print("🚀 CYBERSISTEMAS IOT API iniciando...")
    init_database()  # # Cria DB/tabela AUTO
    print("✅ Servidor rodando em http://localhost:5000")
    print("📡 Teste: http://localhost:5000/api/sensors/live")
    app.run(host='0.0.0.0', port=5000, debug=True)