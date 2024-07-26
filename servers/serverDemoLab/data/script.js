window.onload = function () {

  var websocket = new WebSocket('ws://' + window.location.hostname + '/ws');

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
            displayFormars: {
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

  websocket.onopen = function (event) {
    console.log("Conectado al WebSocket");
  };

  websocket.onmessage = function (event) {
    console.log("Mensaje recibido: " + event.data);

    // Suponemos que el mensaje es un JSON con temperatura, humedad y distancia
    const data = JSON.parse(event.data);

    // Actualizar los valores en el HTML
    document.getElementById('distancia-value').innerText = data.distancia.toFixed(2);
    document.getElementById('temperatura-value').innerText = data.temperatura.toFixed(2);
    document.getElementById('humedad-value').innerText = data.humedad.toFixed(2);

    // Añadir los nuevos datos al gráfico
    const now = new Date().toLocaleTimeString();
    labels.push(now);
    temperaturaData.push(data.temperatura);
    humedadData.push(data.humedad);

    // Limitar el número de puntos en el gráfico si es necesario
    if (labels.length > 500) {  // Por ejemplo, mantener solo los últimos 20 puntos
      labels.shift();
      temperaturaData.shift();
      humedadData.shift();
    }

    // Actualizar el gráfico
    chart.update();
  };


};

