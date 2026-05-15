from flask import Flask, request, jsonify
from flask_cors import CORS
import pyodbc
import gspread
from oauth2client.service_account import ServiceAccountCredentials
from datetime import datetime
import logging

# Configuração de Logs Profissionais
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

app = Flask(__name__)
CORS(app) # Permite a comunicação com o seu Dashboard HTML

TOKEN_SECRETO = 'CyberProject'

# ID ÚNICO DA SUA PLANILHA REAL (ATUALIZADO)
SPREADSHEET_ID = "1i-JjwKm-nMN8rGkFIAg583FaO3pLqIIdhfsMm76fW6k"

# String de Conexão com o SQL Server (SSMS)
SQL_CONN_STR = (
    'DRIVER={ODBC Driver 18 for SQL Server};'
    'SERVER=localhost;'
    'DATABASE=MonitoramentoIoT;'
    'UID=api_iot_login;'
    'PWD=SenhaForte@123;'
    'Encrypt=yes;'
    'TrustServerCertificate=yes;'
)

SCOPE = ['https://spreadsheets.google.com/feeds', 'https://www.googleapis.com/auth/drive']

# Referências globais para as abas específicas
aba_historico = None
aba_criticos = None

def init_google_api():
    global aba_historico, aba_criticos
    try:
        creds = ServiceAccountCredentials.from_json_keyfile_name('credentials.json', SCOPE)
        client = gspread.authorize(creds)
        planilha_mae = client.open_by_key(SPREADSHEET_ID)

        logging.info("Abas encontradas:")
        for ws in planilha_mae.worksheets():
            logging.info(f"- {ws.title}")

        aba_historico = planilha_mae.worksheet("Historico_Geral")
        aba_criticos = planilha_mae.worksheet("Ocorrencias_Criticas")

        logging.info("Google Sheets conectado!")
    except Exception as e:
        logging.error(f"Erro Google Sheets: {type(e).__name__}: {e}")
        aba_historico = None
        aba_criticos = None

# 1. RECEBIMENTO DE DADOS DO ARDUINO/WEMOS
@app.route('/data', methods=['POST'])
def receive_data():
    if request.headers.get('Authorization') != TOKEN_SECRETO:
        return jsonify({'error': 'Não autorizado'}), 401

    data = request.json
    try:
        agora = datetime.now()
        ts_sql = agora.strftime('%Y-%m-%d %H:%M:%S')
        ts_sheets = agora.strftime('%d/%m/%Y %H:%M:%S')
        ir_bit = 1 if data.get('ir') else 0
        status_atual = str(data.get('status', 'normal')).lower().strip()
        
        # ---------------- TRATAMENTO NO SQL SERVER ----------------
        conn = pyodbc.connect(SQL_CONN_STR)
        cursor = conn.cursor()
        cursor.execute("UPDATE Dispositivos SET IP = ? WHERE Id = 1", (data.get('ip', '10.106.202.20'),))
        cursor.execute("""
            INSERT INTO Leituras (DispositivoId, Temperatura, Umidade, Rotacao, IRDetectado, Status, DataLeitura)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        """, (1, data.get('temp', 0), data.get('hum', 0), data.get('rot', 0), ir_bit, status_atual, ts_sql))
        conn.commit()
        conn.close()

        # ---------------- TRATAMENTO NO GOOGLE SHEETS ----------------
        linha_google = [ts_sheets, data.get('temp'), data.get('hum'), data.get('rot'), ir_bit, status_atual]
        
        # Envia sempre para o histórico geral
        if aba_historico:
            try:
                aba_historico.append_row(linha_google)
            except Exception as e:
                logging.warning(f"Erro ao salvar na aba Historico_Geral: {e}")
        
        # Filtro de Inteligência: Envia duplicado para a aba de críticas apenas se houver anormalidades
        if status_atual in ['alerta', 'critico'] and aba_criticos:
            try:
                aba_criticos.append_row(linha_google)
                logging.info(f"⚠️ Alerta/Crítico gravado na aba de Ocorrências Críticas!")
            except Exception as e:
                logging.warning(f"Erro ao salvar na aba Ocorrencias_Criticas: {e}")

        return jsonify({'status': 'sucesso'}), 201
    except Exception as e:
        logging.error(f"Erro interno no servidor: {str(e)}")
        return jsonify({'error': str(e)}), 500

# 2. ENDPOINT PARA LEITURA EM TEMPO REAL DO DASHBOARD HTML
@app.route('/api/sensors/live', methods=['GET'])
def get_live_status():
    try:
        conn = pyodbc.connect(SQL_CONN_STR)
        cursor = conn.cursor()
        cursor.execute("SELECT TOP 1 Temperatura, Umidade, Rotacao, IRDetectado, Status FROM Leituras ORDER BY Id DESC")
        row = cursor.fetchone()
        conn.close()
        if row:
            status = str(row[4]).strip()
            msg = "Temperatura crítica - Intervenção necessária imediata" if status == "critico" else "Todas operações nominais"
            return jsonify({
                "temp": float(row[0]) if row[0] is not None else 0.0,
                "hum": float(row[1]) if row[1] is not None else 0.0,
                "rpm": int(row[2]) if row[2] is not None else 0,
                "ir": int(row[3]),
                "status": status,
                "message": msg
            })
    except Exception as e:
        logging.error(f"Erro live status: {e}")
    return jsonify({"temp": 0, "hum": 0, "rpm": 0, "ir": 0, "status": "normal", "message": "Aguardando dados"})

# 3. ENDPOINT DA TABELA DATABASE LIVE STREAM (Puxa do SQL Server de forma veloz)
@app.route('/api/mysql/recent', methods=['GET'])
def get_mysql_recent():
    try:
        limit = request.args.get('limit', default=15, type=int)
        conn = pyodbc.connect(SQL_CONN_STR)
        cursor = conn.cursor()
        cursor.execute(f"SELECT TOP {limit} DataLeitura, Temperatura, Umidade, Rotacao, IRDetectado, Status FROM Leituras ORDER BY Id DESC")
        rows = cursor.fetchall()
        conn.close()
        return jsonify([{
            "timestamp": r[0].strftime('%Y-%m-%d %H:%M:%S'), "temp": float(r[1]), "hum": float(r[2]),
            "rpm": int(r[3]), "ir": int(r[4]), "status": str(r[5]).strip()
        } for r in rows])
    except Exception as e:
        return jsonify([]), 500

# 4. ENDPOINT DA TABELA GOOGLE SHEETS LOGS
@app.route('/api/sheets/latest', methods=['GET'])
def get_sheets_latest():
    try:
        rows_num = request.args.get('rows', default=10, type=int)
        conn = pyodbc.connect(SQL_CONN_STR)
        cursor = conn.cursor()
        cursor.execute(f"SELECT TOP {rows_num} DataLeitura, Temperatura, Umidade, Rotacao, IRDetectado, Status FROM Leituras ORDER BY Id DESC")
        rows = cursor.fetchall()
        conn.close()
        return jsonify([{
            "timestamp": r[0].strftime('%d/%m/%Y %H:%M:%S'), "temp": float(r[1]), "hum": float(r[2]),
            "rpm": int(r[3]), "ir": int(r[4]), "status": str(r[5]).strip()
        } for r in rows])
    except Exception as e:
        return jsonify([]), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)