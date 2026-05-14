from flask import Flask, jsonify, request
from flask_cors import CORS
import mysql.connector
import gspread
from oauth2client.service_account import ServiceAccountCredentials
import time
import random
from datetime import datetime

app = Flask(__name__)
CORS(app)

# # Config MySQL (substitua suas credenciais)
MYSQL_CONFIG = {
    'host': 'localhost',
    'user': 'seu_user',
    'password': 'sua_senha',
    'database': 'iot_cybersistemas'
}

# # Google Sheets Config
SHEETS_CONFIG = {
    'scope': "https://spreadsheets.google.com/feeds 'https://www.googleapis.com/auth/drive'",
    'creds': ServiceAccountCredentials.from_json_keyfile_name('sheets-credentials.json', SHEETS_CONFIG['scope'])
}

@app.route('/api/sensors/live')
def live_sensors():
    """# Endpoint principal - Dados simulados/reais Wemos"""
    data = {
        'temp': round(random.uniform(18, 45), 1),
        'hum': round(random.uniform(30, 90), 1),
        'rpm': random.randint(800, 4500),
        'ir': random.choice([0, 1]),
        'status': 'critical' if random.random() > 0.9 else 'normal',
        'timestamp': datetime.now().isoformat()
    }
    save_to_mysql(data)
    save_to_sheets(data)
    return jsonify(data)

@app.route('/api/mysql/recent')
def recent_mysql():
    """# Últimos 15 registros MySQL"""
    conn = mysql.connector.connect(**MYSQL_CONFIG)
    cursor = conn.cursor(dictionary=True)
    cursor.execute("SELECT * FROM sensors ORDER BY timestamp DESC LIMIT 15")
    data = cursor.fetchall()
    cursor.close()
    conn.close()
    return jsonify(data)

@app.route('/api/sheets/latest')
def sheets_latest():
    """# Proxy Google Sheets - Evita CORS"""
    scope = SHEETS_CONFIG['scope']
    creds = SHEETS_CONFIG['creds']
    client = gspread.authorize(creds)
    sheet = client.open("Cybersistemas_IOT").sheet1  # Seu sheet ID
    records = sheet.get_all_records()[-10:]  # Últimos 10
    return jsonify(records)

def save_to_mysql(data):
    """# Salva no MySQL"""
    conn = mysql.connector.connect(**MYSQL_CONFIG)
    cursor = conn.cursor()
    cursor.execute("""
        INSERT INTO sensors (temp, hum, rpm, ir, status, timestamp) 
        VALUES (%s, %s, %s, %s, %s, %s)
    """, (data['temp'], data['hum'], data['rpm'], data['ir'], data['status'], data['timestamp']))
    conn.commit()
    cursor.close()
    conn.close()

def save_to_sheets(data):
    """# Append Google Sheets"""
    scope = SHEETS_CONFIG['scope']
    creds = SHEETS_CONFIG['creds']
    client = gspread.authorize(creds)
    sheet = client.open("Cybersistemas_IOT").sheet1
    sheet.append_row([
        data['timestamp'], data['temp'], data['hum'], 
        data['rpm'], data['ir'], data['status']
    ])

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)