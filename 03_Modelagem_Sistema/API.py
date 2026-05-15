from flask import Flask, request, jsonify
from flask_cors import CORS
from datetime import datetime
from oauth2client.service_account import ServiceAccountCredentials

import pyodbc
import gspread
import logging
import os


# ==================================================
# CONFIG
# ==================================================

app = Flask(__name__)
CORS(app)

logging.basicConfig(level=logging.INFO)

TOKEN = "CyberProject"

SQL_CONN = (
    "DRIVER={ODBC Driver 18 for SQL Server};"
    "SERVER=localhost;"
    "DATABASE=MonitoramentoIoT;"
    "UID=login_adm;"
    "PWD=0s_comp1ladx;"
    "Encrypt=yes;"
    "TrustServerCertificate=yes;"
)

SCOPE = [
    "https://www.googleapis.com/auth/spreadsheets",
    "https://www.googleapis.com/auth/drive"
]

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
CREDS_FILE = os.path.join(BASE_DIR, "credentials.json")

SPREADSHEET_ID = "1i-JjwKm-nMN8rGkFIAg583FaO3pLqIIdhfsMm76fW6k"


# ==================================================
# GOOGLE SHEETS
# ==================================================

creds = ServiceAccountCredentials.from_json_keyfile_name(
    CREDS_FILE,
    SCOPE
)

client = gspread.authorize(creds)
sheet = client.open_by_key(SPREADSHEET_ID)

history_sheet = sheet.worksheet("Historico_Geral")
critical_sheet = sheet.worksheet("Ocorrencias_Criticas")


# ==================================================
# HELPERS
# ==================================================

def db():
    return pyodbc.connect(SQL_CONN)


def save_sheets(row, status):
    history_sheet.append_row(row)

    if status in ["alerta", "critico"]:
        critical_sheet.append_row(row)


def latest_reading():
    with db() as conn:
        cursor = conn.cursor()

        cursor.execute("""
            SELECT TOP 1
                Temperatura,
                Rotacao,
                IRDetectado,
                Status
            FROM Leituras
            ORDER BY Id DESC
        """)

        return cursor.fetchone()


# ==================================================
# ROUTES
# ==================================================

@app.route("/data", methods=["POST"])
def receive_data():

    if request.headers.get("Authorization") != TOKEN:
        return jsonify({"error": "Não autorizado"}), 401

    try:
        data = request.json or {}

        now = datetime.now()

        row = [
            now.strftime("%d/%m/%Y %H:%M:%S"),
            data.get("temp", 0),
            data.get("rot", 0),
            1 if data.get("ir") else 0,
            str(data.get("status", "normal")).lower().strip()
        ]

        ip = data.get("ip", "")

        with db() as conn:
            cursor = conn.cursor()

            cursor.execute(
                "UPDATE Dispositivos SET IP = ? WHERE Id = 1",
                (ip,)
            )

            cursor.execute("""
                INSERT INTO Leituras
                (
                    DispositivoId,
                    Temperatura,
                    Rotacao,
                    IRDetectado,
                    Status,
                    DataLeitura
                )
                VALUES (?, ?, ?, ?, ?, ?)
            """, (
                1,
                row[1],
                row[2],
                row[3],
                row[4],
                now.strftime("%Y-%m-%d %H:%M:%S")
            ))

            conn.commit()

        save_sheets(row, row[4])

        return jsonify({"status": "sucesso"}), 201

    except Exception as e:
        logging.error(e)
        return jsonify({"error": str(e)}), 500


@app.route("/api/sensors/live")
def live():

    try:
        row = latest_reading()

        if not row:
            raise Exception("Sem dados")

        return jsonify({
            "temp": float(row[0] or 0),
            "rpm": int(row[1] or 0),
            "ir": int(row[2] or 0),
            "status": str(row[3] or "normal")
        })

    except Exception as e:
        logging.error(e)

        return jsonify({
            "temp": 0,
            "rpm": 0,
            "ir": 0,
            "status": "normal"
        })


# ==================================================
# START
# ==================================================

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)