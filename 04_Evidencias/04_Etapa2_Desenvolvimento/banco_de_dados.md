# Banco de Dados - Etapa 2

**Grupo Alpha**  
**Curso:** Técnico em Cibersistemas para Automação  
**Docente:** William Belarmino da Silva  
**Data:** Maio de 2026

---

## 1. Objetivo

Armazenar de forma estruturada e persistente os dados coletados pelo ESP8266 (temperatura, umidade, rotação, etc.), permitindo consulta, histórico e análise futura.

---

## 2. Tecnologias Analisadas

| Tecnologia              | Dificuldade | Persistência | Facilidade de Integração | Recomendado? | Motivo |
|-------------------------|-------------|--------------|--------------------------|--------------|--------|
| Google Sheets           | Baixa       | Boa          | Alta                     | Sim (atual)  | Fácil visualização |
| Firebase Realtime DB    | Média       | Excelente    | Alta                     | **Sim**      | Ideal para IoT |
| MySQL + PHP             | Alta        | Excelente    | Média                    | Futuro       | Mais robusto |
| ThingSpeak              | Baixa       | Boa          | Alta                     | Alternativa  | Bom para testes |

**Decisão do Grupo:**  
Utilizar **Google Sheets** (já implementado) + **Firebase Realtime Database** como banco de dados principal para a próxima fase.

---

## 3. Firebase Realtime Database (Recomendado)

### Vantagens:
- Gratuito para projetos pequenos
- Atualização em tempo real
- Fácil integração com ESP8266
- Suporta JSON nativamente
- Pode ser visualizado no celular/PC

### Como Configurar (Passo a passo):

1. Acessar [console.firebase.google.com](https://console.firebase.google.com)
2. Criar novo projeto
3. Ativar **Realtime Database**
4. Configurar regras (modo teste por enquanto):

```json
{
  "rules": {
    ".read": true,
    ".write": true
  }
}
