from flask import Flask, request, jsonify
from flask_cors import CORS

import pyodbc
import gspread
import logging
import os

from datetime import datetime
from oauth2client.service_account import ServiceAccountCredentials


# ==================================================
# FLASK
# ==================================================

app = Flask(__name__)
CORS(app)

logging.basicConfig(level=logging.INFO)


# ==================================================
# TOKEN
# ==================================================

TOKEN = "CyberProject"


# ==================================================
# SQL SERVER
# ==================================================

conexao = pyodbc.connect(

    'DRIVER={SQL Server};'
    'SERVER=.\\SQLEXPRESS;'
    'DATABASE=ProjetoIoT;'
    'Trusted_Connection=yes;'
)

cursor = conexao.cursor()

print("✅ SQL Server conectado!")


# ==================================================
# GOOGLE SHEETS
# ==================================================

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

# ARQUIVO JSON DA GOOGLE API
credenciais = os.path.join(

    BASE_DIR,

    "credentials.json.json"
)

scope = [

    "https://spreadsheets.google.com/feeds",

    "https://www.googleapis.com/auth/drive"
]


# ==================================================
# LOGIN GOOGLE
# ==================================================

creds = ServiceAccountCredentials.from_json_keyfile_name(

    credenciais,
    scope
)

client = gspread.authorize(creds)


# ==================================================
# PLANILHA
# ==================================================

sheet = client.open("TemperaturaESP").sheet1

print("✅ Google Sheets conectado!")


# ==================================================
# HOME
# ==================================================

@app.route("/")
def home():

    return "API funcionando!"


# ==================================================
# DATA / HORA
# ==================================================

@app.route("/hora")
def hora():

    return jsonify({

        "datahora": datetime.now().strftime("%d/%m/%Y %H:%M:%S")
    })


# ==================================================
# RECEBER DADOS ESP8266
# ==================================================

@app.route("/data", methods=["POST"])
def receber():

    try:

        print("================================")
        print("📡 NOVA REQUISIÇÃO")

        # ==========================================
        # TOKEN
        # ==========================================

        if request.headers.get("Authorization") != TOKEN:

            print("❌ Token inválido")

            return jsonify({

                "erro": "Não autorizado"

            }), 401

        # ==========================================
        # JSON
        # ==========================================

        data = request.json or {}

        print(data)

        # ==========================================
        # DADOS
        # ==========================================

        temperatura = data.get("temp", 0)

        rotacao = data.get("rot", 0)

        ir = 1 if data.get("ir") else 0

        status = str(

            data.get("status", "normal")

        ).lower().strip()

        ip = data.get("ip", "")

        agora = datetime.now()

        # ==========================================
        # LOGS
        # ==========================================

        print("IP:", ip)
        print("Temperatura:", temperatura)
        print("Rotação:", rotacao)
        print("IR:", ir)
        print("Status:", status)

        # ==========================================
        # SQL SERVER
        # ==========================================

        cursor.execute("""

INSERT INTO leituras (

    sensor_id,
    temperatura,
    umidade,
    data_hora

)

VALUES (?, ?, ?, ?)

""", (

    1,
    temperatura,
    0,
    agora

))

        conexao.commit()

        print("✅ Dados salvos no SQL Server!")

        # ==========================================
        # GOOGLE SHEETS
        # ==========================================

        sheet.append_row([

            agora.strftime("%d/%m/%Y %H:%M:%S"),

            temperatura,

            rotacao,

            ir,

            status,

            ip

        ])

        print("✅ Dados enviados ao Google Sheets!")

        print("================================")

        return jsonify({

            "status": "sucesso"

        }), 201

    except Exception as erro:

        print("❌ ERRO:")
        print(erro)

        return jsonify({

            "erro": str(erro)

        }), 500


# ==================================================
# STATUS AO VIVO
# ==================================================

@app.route("/api/sensors/live")
def live():

    try:

        cursor.execute("""

SELECT TOP 1

    temperatura

FROM leituras

ORDER BY id DESC

""")

        dado = cursor.fetchone()

        temp = 0

        if dado:

            temp = float(dado[0])

        return jsonify({

            "temp": temp,
            "status": "online"

        })

    except Exception as erro:

        print(erro)

        return jsonify({

            "temp": 0,
            "status": "erro"

        })


# ==================================================
# START
# ==================================================

if __name__ == "__main__":

    print("================================")
    print("🚀 API INICIADA")
    print("🌐 http://10.106.202.20:5000")
    print("================================")

    app.run(

        host="0.0.0.0",

        port=5000
    )