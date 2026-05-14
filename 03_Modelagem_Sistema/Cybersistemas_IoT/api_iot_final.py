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
CORS(app) # Permite que o Dashboard (HTML) aceda à API mesmo em domínios diferentes

# ================= CONFIGURAÇÕES DE ACESSO =================
TOKEN_SECRETO = 'CyberProject'

# IDs das tuas 3 Planilhas Google (fornecidos pelo utilizador)
SHEETS_CONFIG = [
    "1bUNMPcfU8TX94PbhaJo4BY4oEJAYoccNx6nh2bMZrcw", # Planilha 1
    "1i-JjwKm-nMN8rGkFIAg583FaO3pLqIIdhfsMm76fW6k", # Planilha 2
    "19nM9DBSt4Us8D6WIgvMR_EWAKRm2vzoFjrEar4eMZt0"  # Planilha 3
]

# String de Conexão SQL Server (Localhost)
SQL_CONN_STR = (
    'DRIVER={ODBC Driver 18 for SQL Server};'
    'SERVER=localhost;'
    'DATABASE=MonitoramentoIoT;'
    'UID=api_iot_login;'
    'PWD=SenhaForte@123;'
    'Encrypt=no;'
    'TrustServerCertificate=yes;'
)

# Configuração da API Google Sheets
SCOPE = ['https://spreadsheets.google.com/feeds', 'https://www.googleapis.com/auth/drive']
sheets_instances = []

def init_google_api():
    """Inicializa a conexão com as instâncias do Google Sheets."""
    try:
        # Certifica-te que o credentials.json está na mesma pasta!
        creds = ServiceAccountCredentials.from_json_keyfile_name('credentials.json', SCOPE)
        client = gspread.authorize(creds)
        
        for s_id in SHEETS_CONFIG:
            try:
                # Tenta abrir a primeira folha de cada planilha pelo ID
                inst = client.open_by_key(s_id).sheet1
                sheets_instances.append(inst)
                logging.info(f"✅ Sincronização Ativa: Planilha {s_id[:8]}...")
            except Exception as e:
                logging.error(f"❌ Falha ao abrir planilha {s_id[:8]}: {e}")
    except Exception as e:
        logging.error(f"🚨 Erro fatal no credentials.json: {e}")

# Inicia a conexão com a Nuvem ao arrancar
init_google_api()

# ================= ROTAS DA API =================

@app.route('/data', methods=['POST'])
def receive_data():
    """Recebe dados do Wemos, salva no SQL e replica nas 3 Planilhas."""
    auth = request.headers.get('Authorization')
    if auth != TOKEN_SECRETO:
        return jsonify({'error': 'Não autorizado'}), 401

    data = request.json
    # Estrutura: {"temp": float, "hum": float, "rot": int, "ir": int, "status": "string"}
    
    try:
        agora = datetime.now()
        ts_sql = agora.strftime('%Y-%m-%d %H:%M:%S')
        ts_sheets = agora.strftime('%d/%m/%Y %H:%M:%S')
        
        # 1. Gravação no SQL Server (Histórico Local)
        conn = pyodbc.connect(SQL_CONN_STR)
        cursor = conn.cursor()
        cursor.execute("""
            INSERT INTO Leituras (Temperatura, Umidade, Rotacao, IRDetectado, Status, DataHora)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (data['temp'], data['hum'], data['rot'], data['ir'], data['status'], ts_sql))
        conn.commit()
        conn.close()

        # 2. Replicação em Massa para Google Sheets (Nuvem)
        linha_google = [ts_sheets, data['temp'], data['hum'], data['rot'], data['ir'], data['status']]
        for sheet in sheets_instances:
            try:
                sheet.append_row(linha_google)
            except Exception as e:
                logging.warning(f"Erro ao atualizar folha Google: {e}")

        logging.info(f"📥 Dados Recebidos: T:{data['temp']} H:{data['hum']} | Sync: OK")
        return jsonify({'status': 'sucesso', 'notified_sheets': len(sheets_instances)}), 201

    except Exception as e:
        logging.error(f"Erro interno: {str(e)}")
        return jsonify({'error': str(e)}), 500

@app.route('/api/status', methods=['GET'])
def get_live_status():
    """Ponto de leitura para o Dashboard ler o último dado do banco."""
    try:
        conn = pyodbc.connect(SQL_CONN_STR)
        cursor = conn.cursor()
        cursor.execute("SELECT TOP 1 Temperatura, Umidade, Rotacao, IRDetectado, Status FROM Leituras ORDER BY Id DESC")
        row = cursor.fetchone()
        conn.close()
        
        if row:
            return jsonify({
                "temp": row[0], "hum": row[1], "rot": row[2], 
                "ir": row[3], "status": row[4]
            })
    except:
        pass
    return jsonify({"temp": 0, "hum": 0, "rot": 0, "ir": 0, "status": "offline"})

if __name__ == '__main__':
    # O IP 0.0.0.0 permite acesso externo pelo Wemos
    app.run(host='0.0.0.0', port=5000, debug=True)
