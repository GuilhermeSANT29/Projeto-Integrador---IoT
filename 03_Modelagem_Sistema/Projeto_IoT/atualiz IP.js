async function updateSensorsFromAPI() {
    try {
        // Atualizado com o IP real da sua máquina Windows
        const response = await fetch('http://10.106.202.20:5000/api/sensors/live');
        const data = await response.json();
        
        document.getElementById('temp-val').textContent = `${data.temp.toFixed(1)}°C`;
        document.getElementById('rot-val').textContent = `${data.rpm} RPM`;
        document.getElementById('wemos-ip').textContent = data.ip; 
        
        let logMensagem = `IP: ${data.ip} | `;
        logMensagem += data.ir ? "Sensor IR: Acionado | " : "Sensor IR: Normal | ";
        logMensagem += data.btn ? "Botão: Pressionado" : "Botão: Solto";
        
        updateStatusCard(data.status, logMensagem);
        addMySQLLog(data); 
        
    } catch (error) {
        console.error('Falha de comunicação com a API SQL Server:', error);
    }
}

function addMySQLLog(data) {
    const tbody = document.getElementById('mysql-logs');
    const tr = document.createElement('tr');
    const horarioStamp = new Date().toLocaleTimeString('pt-BR');
    const cssStatus = data.status === 'critical' ? 'text-red-400 font-bold' : 'text-green-400 font-bold';
    
    tr.innerHTML = `
        <td class="opacity-75">${horarioStamp}</td>
        <td class="font-bold text-blue-400">${data.temp}°C</td>
        <td class="font-bold text-blue-400">${data.hum}%</td>
        <td class="font-bold text-blue-400">${data.rpm}</td>
        <td class="font-medium text-[var(--neon-orange)]">IR: ${data.ir ? '1':'0'} | BTN: ${data.btn ? '1':'0'}</td>
        <td class="text-right ${cssStatus} uppercase font-bold">${data.status}</td>
    `;
    
    const placeholder = document.getElementById('initial-row');
    if(placeholder) placeholder.remove();

    tbody.insertBefore(tr, tbody.firstChild);
    if (tbody.children.length > 15) tbody.removeChild(tbody.lastChild);
}