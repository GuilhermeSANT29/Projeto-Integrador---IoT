import gspread
from google.oauth2.service_account import Credentials

# ========================================================
# CONFIGURAÇÕES DE CAMINHO E PERMISSÕES
# ========================================================
# O 'r' antes das aspas diz ao Python para aceitar as barras invertidas (\) do Windows
CAMINHO_JSON = r"C:\Users\Aluno\Desktop\Dev\aula\credentials.json"

ESCOPOS = [
    "https://www.googleapis.com/auth/spreadsheets",
    "https://www.googleapis.com/auth/drive"
]

# ========================================================
# AUTENTICAÇÃO E CONEXÃO
# ========================================================
# Usando o caminho absoluto que você definiu acima
credenciais = Credentials.from_service_account_file(
    CAMINHO_JSON, 
    scopes=ESCOPOS
)

cliente = gspread.authorize(credenciais)

# Abrir a planilha pelo link
url_da_sua_planilha = "https://docs.google.com/spreadsheets/d/1i-JjwKm-nMN8rGkFIAg583FaO3pLqIIdhfsMm76fW6k/edit?usp=sharing"
planilha = cliente.open_by_url(url_da_sua_planilha)

# Selecionar a aba pelo nome exato
aba = planilha.worksheet("Historico_Geral")

print("Google Sheets conectado com sucesso!")

# --- EXEMPLO: ESCREVER DADOS ---
aba.update_acell('A1', 'Dado enviado pelo Python!')
print("Célula A1 atualizada!")