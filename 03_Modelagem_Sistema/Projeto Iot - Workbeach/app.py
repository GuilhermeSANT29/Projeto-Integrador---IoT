import mysql.connector

from flask import (
    Flask,
    request,
    jsonify,
    render_template,
    redirect,
    url_for
)

from flask_cors import CORS

from werkzeug.security import (
    generate_password_hash,
    check_password_hash
)

import json
from datetime import datetime
import os

import gspread
from google.oauth2.service_account import Credentials

# ==================================================
# CONFIGURAÇÃO FLASK
# ==================================================
app = Flask(__name__)

CORS(app)

# ==================================================
# CONFIGURAÇÕES GERAIS
# ==================================================
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

CAMINHO_JSON = os.path.join(
    BASE_DIR,
    "credentials.json"
)

AUTH_TOKEN = "CyberProject"

# ==================================================
# CONFIGURAÇÃO MYSQL
# ==================================================
MYSQL_CONFIG = {
    "host": "localhost",
    "user": "ADM_login",
    "password": "CyberCopilots123*",
    "database": "api_automacao"
}

# ==================================================
# GOOGLE SHEETS
# ==================================================
ESCOPOS = [
    "https://www.googleapis.com/auth/spreadsheets",
    "https://www.googleapis.com/auth/drive"
]

try:

    credenciais = Credentials.from_service_account_file(
        CAMINHO_JSON,
        scopes=ESCOPOS
    )

    cliente = gspread.authorize(credenciais)

    URL_PLANILHA = "https://docs.google.com/spreadsheets/d/1i-JjwKm-nMN8rGkFIAg583FaO3pLqIIdhfsMm76fW6k/edit?usp=sharing"

    planilha = cliente.open_by_url(URL_PLANILHA)

    aba = planilha.worksheet("Historico_Geral")

    print("✅ Google Sheets conectado!")

except Exception as e:

    print(f"❌ Erro Google Sheets: {e}")

# ==================================================
# VALIDAÇÃO SENSOR
# ==================================================
def validate_sensor_data(data):

    required = [
        "temperatura",
        "umidade",
        "rotacao",
        "estado"
    ]

    if not isinstance(data, dict):
        return False, "JSON inválido"

    for key in required:

        if key not in data:
            return False, f"Campo ausente: {key}"

    try:

        float(data["temperatura"])
        float(data["umidade"])
        int(data["rotacao"])
        str(data["estado"])

    except (ValueError, TypeError):

        return False, "Tipos inválidos"

    return True, ""

# ==================================================
# HOME
# ==================================================
@app.route("/")
def home():

    return redirect("/login")

# ==================================================
# REGISTER
# ==================================================
@app.route("/register", methods=["GET", "POST"])
def register():

    if request.method == "GET":

        return render_template("register.html")

    username = request.form.get("username")

    password = request.form.get("password")

    confirm_password = request.form.get(
        "confirm_password"
    )

    # ==================================================
    # VALIDAÇÕES
    # ==================================================
    if password != confirm_password:

        return render_template(
            "register.html",
            error="As senhas não coincidem"
        )

    if len(password) < 6:

        return render_template(
            "register.html",
            error="Senha muito curta"
        )

    try:

        conexao = mysql.connector.connect(
            **MYSQL_CONFIG
        )

        cursor = conexao.cursor()

        # VERIFICA USUÁRIO
        query_check = """
            SELECT id
            FROM usuarios
            WHERE username = %s
        """

        cursor.execute(
            query_check,
            (username,)
        )

        usuario_existente = cursor.fetchone()

        if usuario_existente:

            cursor.close()
            conexao.close()

            return render_template(
                "register.html",
                error="Usuário já existe"
            )

        # HASH SENHA
        senha_hash = generate_password_hash(
            password
        )

        # INSERT
        query_insert = """
            INSERT INTO usuarios
            (
                username,
                password_hash
            )
            VALUES (%s, %s)
        """

        cursor.execute(
            query_insert,
            (
                username,
                senha_hash
            )
        )

        conexao.commit()

        cursor.close()

        conexao.close()

        return render_template(
            "register.html",
            success="Usuário registrado com sucesso"
        )

    except Exception as e:

        return render_template(
            "register.html",
            error=f"Erro no servidor: {e}"
        )

