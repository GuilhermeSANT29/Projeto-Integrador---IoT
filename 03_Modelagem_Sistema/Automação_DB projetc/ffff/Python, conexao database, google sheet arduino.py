from flask import Flask, request, jsonify
import pyodbc
import gspread
from oauth2client.service_account import ServiceAccountCredentials
from datetime import datetime
import logging

app = Flask(__name__)
logging.basicConfig(filename='app.log', level=logging.INFO)

# Conexão SQL Server (ajuste SERVER/UID/PWD)
SQL_CONN_STR = (
    'DRIVER={ODBC Driver 18 for SQL Server};'
    'SERVER=localhost;'
    'DATABASE=MonitoramentoIoT;'
    'UID=api_iot_login;'
    'PWD=SenhaForte@123;'
    'Encrypt=no;'
    'TrustServerCertificate=yes;'
)

# Google Sheets 
scope = ['https://spreadsheets.google.com/feeds', 'https://www.googleapis.com/auth/drive']
creds = ServiceAccountCredentials.from_json_keyfile_name('credentials.json', scope)
gc = gspread.authorize(creds)
sheet = gc.open('Monitoramento').sheet1  # Nome da planilha

def get_db_connection():
    return pyodbc.connect(SQL_CONN_STR)

@app.route('/data', methods=['POST'])
def receive_data():
    auth = request.headers.get('Authorization')
    if not auth or auth != 'seutoken123':
        return jsonify({'error': 'Unauthorized'}), 401
    
    data = request.get_json()
    logging.info(f"Dados recebidos: {data}")
    
    # Validação
    if not all(k in data for k in ['temp', 'hum', 'rot', 'ir', 'status']) or data['temp'] > 100:
        return jsonify({'status': 'invalid data'}), 400
    
    try:
        # Insere SQL Server
        conn = get_db_connection()
        cursor = conn.cursor()
        
        # Pega ID dispositivo (ESP1)
        cursor.execute("SELECT Id FROM Dispositivos WHERE Nome = ?", ('ESP8266-HY302',))
        dispositivo_id = cursor.fetchone()[0]
        
        cursor.execute("""
            INSERT INTO Leituras (DispositivoId, Temperatura, Umidade, Rotacao, IRDetectado, Status)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (dispositivo_id, data['temp'], data['hum'], data['rot'], data['ir'], data['status']))
        
        # Alerta se crítico
        if data['status'] == 'critico':
            cursor.execute("""
                INSERT INTO Alertas (DispositivoId, TipoAlerta, Severidade, Descricao)
                VALUES (?, 'Status Critico', 'critica', ?)
            """, (dispositivo_id, f"Temp: {data['temp']}, Status: {data['status']}"))
        
        conn.commit()
        conn.close()
        
        # Google Sheets
        row = [datetime.now().strftime('%Y-%m-%d %H:%M:%S'), data['temp'], data['hum'], 
               data['rot'], data['ir'], data['status']]
        sheet.append_row(row)
        
        logging.info("Dados salvos com sucesso")
        return jsonify({'status': 'OK', 'id': cursor.lastrowid})
    
    except Exception as e:
        logging.error(f"Erro: {str(e)}")
        return jsonify({'error': str(e)}), 500

@app.route('/leituras', methods=['GET'])
def get_leituras():
    try:
        conn = get_db_connection()
        cursor = conn.cursor()
        cursor.execute("""
            SELECT TOP 20 l.DataLeitura, l.Temperatura, l.Umidade, l.Status, d.Nome
            FROM Leituras l JOIN Dispositivos d ON l.DispositivoId = d.Id
            ORDER BY l.DataLeitura DESC
        """)
        rows = cursor.fetchall()
        conn.close()
        return jsonify([dict(row) for row in rows])
    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)