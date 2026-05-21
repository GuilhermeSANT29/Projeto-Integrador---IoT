from flask import Flask, request, jsonify
from flask_cors import CORS
import mysql.connector
import json
from datetime import datetime
import os
import gspread
from google.oauth2.service_account import Credentials

app = Flask(__name__)
CORS(app)

# ==================================================
# CONFIGURAÇÕES DE CAMINHOS E SEGURANÇA
# ==================================================
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
CAMINHO_JSON = os.path.join(BASE_DIR, "credentials.json")
AUTH_TOKEN = "CyberProject"

# Credenciais do MySQL Workbench
DB_CONFIG = {
    'host': 'localhost',
    'user': 'ADM_login',
    'password': 'CyberCopilots',
    'database': 'api_automacao'
}

# ==================================================
# CONEXÃO COM GOOGLE SHEETS
# ==================================================
ESCOPOS = [
    "https://www.googleapis.com/auth/spreadsheets",
    "https://www.googleapis.com/auth/drive"
]

try:
    credenciais = Credentials.from_service_account_file(CAMINHO_JSON, scopes=ESCOPOS)
    cliente = gspread.authorize(credenciais)
    URL_PLANILHA = "https://docs.google.com/spreadsheets/d/1i-JjwKm-nMN8rGkFIAg583FaO3pLqIIdhfsMm76fW6k/edit?usp=sharing"
    planilha = cliente.open_by_url(URL_PLANILHA)
    aba = planilha.worksheet("Historico_Geral")
    print("✅ Google Sheets conectado com sucesso!")
except Exception as e:
    print(f"❌ Erro crítico ao conectar no Google Sheets: {e}")

# ==================================================
# FUNÇÃO DE VALIDAÇÃO INDUSTRIAL
# ==================================================
def validate_sensor_data(data):
    required = ["temperatura", "umidade", "rotacao", "estado"]
    if not isinstance(data, dict):
        return False, "JSON inválido"
    for key in required:
        if key not in data:
            return False, f"Campo obrigatório ausente: {key}"
    try:
        float(data["temperatura"])
        float(data["umidade"])
        int(data["rotacao"])
        str(data["estado"])
    except (ValueError, TypeError):
        return False, "Tipo de dado inválido nos campos medidos"
    return True, ""

# ==================================================
# ENDPOINT PRINCIPAL: RECEBIMENTO DE DADOS
# ==================================================
@app.route("/data", methods=["POST"])
def receive_data():
    print("\n=============================================")
    print("📥 NOVA REQUISIÇÃO RECEBIDA DO ESP8266")
    
    auth_header = request.headers.get("Authorization")
    if auth_header != AUTH_TOKEN:
        print("❌ Acesso bloqueado: Token Inválido")
        return jsonify({"error": "Unauthorized"}), 401

    raw = request.get_data()
    if not raw:
        return jsonify({"error": "Empty body"}), 400

    try:
        data = json.loads(raw)
    except json.JSONDecodeError:
        return jsonify({"error": "Invalid JSON format"}), 400

    ok, msg = validate_sensor_data(data)
    if not ok:
        print(f"❌ Dados rejeitados na validação: {msg}")
        return jsonify({"error": "Bad Request", "details": msg}), 400

    agora_mysql = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    agora_sheets = datetime.now().strftime("%d/%m/%Y %H:%M:%S")
    estado_formatado = str(data["estado"]).upper()
    print(f"📊 Dados validados -> Temp: {data['temperatura']}C | Umid: {data['umidade']}% | Rot: {data['rotacao']} | Status: {estado_formatado}")

    # 4. Inserção no MySQL Workbench
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cur = conn.cursor()
        
        query_leitura = """
            INSERT INTO leituras (dispositivo_id, data_hora, temperatura, umidade, rotacao, estado) 
            VALUES (%s, %s, %s, %s, %s, %s)
        """
        valores_leitura = (1, agora_mysql, data["temperatura"], data["umidade"], data["rotacao"], estado_formatado)
        
        cur.execute(query_leitura, valores_leitura)
        conn.commit()
        print("🗄️ Leitura salva com sucesso no MySQL!")
        
        if estado_formatado in ["ALERTA", "CRÍTICO", "CRITICO"]:
            id_da_leitura = cur.lastrowid
            query_alerta = """
                INSERT INTO alertas (leitura_id, tipo_alerta, descricao, data_hora)
                VALUES (%s, %s, %s, %s)
            """
            valores_alerta = (id_da_leitura, estado_formatado, f"Sistema detectou condição de {estado_formatado}.", agora_mysql)
            cur.execute(query_alerta, valores_alerta)
            conn.commit()
            print("⚠️ ATENÇÃO: Evento salvo na tabela de alertas!")
        
        cur.close()
        conn.close()
    except Exception as e:
        print(f"❌ Falha ao salvar no MySQL: {e}")

    # 5. Inserção no Google Sheets
    try:
        aba.append_row([
            agora_sheets, 
            data["temperatura"], 
            data["umidade"], 
            data["rotacao"], 
            estado_formatado
        ])
        print("☁️ Dados sincronizados com o Google Sheets!")
    except Exception as e:
        print(f"❌ Falha ao enviar para o Google Sheets: {e}")

    print("=============================================\n")

    return jsonify({
        "status": "success",
        "message": "Dados integrados com MySQL e Nuvem",
        "timestamp": agora_mysql
    }), 201

if __name__ == "__main__":
    print("🚀 Iniciando API Integrada (MySQL + Sheets)...")
    app.run(host="10.106.208.30", port=5000, debug=True)