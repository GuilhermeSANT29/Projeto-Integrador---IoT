from flask import Flask, request, jsonify, session, redirect, url_for, render_template
from flask_cors import CORS
from werkzeug.security import generate_password_hash, check_password_hash
import pyodbc
import json
from datetime import datetime
import os
import gspread
from google.oauth2.service_account import Credentials
from functools import wraps

app = Flask(__name__)
CORS(app)

# ==================================================
# CONFIGURAÇÕES DE SEGURANÇA E CAMINHOS
# ==================================================
app.secret_key = os.environ.get("FLASK_SECRET_KEY", "CyberProject_SecretKey_2025!#$")
# ⚠️  Em produção, defina FLASK_SECRET_KEY como variável de ambiente.
#     Exemplo no terminal: set FLASK_SECRET_KEY=sua_chave_super_secreta_aqui

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
CAMINHO_JSON = os.path.join(BASE_DIR, "credentials.json")

# Token de autenticação exclusivo para o ESP8266 (hardware)
AUTH_TOKEN_ESP = "CyberProject"

# ==================================================
# CONFIGURAÇÃO DO BANCO DE DADOS SQL SERVER
# ==================================================
DB_CONNECTION_STRING = (
    "DRIVER={ODBC Driver 17 for SQL Server};"
    "SERVER=localhost\\SQLEXPRESS;"
    "DATABASE=api_automacao;"
    "UID=ADM_login;"
    "PWD=CyberCopilots123*;"
)

def get_db_connection():
    """Retorna uma nova conexão com o banco de dados."""
    return pyodbc.connect(DB_CONNECTION_STRING)

# ==================================================
# CONFIGURAÇÃO DO GOOGLE SHEETS
# ==================================================
ESCOPOS = [
    "https://www.googleapis.com/auth/spreadsheets",
    "https://www.googleapis.com/auth/drive"
]

aba = None  # Objeto global da aba do Sheets
try:
    credenciais = Credentials.from_service_account_file(CAMINHO_JSON, scopes=ESCOPOS)
    cliente = gspread.authorize(credenciais)
    URL_PLANILHA = "https://docs.google.com/spreadsheets/d/1i-JjwKm-nMN8rGkFIAg583FaO3pLqIIdhfsMm76fW6k/edit?usp=sharing"
    planilha = cliente.open_by_url(URL_PLANILHA)
    aba = planilha.worksheet("Historico_Geral")
    print("✅ Google Sheets conectado com sucesso!")
except Exception as e:
    print(f"⚠️  Google Sheets não conectado: {e}")

# ==================================================
# DECORADOR DE PROTEÇÃO DE ROTAS (SESSION)
# ==================================================
def login_required(f):
    """
    Decorador que protege rotas web (HTML).
    Redireciona para /login se não houver sessão ativa.
    NÃO deve ser aplicado em rotas de API do ESP8266.
    """
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if "user_id" not in session:
            return redirect(url_for("login"))
        return f(*args, **kwargs)
    return decorated_function

# ==================================================
# FUNÇÃO DE VALIDAÇÃO DE DADOS DO SENSOR
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
# ROTAS DE AUTENTICAÇÃO WEB (HTML)
# ==================================================

@app.route("/register", methods=["GET", "POST"])
def register():
    """
    GET  → Exibe o formulário de cadastro.
    POST → Processa o cadastro: verifica duplicidade e salva o hash da senha.
    """
    if "user_id" in session:
        return redirect(url_for("dashboard"))

    error = None
    success = None

    if request.method == "POST":
        username = request.form.get("username", "").strip()
        password = request.form.get("password", "").strip()
        confirm  = request.form.get("confirm_password", "").strip()

        # --- Validações básicas ---
        if not username or not password:
            error = "Usuário e senha são obrigatórios."
        elif len(username) < 3:
            error = "O nome de usuário deve ter ao menos 3 caracteres."
        elif len(password) < 6:
            error = "A senha deve ter ao menos 6 caracteres."
        elif password != confirm:
            error = "As senhas não coincidem."
        else:
            try:
                conn = get_db_connection()
                cur = conn.cursor()

                # Verifica se o username já existe
                cur.execute("SELECT id FROM usuarios WHERE username = ?", (username,))
                if cur.fetchone():
                    error = "Nome de usuário já está em uso. Escolha outro."
                else:
                    # Gera o hash seguro da senha com werkzeug
                    senha_hash = generate_password_hash(password)

                    cur.execute(
                        "INSERT INTO usuarios (username, password_hash) VALUES (?, ?)",
                        (username, senha_hash)
                    )
                    conn.commit()
                    print(f"✅ Novo usuário cadastrado: {username}")
                    success = "Conta criada com sucesso! Faça login para continuar."

                cur.close()
                conn.close()

            except Exception as e:
                print(f"❌ Erro ao registrar usuário: {e}")
                error = "Erro interno ao criar conta. Tente novamente."

    return render_template("register.html", error=error, success=success)


@app.route("/login", methods=["GET", "POST"])
def login():
    """
    GET  → Exibe o formulário de login.
    POST → Valida credenciais e inicia a sessão Flask.
    """
    if "user_id" in session:
        return redirect(url_for("dashboard"))

    error = None

    if request.method == "POST":
        username = request.form.get("username", "").strip()
        password = request.form.get("password", "").strip()

        if not username or not password:
            error = "Preencha todos os campos."
        else:
            try:
                conn = get_db_connection()
                cur = conn.cursor()

                cur.execute(
                    "SELECT id, username, password_hash FROM usuarios WHERE username = ?",
                    (username,)
                )
                user = cur.fetchone()
                cur.close()
                conn.close()

                if user and check_password_hash(user.password_hash, password):
                    # Credenciais válidas: inicia a sessão
                    session.permanent = True
                    session["user_id"]   = user.id
                    session["username"]  = user.username
                    print(f"🔐 Login bem-sucedido: {username}")
                    return redirect(url_for("dashboard"))
                else:
                    error = "Usuário ou senha incorretos."
                    print(f"❌ Tentativa de login falhou para: {username}")

            except Exception as e:
                print(f"❌ Erro ao autenticar: {e}")
                error = "Erro interno. Tente novamente."

    return render_template("login.html", error=error)


@app.route("/logout")
def logout():
    """Encerra a sessão e redireciona para o login."""
    username = session.get("username", "desconhecido")
    session.clear()
    print(f"🚪 Logout: {username}")
    return redirect(url_for("login"))


# ==================================================
# ROTA PRINCIPAL — DASHBOARD (PROTEGIDA)
# ==================================================

@app.route("/")
@login_required
def dashboard():
    """
    Rota principal do site/dashboard.
    Protegida pelo decorador @login_required.
    Apenas usuários autenticados têm acesso.
    """
    try:
        conn = get_db_connection()
        cur = conn.cursor()

        # Busca as 10 últimas leituras para exibir no dashboard
        cur.execute("""
            SELECT TOP 10 data_hora, temperatura, umidade, rotacao, estado
            FROM leituras
            ORDER BY data_hora DESC
        """)
        leituras = cur.fetchall()

        # Busca os 5 últimos alertas
        cur.execute("""
            SELECT TOP 5 a.tipo_alerta, a.descricao, a.data_hora
            FROM alertas a
            ORDER BY a.data_hora DESC
        """)
        alertas = cur.fetchall()

        cur.close()
        conn.close()
    except Exception as e:
        print(f"❌ Erro ao buscar dados para o dashboard: {e}")
        leituras = []
        alertas = []

    return render_template(
        "dashboard.html",
        username=session.get("username"),
        leituras=leituras,
        alertas=alertas
    )


# ==================================================
# ROTA DA API ESP8266 — ISOLADA DA SESSÃO WEB
# ==================================================

@app.route("/receive_data", methods=["POST"])
def receive_data():
    """
    Endpoint exclusivo do ESP8266.
    Autenticado apenas por cabeçalho Authorization: CyberProject.
    Completamente isolado do sistema de sessões web (login/logout).
    """
    print("\n=============================================")
    print("📥 NOVA REQUISIÇÃO RECEBIDA DO ESP8266")

    # --- Autenticação por token de hardware (não usa session) ---
    auth_header = request.headers.get("Authorization")
    if auth_header != AUTH_TOKEN_ESP:
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

    agora_db     = datetime.now()
    agora_sheets = datetime.now().strftime("%d/%m/%Y %H:%M:%S")
    estado_fmt   = str(data["estado"]).upper()
    print(f"📊 Dados validados -> Temp: {data['temperatura']}°C | Umid: {data['umidade']}% | Rot: {data['rotacao']} RPM | Status: {estado_fmt}")

    # --- Inserção no SQL Server ---
    try:
        conn = get_db_connection()
        cur = conn.cursor()

        cur.execute(
            """INSERT INTO leituras (dispositivo_id, data_hora, temperatura, umidade, rotacao, estado)
               VALUES (?, ?, ?, ?, ?, ?)""",
            (1, agora_db, data["temperatura"], data["umidade"], data["rotacao"], estado_fmt)
        )
        conn.commit()
        print("🗄️  Leitura salva no SQL Server!")

        if estado_fmt in ["ALERTA", "CRÍTICO", "CRITICO"]:
            cur.execute("SELECT SCOPE_IDENTITY()")
            id_leitura = cur.fetchone()[0]
            cur.execute(
                """INSERT INTO alertas (leitura_id, tipo_alerta, descricao, data_hora)
                   VALUES (?, ?, ?, ?)""",
                (id_leitura, estado_fmt, f"Sistema detectou condição de {estado_fmt}.", agora_db)
            )
            conn.commit()
            print("⚠️  Alerta registrado na tabela de alertas!")

        cur.close()
        conn.close()
    except Exception as e:
        print(f"❌ Falha ao salvar no SQL Server: {e}")

    # --- Sincronização com Google Sheets ---
    try:
        if aba:
            aba.append_row([
                agora_sheets,
                data["temperatura"],
                data["umidade"],
                data["rotacao"],
                estado_fmt
            ])
            print("☁️  Dados sincronizados com o Google Sheets!")
    except Exception as e:
        print(f"❌ Falha ao enviar para o Google Sheets: {e}")

    print("=============================================\n")

    return jsonify({
        "status": "success",
        "message": "Dados integrados com SQL Server e Nuvem",
        "timestamp": str(agora_db)
    }), 201


# ==================================================
# INICIALIZAÇÃO
# ==================================================
if __name__ == "__main__":
    print("🚀 Iniciando API Integrada (SQL Server + Sheets + Auth)...")
    app.run(host="0.0.0.0", port=5000, debug=True)