# ==================================================
# LOGIN
# ==================================================
@app.route("/login", methods=["GET", "POST"])
def login():

    if request.method == "GET":

        return render_template("login.html")

    username = request.form.get("username")

    password = request.form.get("password")

    try:

        conexao = mysql.connector.connect(
            **MYSQL_CONFIG
        )

        cursor = conexao.cursor(
            dictionary=True
        )

        query = """
            SELECT *
            FROM usuarios
            WHERE username = %s
        """

        cursor.execute(
            query,
            (username,)
        )

        usuario = cursor.fetchone()

        cursor.close()

        conexao.close()

        if usuario:

            senha_correta = check_password_hash(
                usuario["password_hash"],
                password
            )

            if senha_correta:

                return """
                <body style="
                    background:#050a0f;
                    color:#00ff9d;
                    display:flex;
                    justify-content:center;
                    align-items:center;
                    height:100vh;
                    font-family:Arial;
                ">
                    <h1>
                    LOGIN REALIZADO COM SUCESSO 🚀
                    </h1>
                </body>
                """

        return render_template(
            "login.html",
            error="Usuário ou senha inválidos"
        )

    except Exception as e:

        return render_template(
            "login.html",
            error=f"Erro no servidor: {e}"
        )

# ==================================================
# API ESP8266
# ==================================================
@app.route("/receive_data", methods=["POST"])
def receive_data():

    print("\n===================================")

    print("📥 NOVA REQUISIÇÃO DO ESP8266")

    # ==================================================
    # TOKEN
    # ==================================================
    auth_header = request.headers.get(
        "Authorization"
    )

    if auth_header != AUTH_TOKEN:

        print("❌ Token inválido")

        return jsonify({
            "error": "Unauthorized"
        }), 401

    # ==================================================
    # JSON
    # ==================================================
    raw = request.get_data()

    if not raw:

        return jsonify({
            "error": "Body vazio"
        }), 400

    try:

        data = json.loads(raw)

    except json.JSONDecodeError:

        return jsonify({
            "error": "JSON inválido"
        }), 400

    # ==================================================
    # VALIDA
    # ==================================================
    ok, msg = validate_sensor_data(data)

    if not ok:

        print(f"❌ Dados inválidos: {msg}")

        return jsonify({
            "error": msg
        }), 400

    temperatura = float(data["temperatura"])

    umidade = float(data["umidade"])

    rotacao = int(data["rotacao"])

    estado = str(
        data["estado"]
    ).upper()

    agora_mysql = datetime.now()

    agora_sheets = agora_mysql.strftime(
        "%d/%m/%Y %H:%M:%S"
    )

    print(
        f"📊 Temp: {temperatura}C | "
        f"Umid: {umidade}% | "
        f"Rot: {rotacao} | "
        f"Estado: {estado}"
    )

    # ==================================================
    # MYSQL
    # ==================================================
    try:

        conexao = mysql.connector.connect(
            **MYSQL_CONFIG
        )

        cursor = conexao.cursor()

        query_leitura = """
            INSERT INTO leituras
            (
                dispositivo_id,
                temperatura,
                umidade,
                rotacao,
                estado,
                data_hora
            )
            VALUES (%s, %s, %s, %s, %s, %s)
        """

        valores_leitura = (
            1,
            temperatura,
            umidade,
            rotacao,
            estado,
            agora_mysql
        )

        cursor.execute(
            query_leitura,
            valores_leitura
        )

        conexao.commit()

        leitura_id = cursor.lastrowid

        print("🗄️ Dados salvos MySQL!")

        # ==================================================
        # ALERTA
        # ==================================================
        if estado in [
            "ALERTA",
            "CRÍTICO",
            "CRITICO"
        ]:

            query_alerta = """
                INSERT INTO alertas
                (
                    leitura_id,
                    tipo_alerta,
                    descricao,
                    data_hora
                )
                VALUES (%s, %s, %s, %s)
            """

            valores_alerta = (
                leitura_id,
                estado,
                f"Sistema detectou {estado}",
                agora_mysql
            )

            cursor.execute(
                query_alerta,
                valores_alerta
            )

            conexao.commit()

            print("⚠️ Alerta salvo!")

        cursor.close()

        conexao.close()

    except Exception as e:

        print(f"❌ Erro MySQL: {e}")

    # ==================================================
    # GOOGLE SHEETS
    # ==================================================
    try:

        aba.append_row([
            agora_sheets,
            temperatura,
            umidade,
            rotacao,
            estado
        ])

        print("☁️ Enviado para Sheets!")

    except Exception as e:

        print(f"❌ Erro Sheets: {e}")

    print("===================================\n")

    return jsonify({
        "status": "success",
        "message": "Dados recebidos",
        "timestamp": agora_sheets
    }), 201

# ==================================================
# START
# ==================================================
if __name__ == "__main__":

    print("🚀 API INICIADA!")

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )