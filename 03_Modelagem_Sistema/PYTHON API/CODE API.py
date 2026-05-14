from flask import Flask, request, jsonify
import pyodbc
import gspread
from oauth2client.service_account import ServiceAccountCredentials
from datetime import datetime
import logging

app = Flask(__name__)
logging.basicConfig(filename='api_iot.log', level=logging.INFO)

# ================= CONFIGURAÇÕES =================
TOKEN_SECRETO = 'CyberProject'

# Configuração da ligação ao SQL Server
SQL_CONN_STR = (
    'DRIVER={ODBC Driver 18 for SQL Server};'
    'SERVER=localhost;'
    'DATABASE=MonitoramentoIoT;'
    'UID=api_iot_login;'
    'PWD=SenhaForte@123;'
    'Encrypt=no;'
    'TrustServerCertificate=yes;'
)

# Configuração do Google Sheets
# O ficheiro credentials.json deve estar na mesma pasta deste script!
SCOPE = ['https://spreadsheets.google.com/feeds', 'https://www.googleapis.com/auth/drive']
try:
    CREDS = ServiceAccountCredentials.from_json_keyfile_name('credentials.json', SCOPE)
    gc = gspread.authorize(CREDS)
    sheet = gc.open('Monitoramento').sheet1
except Exception as e:
    print(f"Aviso: Erro ao carregar credenciais do Google Sheets: {e}")
    sheet = None

def get_db_connection():
    return pyodbc.connect(SQL_CONN_STR)

# ================= ROTA DE RECEBIMENTO DE DADOS =================
@app.route('/data', methods=['POST'])
def receive_data():
    # 1. Segurança: Verificação do Token
    auth = request.headers.get('Authorization')
    if not auth or auth != TOKEN_SECRETO:
        logging.warning("Tentativa de acesso não autorizado.")
        return jsonify({'error': 'Acesso Negado. Token inválido.'}), 401
    
    data = request.get_json()
    if not data:
        return jsonify({'error': 'Corpo da requisição vazio ou JSON inválido.'}), 400

    print(f"📦 Dados Recebidos do ESP8266: {data}")

    try:
        # 2. Inserção no Banco de Dados (SQL Server)
        conn = get_db_connection()
        cursor = conn.cursor()
        
        # Inserir na tabela Leituras (assume DispositivoId = 1 para o Wemos)
        cursor.execute("""
            INSERT INTO Leituras (DispositivoId, Temperatura, Umidade, Rotacao, IRDetectado, Status)
            VALUES (1, ?, ?, ?, ?, ?)
        """, (data['temp'], data['hum'], data['rot'], data['ir'], data['status']))
        
        # Registar na tabela de Alertas se houver anomalias
        if data['status'] in ['critico', 'alerta']:
            severidade = 'alta' if data['status'] == 'critico' else 'media'
            cursor.execute("""
                INSERT INTO Alertas (DispositivoId, TipoAlerta, Severidade, Descricao)
                VALUES (1, 'Alerta de Status', ?, ?)
            """, (severidade, f"Parâmetro fora do padrão. Temp: {data['temp']}°C, Status: {data['status']}"))
        
        conn.commit()
        conn.close()

        # 3. Envio para o Google Sheets (Nuvem)
        if sheet:
            data_hora_atual = datetime.now().strftime('%d/%m/%Y %H:%M:%S')
            linha = [
                data_hora_atual, 
                data['temp'], 
                data['hum'], 
                data['rot'], 
                data['ir'], 
                data['status']
            ]
            sheet.append_row(linha)
            logging.info("Dados processados e enviados para o Google Sheets.")
        
        return jsonify({'status': 'sucesso', 'mensagem': 'Dados registados no BD e Nuvem.'}), 201
    
    except Exception as e:
        logging.error(f"Erro interno no servidor: {str(e)}")
        print(f"Erro: {e}")
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    # Roda a API exposta na rede local na porta 5000 (0.0.0.0 permite acesso externo)
    print("🚀 API IoT a iniciar. A aguardar dados do ESP8266...")
    app.run(host='0.0.0.0', port=5000, debug=True)