window.onload = function () {
    var websocket = new WebSocket('ws://' + window.location.hostname + ':81');
    const statusIndicator = document.getElementById('status-indicator');
    const statusText = document.getElementById('status-text');
    const ctx = document.getElementById('chart');

    const MAX_DATA_POINTS = 100;

    const chart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                {
                    label: 'Humedad (%)',
                    data: [],
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
                    data: [],
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
                        maxTicksLimit: 10
                    },
                },
                y: {
                    beginAtZero: true
                }
            }
        }
    });

    function addData(label, temperatura, humedad) {
        if (chart.data.labels.length >= MAX_DATA_POINTS) {
            chart.data.labels.shift();
            chart.data.datasets[0].data.shift();
            chart.data.datasets[1].data.shift();
        }
        chart.data.labels.push(label);
        chart.data.datasets[0].data.push(humedad);
        chart.data.datasets[1].data.push(temperatura);
        chart.update();
    }

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
            const data = JSON.parse(event.data);
            
            document.getElementById('distancia-value').innerText = data.distancia === 9999.0 ? 'OUT RANGE' : data.distancia.toFixed(2) + ' cm';
            document.getElementById('temperatura-value').innerText = data.temperatura.toFixed(2);
            document.getElementById('humedad-value').innerText = data.humedad.toFixed(2);
            
            const now = new Date().toLocaleTimeString();
            addData(now, data.temperatura, data.humedad);
        } catch (error) {
            console.error("Error procesando el mensaje WebSocket: ", error);
        }
    };
};

