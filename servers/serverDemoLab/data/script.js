window.onload = function () {
    var websocket = new WebSocket('ws://' + window.location.hostname + ':81');
    const statusIndicator = document.getElementById('status-indicator');
    const statusText = document.getElementById('status-text');
    const ctx = document.getElementById('chart');
    
    // Variables para el gráfico
    const labels = [];
    const temperaturaData = [];
    const humedadData = [];
    const chart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [
                {
                    label: 'Humedad (%)',
                    data: humedadData,
                    borderWidth: 1.2,
                    fill: false,
                    tension: 0,
                    pointRadius: 0.5,
                    pointHoverRadius: 7,
                    borderColor: "rgba(0, 129, 255, 1)",
                    backgroundColor: "rgba(0, 129, 255, 0.2)"
                },
                {
                    label: 'Temperatura (°C)',
                    data: temperaturaData,
                    borderWidth: 1.2,
                    fill: false,
                    tension: 0,
                    pointRadius: 0.5,
                    pointHoverRadius: 7,
                    borderColor: "rgba(255, 99, 132, 1)",
                    backgroundColor: "rgba(255, 99, 132, 0.2)"
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                x: {
                    type: 'category',
                    time: {
                        displayFormats: {
                            second: 'HH:mm:ss'
                        }
                    },
                    ticks: {
                    },
                },
                y: {
                    beginAtZero: true
                }
            }
        }
    });

    function updateConnectionStatus(connected) {
        if (connected) {
            statusIndicator.classList.add('connected');
            statusText.textContent = 'Conectado';
        } else {
            statusIndicator.classList.remove('connected');
            statusText.textContent = 'Desconectado';
        }
    }

    websocket.onopen = function (event) {
        console.log("Conectado al WebSocket");
        updateConnectionStatus(true);
    };

    websocket.onclose = function (event) {
        console.log("Desconectado del WebSocket");
        updateConnectionStatus(false);
    };

    websocket.onerror = function (error) {
        console.error('Error en la conexión WebSocket:', error);
        updateConnectionStatus(false);
    };

    websocket.onmessage = function (event) {
        try {
            console.log("Mensaje recibido: " + event.data);
            // Suponemos que el mensaje es un JSON con temperatura, humedad y distancia
            const data = JSON.parse(event.data);
            // Actualizar los valores en el HTML
            document.getElementById('distancia-value').innerText = data.distancia === 9999.0 ? 'OUT RANGE' : data.distancia.toFixed(2) + ' cm';
            document.getElementById('temperatura-value').innerText = data.temperatura.toFixed(2);
            document.getElementById('humedad-value').innerText = data.humedad.toFixed(2);
            // Añadir los nuevos datos al gráfico
            const now = new Date().toLocaleTimeString();
            labels.push(now);
            temperaturaData.push(data.temperatura);
            humedadData.push(data.humedad);
            // Limitar el número de puntos en el gráfico si es necesario
            if (labels.length > 100) {  // Por ejemplo, mantener solo los últimos 100 puntos
                labels.shift();
                temperaturaData.shift();
                humedadData.shift();
            }
            // Actualizar el gráfico
            chart.update();
        } catch (error) {
            console.error("Error procesando el mensaje WebSocket: ", error);
        }
    };
};
