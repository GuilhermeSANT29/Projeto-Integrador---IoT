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
# IP DO ESP8266
# ==================================================
ESP_IP = "192.168.0.108"

# ==================================================
# VARIÁVEIS GLOBAIS DASHBOARD
# ==================================================
ultima_temperatura = "--"
ultima_umidade = "--"
ultimo_estado = "NORMAL"
ultima_datahora = "--"

# ==================================================
# CONFIGURAÇÃO MYSQL
# ==================================================
MYSQL_CONFIG = {
    "host": "localhost",
    "user": "ADM_login",
    "password": "CyberCopilots",
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

        senha_hash = generate_password_hash(
            password
        )

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

        return redirect("/login")

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

                return redirect("/dashboard")

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
# DASHBOARD CYBERSISTEMAS
# ==================================================
@app.route("/dashboard")
def dashboard():

    global ultima_temperatura
    global ultima_umidade
    global ultimo_estado
    global ultima_datahora

    status = ultimo_estado.lower()

    if status == "critico":
        statusColor = "#ff003c"
        statusText = "CRÍTICO"
    elif status == "alerta":
        statusColor = "#ff9500"
        statusText = "ALERTA"
    else:
        statusColor = "#00ff41"
        statusText = "NORMAL"

    html = f"""
    <!DOCTYPE html>
    <html lang='pt-br'>
    <head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <meta http-equiv='refresh' content='5'>

    <title>Cybersistemas IOT Dashboard</title>

    <link href='https://fonts.googleapis.com/css2?family=Fira+Code:wght@400;500;600;700&display=swap' rel='stylesheet'>

    <style>

    :root{{
      --neon-blue:#00f2ff;
      --neon-green:#00ff41;
      --neon-red:#ff003c;
      --neon-orange:#ff9500;
      --bg-dark:#0a0a0f;
      --bg-card:rgba(20,20,30,0.98);
    }}

    body{{
      font-family:'Fira Code',monospace;
      background:linear-gradient(135deg,#0a0a0f 0%,#1a1a2e 50%,#0a0a0f 100%);
      color:#e5e5e5;
      margin:0;
      min-height:100vh;
    }}

    .header-main{{
      text-align:center;
      padding:3rem;
      border-bottom:1px solid rgba(0,255,65,0.3);
    }}

    .header-title{{
      font-size:4rem;
      background:linear-gradient(90deg,var(--neon-blue),var(--neon-green));
      -webkit-background-clip:text;
      -webkit-text-fill-color:transparent;
      font-weight:700;
    }}

    .header-sub{{
      opacity:0.8;
      margin-top:10px;
    }}

    main{{
      max-width:1000px;
      margin:auto;
      padding:40px;
    }}

    .cards-grid{{
      display:grid;
      grid-template-columns:1fr 1fr;
      gap:30px;
      margin-bottom:40px;
    }}

    .sensor-card{{
      background:var(--bg-card);
      border:2px solid rgba(0,255,65,0.25);
      border-radius:24px;
      padding:40px;
      text-align:center;
    }}

    .card-label{{
      font-size:1rem;
      opacity:0.7;
      margin-bottom:20px;
    }}

    .value-main{{
      font-size:5rem;
      color:var(--neon-blue);
      font-weight:700;
    }}

    .status-card{{
      background:var(--bg-card);
      border-radius:24px;
      padding:40px;
      border:2px solid {statusColor};
      text-align:center;
    }}

    .status-val{{
      font-size:3rem;
      color:{statusColor};
      font-weight:900;
    }}

    .status-msg{{
      margin-top:20px;
      opacity:0.8;
    }}

    .info-grid{{
      display:grid;
      grid-template-columns:repeat(auto-fit,minmax(180px,1fr));
      gap:20px;
      margin-top:30px;
    }}

    .info-item{{
      background:rgba(0,255,65,0.06);
      border:1px solid rgba(0,255,65,0.2);
      border-radius:16px;
      padding:20px;
      text-align:center;
    }}

    .info-label{{
      font-size:0.7rem;
      opacity:0.6;
      margin-bottom:10px;
    }}

    .info-val{{
      color:#00ff41;
      font-weight:700;
    }}

    footer{{
      text-align:center;
      padding:20px;
      opacity:0.5;
    }}

    </style>

    </head>

    <body>

    <header class='header-main'>
      <h1 class='header-title'>CYBERSISTEMAS IOT</h1>

      <p class='header-sub'>
      Dashboard Profissional • ESP8266 • DHT11 • Flask API • MySQL • Google Sheets
      </p>
    </header>

    <main>

      <div class='cards-grid'>

        <div class='sensor-card'>
          <div class='card-label'>TEMPERATURA</div>
          <div class='value-main'>{ultima_temperatura}°C</div>
        </div>

        <div class='sensor-card'>
          <div class='card-label'>UMIDADE</div>
          <div class='value-main'>{ultima_umidade}%</div>
        </div>

      </div>

      <div class='status-card'>

        <div class='status-val'>
        {statusText}
        </div>

        <div class='status-msg'>
        Sistema monitorando sensores em tempo real
        </div>

        <div class='info-grid'>

          <div class='info-item'>
            <div class='info-label'>ESP8266</div>
            <div class='info-val'>{ESP_IP}</div>
          </div>

          <div class='info-item'>
            <div class='info-label'>DATABASE</div>
            <div class='info-val'>MYSQL</div>
          </div>

          <div class='info-item'>
            <div class='info-label'>GOOGLE SHEETS</div>
            <div class='info-val'>ONLINE</div>
          </div>

          <div class='info-item'>
            <div class='info-label'>ÚLTIMA LEITURA</div>
            <div class='info-val'>{ultima_datahora}</div>
          </div>

        </div>

      </div>

    </main>

    <footer>
    CYBERSISTEMAS © 2026
    </footer>

    </body>
    </html>
    """

    return html

# ==================================================
# API ESP8266
# ==================================================
@app.route("/receive_data", methods=["POST"])
def receive_data():

    global ultima_temperatura
    global ultima_umidade
    global ultimo_estado
    global ultima_datahora

    print("\n===================================")

    print("📥 NOVA REQUISIÇÃO DO ESP8266")

    auth_header = request.headers.get(
        "Authorization"
    )

    if auth_header != AUTH_TOKEN:

        print("❌ Token inválido")

        return jsonify({
            "error": "Unauthorized"
        }), 401

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

    ok, msg = validate_sensor_data(data)

    if not ok:

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

    ultima_temperatura = temperatura
    ultima_umidade = umidade
    ultimo_estado = estado
    ultima_datahora = agora_sheets

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

        cursor.close()

        conexao.close()

        print("🗄️ Dados salvos no MySQL!")

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
# HORA
# ==================================================
@app.route("/hora")
def hora():

    agora = datetime.now().strftime(
        "%d/%m/%Y %H:%M:%S"
    )

    return jsonify({
        "datahora": agora
    })

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