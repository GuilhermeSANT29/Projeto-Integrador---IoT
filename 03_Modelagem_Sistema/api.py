# ==================================================
# DASHBOARD CYBERSISTEMAS
# ==================================================
@app.route("/dashboard")
def dashboard():

    # --- Busca a última leitura direto do MySQL ---
    # (já antecipando a Melhoria 2, mas mantendo
    #  o fallback nas globais por segurança agora)
    ultima_temp_exib    = ultima_temperatura
    ultima_umid_exib    = ultima_umidade
    ultimo_estado_exib  = ultimo_estado
    ultima_dh_exib      = ultima_datahora

    status = ultimo_estado_exib.lower()

    if status == "critico":
        statusColor = "#ff003c"
        statusText  = "CRÍTICO"
    elif status == "alerta":
        statusColor = "#ff9500"
        statusText  = "ALERTA"
    else:
        statusColor = "#00ff41"
        statusText  = "NORMAL"

    html = f"""
    <!DOCTYPE html>
    <html lang='pt-br'>
    <head>
      <meta charset='UTF-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <meta http-equiv='refresh' content='5'>
      <title>Cybersistemas IOT Dashboard</title>
      <link href='[fonts.googleapis.com](https://fonts.googleapis.com/css2?family=Fira+Code:wght@400;500;600;700&display=swap)' rel='stylesheet'>

      <style>
        :root {{
          --neon-blue:   #00f2ff;
          --neon-green:  #00ff41;
          --neon-red:    #ff003c;
          --neon-orange: #ff9500;
          --bg-dark:     #0a0a0f;
          --bg-card:     rgba(20, 20, 30, 0.98);
        }}

        *, *::before, *::after {{
          box-sizing: border-box;
          margin: 0;
          padding: 0;
        }}

        body {{
          font-family: 'Fira Code', monospace;
          background: linear-gradient(135deg, #0a0a0f 0%, #1a1a2e 50%, #0a0a0f 100%);
          color: #e5e5e5;
          min-height: 100vh;
        }}

        /* ── HEADER ── */
        .header-main {{
          text-align: center;
          padding: 2rem 1rem;
          border-bottom: 1px solid rgba(0, 255, 65, 0.3);
        }}

        .header-title {{
          font-size: clamp(1.8rem, 6vw, 4rem); /* escala com a tela */
          background: linear-gradient(90deg, var(--neon-blue), var(--neon-green));
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
          background-clip: text;
          font-weight: 700;
          line-height: 1.2;
        }}

        .header-sub {{
          opacity: 0.7;
          margin-top: 0.5rem;
          font-size: clamp(0.6rem, 2vw, 0.85rem);
          padding: 0 1rem;
        }}

        /* ── LAYOUT PRINCIPAL ── */
        main {{
          max-width: 1000px;
          margin: 0 auto;
          padding: 2rem 1rem;
        }}

        /* ── GRID DE CARTÕES ── */
        .cards-grid {{
          display: grid;
          grid-template-columns: 1fr 1fr; /* 2 colunas no desktop */
          gap: 1.5rem;
          margin-bottom: 1.5rem;
        }}

        /* MOBILE: empilha em 1 coluna */
        @media (max-width: 768px) {{
          .cards-grid {{
            grid-template-columns: 1fr;
            gap: 1rem;
          }}
        }}

        /* ── CARTÃO SENSOR ── */
        .sensor-card {{
          background: var(--bg-card);
          border: 2px solid rgba(0, 255, 65, 0.25);
          border-radius: 1.5rem;
          padding: 2rem 1.5rem;
          text-align: center;
        }}

        .card-label {{
          font-size: 0.85rem;
          opacity: 0.65;
          letter-spacing: 0.15em;
          margin-bottom: 1rem;
        }}

        .value-main {{
          font-size: clamp(2.5rem, 10vw, 5rem); /* escala com a tela */
          color: var(--neon-blue);
          font-weight: 700;
          line-height: 1;
        }}

        /* ── CARTÃO DE STATUS ── */
        .status-card {{
          background: var(--bg-card);
          border-radius: 1.5rem;
          padding: 2rem 1.5rem;
          border: 2px solid {statusColor};
          text-align: center;
        }}

        .status-val {{
          font-size: clamp(2rem, 8vw, 3rem);
          color: {statusColor};
          font-weight: 900;
          letter-spacing: 0.1em;
        }}

        .status-msg {{
          margin-top: 1rem;
          opacity: 0.75;
          font-size: clamp(0.7rem, 2vw, 0.9rem);
        }}

        /* ── GRADE DE INFO ── */
        .info-grid {{
          display: grid;
          grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
          gap: 1rem;
          margin-top: 1.5rem;
        }}

        @media (max-width: 480px) {{
          .info-grid {{
            grid-template-columns: 1fr 1fr; /* 2 colunas em telas muito pequenas */
          }}
        }}

        .info-item {{
          background: rgba(0, 255, 65, 0.06);
          border: 1px solid rgba(0, 255, 65, 0.2);
          border-radius: 1rem;
          padding: 1rem 0.75rem;
          text-align: center;
        }}

        .info-label {{
          font-size: 0.65rem;
          opacity: 0.55;
          letter-spacing: 0.1em;
          margin-bottom: 0.5rem;
        }}

        .info-val {{
          color: var(--neon-green);
          font-weight: 700;
          font-size: clamp(0.7rem, 2.5vw, 0.9rem);
          word-break: break-all; /* evita overflow de IP em telas pequenas */
        }}

        /* ── FOOTER ── */
        footer {{
          text-align: center;
          padding: 1.5rem 1rem;
          opacity: 0.4;
          font-size: 0.75rem;
        }}
      </style>
    </head>

    <body>

      <header class='header-main'>
        <h1 class='header-title'>CYBERSISTEMAS IOT</h1>
        <p class='header-sub'>
          Dashboard Profissional &bull; ESP8266 &bull; DHT11 &bull;
          Flask API &bull; MySQL &bull; Google Sheets
        </p>
      </header>

      <main>

        <div class='cards-grid'>

          <div class='sensor-card'>
            <div class='card-label'>TEMPERATURA</div>
            <div class='value-main'>{ultima_temp_exib}°C</div>
          </div>

          <div class='sensor-card'>
            <div class='card-label'>UMIDADE</div>
            <div class='value-main'>{ultima_umid_exib}%</div>
          </div>

        </div>

        <div class='status-card'>

          <div class='status-val'>{statusText}</div>

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
              <div class='info-val'>{ultima_dh_exib}</div>
            </div>

          </div>

        </div>

      </main>

      <footer>CYBERSISTEMAS &copy; 2026</footer>

    </body>
    </html>
    """

    return html
