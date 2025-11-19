/**
 * Sensor Monitoring Dashboard
 * JavaScript module for managing real-time sensor data display
 */

// ============================================
// Configuration
// ============================================
const CONFIG = {
  websocketUrl: null, // Will be auto-detected from current host
  reconnectInterval: 3000 // Si no se pudo conectar vuelve a intentar en 3 segs
};

// ============================================
// Sensor Data Structure
// ============================================
const sensorData = {
  left: {
    temperature: {
      heel: 0,
      side: 0,
      hallux: 0,
      met1: 0,
      met5: 0
    },
    pressure: {
      heel: 0,
      met1: 0,
      met5: 0,
      mid: 0
    }
  },
  right: {
    temperature: {
      heel: 0,
      side: 0,
      hallux: 0,
      met1: 0,
      met5: 0
    },
    pressure: {
      heel: 0,
      met1: 0,
      met5: 0,
      mid: 0
    }
  }
};

// Sensor location labels
const sensorLabels = {
  heel: 'Talón',
  side: 'Lateral Externo',
  hallux: 'Hallux (Dedo Gordo)',
  met1: '1er Metatarsiano',
  met5: '5to Metatarsiano',
  mid: 'Zona Central'
};

// ============================================
// WebSocket Management
// ============================================
let websocket = null;
let reconnectTimeout = null;
let isReconnecting = false;

// ============================================
// Initialization
// ============================================
document.addEventListener('DOMContentLoaded', () => {
  console.log('Sensor Dashboard Initialized');
  
  // Initialize display
  updateDisplay();
  
  // Setup event listeners
  setupEventListeners();
  
  // Connect to WebSocket
  connectWebSocket();
  
  // Botones
  document.getElementById("guardar-btn")?.addEventListener("click", guardarDatos);
  document.getElementById("descargar-btn")?.addEventListener("click", descargarCSV);

  // Apply fade-in animation
  document.querySelectorAll('.summary-card, .foot-card, .data-table-container').forEach((el, index) => {
    setTimeout(() => {
      el.classList.add('fade-in');
    }, index * 100);
  });
});

// ============================================
// Event Listeners
// ============================================
function setupEventListeners() {
  const refreshBtn = document.getElementById('refresh-btn');
  
  refreshBtn.addEventListener('click', () => {
    refreshBtn.classList.add('rotating');
    
    // Reconnect WebSocket if disconnected
    if (!websocket || websocket.readyState !== WebSocket.OPEN) {
      connectWebSocket();
    }
    
    setTimeout(() => {
      refreshBtn.classList.remove('rotating');
    }, 1000);
  });
}

// ============================================
// WebSocket Functions
// ============================================

/**
 * Connect to WebSocket server
 */
function connectWebSocket() {
  // Auto-detect WebSocket URL from current host
  const wsProtocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
  const wsHost = window.location.hostname;
  const wsPort = window.location.port || '80';
  const wsUrl = CONFIG.websocketUrl || `${wsProtocol}//${wsHost}/ws`;
  
  console.log(`Connecting to WebSocket: ${wsUrl}`);
  
  try {
    websocket = new WebSocket(wsUrl);
    
    websocket.onopen = onWebSocketOpen;
    websocket.onmessage = onWebSocketMessage;
    websocket.onerror = onWebSocketError;
    websocket.onclose = onWebSocketClose;
    
  } catch (error) {
    console.error('WebSocket connection error:', error);
    updateConnectionStatus(false);
    scheduleReconnect();
  }
}

/**
 * Handle WebSocket open event
 */
function onWebSocketOpen(event) {
  console.log('WebSocket connected successfully');
  updateConnectionStatus(true);
  isReconnecting = false;
  
  // Clear any pending reconnect attempts
  if (reconnectTimeout) {
    clearTimeout(reconnectTimeout);
    reconnectTimeout = null;
  }
}

/**
 * Handle incoming WebSocket messages
 */
function onWebSocketMessage(event) {
  try {
    const data = JSON.parse(event.data);
    console.log('Received sensor data:', data);
    
    // Update sensor data
    if (data.left) {
      if (data.left.temperature) {
        Object.assign(sensorData.left.temperature, data.left.temperature);
      }
      if (data.left.pressure) {
        Object.assign(sensorData.left.pressure, data.left.pressure);
      }
    }
    
    if (data.right) {
      if (data.right.temperature) {
        Object.assign(sensorData.right.temperature, data.right.temperature);
      }
      if (data.right.pressure) {
        Object.assign(sensorData.right.pressure, data.right.pressure);
      }
    }
    
    // Update display
    updateDisplay();
    
  } catch (error) {
    console.error('Error parsing WebSocket message:', error);
  }
}

/**
 * Handle WebSocket errors
 */
function onWebSocketError(event) {
  console.error('WebSocket error:', event);
  updateConnectionStatus(false);
}

/**
 * Handle WebSocket close event
 */
function onWebSocketClose(event) {
  console.log('WebSocket closed:', event.code, event.reason);
  updateConnectionStatus(false);
  
  // Attempt to reconnect if not intentionally closed
  if (!isReconnecting) {
    scheduleReconnect();
  }
}

/**
 * Schedule a reconnection attempt
 */
function scheduleReconnect() {
  if (isReconnecting) return;
  
  isReconnecting = true;
  console.log(`Reconnecting in ${CONFIG.reconnectInterval / 1000} seconds...`);
  
  reconnectTimeout = setTimeout(() => {
    console.log('Attempting to reconnect...');
    connectWebSocket();
  }, CONFIG.reconnectInterval);
}

// ============================================
// Data Management
// ============================================

/**
 * Refresh all sensor data
 */
function refreshData() {
  // Reconnect WebSocket if disconnected
  if (!websocket || websocket.readyState !== WebSocket.OPEN) {
    connectWebSocket();
  }
  updateDisplay();
}

// ============================================
// Display Updates
// ============================================

/**
 * Update all display elements
 */
function updateDisplay() {
  updateSensorIndicators();
  updateSummaryStats();
  updateDataTable();
  updateTimestamp();
}

/**
 * Update sensor indicators on feet
 */
function updateSensorIndicators() {
  // Update left foot
  ['left', 'right'].forEach(side => {
    // Temperature sensors
    Object.keys(sensorData[side].temperature).forEach(location => {
      const element = document.querySelector(`#${side}-temp-${location} .sensor-value`);
      if (element) {
        element.textContent = sensorData[side].temperature[location];
      }
    });
    
    // Pressure sensors
    Object.keys(sensorData[side].pressure).forEach(location => {
      const element = document.querySelector(`#${side}-press-${location} .sensor-value`);
      if (element) {
        element.textContent = sensorData[side].pressure[location];
      }
    });
  });
}

/**
 * Calculate and update summary statistics
 */
function updateSummaryStats() {
  // Collect all temperature values
  const allTemps = [];
  ['left', 'right'].forEach(side => {
    Object.values(sensorData[side].temperature).forEach(val => {
      allTemps.push(parseFloat(val));
    });
  });
  
  // Collect all pressure values
  const allPress = [];
  ['left', 'right'].forEach(side => {
    Object.values(sensorData[side].pressure).forEach(val => {
      allPress.push(parseFloat(val));
    });
  });
  
  // Calculate temperature stats
  const avgTemp = (allTemps.reduce((a, b) => a + b, 0) / allTemps.length).toFixed(1);
  const maxTemp = Math.max(...allTemps).toFixed(1);
  const minTemp = Math.min(...allTemps).toFixed(1);
  
  // Calculate pressure stats
  const avgPress = (allPress.reduce((a, b) => a + b, 0) / allPress.length).toFixed(1);
  const maxPress = Math.max(...allPress).toFixed(1);
  const minPress = Math.min(...allPress).toFixed(1);
  
  // Update display
  document.getElementById('avg-temp').textContent = `${avgTemp}°C`;
  document.getElementById('max-temp').textContent = `${maxTemp}°C`;
  document.getElementById('min-temp').textContent = `${minTemp}°C`;
  
  document.getElementById('avg-press').textContent = `${avgPress} N`;
  document.getElementById('max-press').textContent = `${maxPress} N`;
  document.getElementById('min-press').textContent = `${minPress} N`;
}

/**
 * Update data table with detailed sensor information
 */
function updateDataTable() {
  const tableBody = document.getElementById('sensor-data-table');
  tableBody.innerHTML = '';
  
  // Temperature rows
  const tempLocations = ['heel', 'side', 'hallux', 'met1', 'met5'];
  tempLocations.forEach(location => {
    if (sensorData.left.temperature[location] !== undefined) {
      const leftTemp = sensorData.left.temperature[location];
      const rightTemp = sensorData.right.temperature[location];
      const diff = Math.abs(leftTemp - rightTemp).toFixed(1);
      const isCritical = diff > 2.2;
      const diffClass = isCritical ? 'value-critical' : 'value-normal';
      const diffDisplay = `${diff} °C${isCritical ? ' ⚠️' : ''}`;

      const row = createTableRow(
        sensorLabels[location] || location,
        'Temperatura',
        `${leftTemp} °C`,
        `${rightTemp} °C`,
        `<span class="${diffClass}">${diffDisplay}</span>`,
        'temp'
      );
      tableBody.appendChild(row);
    }
  });
  
  // Pressure rows
  const pressLocations = ['heel', 'met1', 'met5', 'mid'];
  pressLocations.forEach(location => {
    if (sensorData.left.pressure[location] !== undefined) {
      const leftPress = sensorData.left.pressure[location];
      const rightPress = sensorData.right.pressure[location];
      const leftCritical = leftPress > 700;
      const rightCritical = rightPress > 700;
      const leftClass = leftCritical ? 'value-critical' : 'value-normal';
      const rightClass = rightCritical ? 'value-critical' : 'value-normal';
      
      const leftDisplay = `${leftPress} N${leftCritical ? ' ⚠️' : ''}`;
      const rightDisplay = `${rightPress} N${rightCritical ? ' ⚠️' : ''}`;
      
      
      const row = createTableRow(
        sensorLabels[location] || location,
        'Presión',
        `<span class="${leftClass}">${leftDisplay}</span>`,
        `<span class="${rightClass}">${rightDisplay}</span>`,
        '--',
        'press'
      );
      tableBody.appendChild(row);
    }
  });
}

function createTableRow(location, type, leftValue, rightValue, difference, badgeType) {
  const row = document.createElement('tr');
  row.innerHTML = `
    <td><strong>${location}</strong></td>
    <td><span class="sensor-type-badge badge-${badgeType}">${type}</span></td>
    <td>${leftValue}</td>
    <td>${rightValue}</td>
    <td>${difference}</td>
  `;
  return row;
}


/**
 * Update connection status indicator
 */
function updateConnectionStatus(connected) {
  const statusElement = document.getElementById('connection-status');
  if (connected) {
    statusElement.textContent = 'Conectado';
    statusElement.className = 'status-value connected';
  } else {
    statusElement.textContent = 'Desconectado';
    statusElement.className = 'status-value disconnected';
  }
}

/**
 * Update timestamp of last update
 */
function updateTimestamp() {
  const now = new Date();
  const timeString = now.toLocaleTimeString('es-AR', { 
    hour: '2-digit', 
    minute: '2-digit', 
    second: '2-digit' 
  });
  document.getElementById('last-update').textContent = timeString;
}

// ============================================
// Public API
// ============================================

/**
 * Update sensor data programmatically
 * @param {Object} newData - New sensor data object
 */
function updateSensorData(newData) {
  if (newData.left) {
    if (newData.left.temperature) {
      Object.assign(sensorData.left.temperature, newData.left.temperature);
    }
    if (newData.left.pressure) {
      Object.assign(sensorData.left.pressure, newData.left.pressure);
    }
  }
  
  if (newData.right) {
    if (newData.right.temperature) {
      Object.assign(sensorData.right.temperature, newData.right.temperature);
    }
    if (newData.right.pressure) {
      Object.assign(sensorData.right.pressure, newData.right.pressure);
    }
  }
  
  updateDisplay();
}

/**
 * Get current sensor data
 * @returns {Object} Current sensor data
 */
function getSensorData() {
  return JSON.parse(JSON.stringify(sensorData));
}

// Export functions for external use
window.SensorDashboard = {
  updateSensorData,
  getSensorData,
  refreshData,
  connectWebSocket,
  CONFIG
};

function guardarDatos() {
    const fecha = new Date().toLocaleString();

    // Recolectar datos de sensores
    const datos = {
        fecha: fecha,
        temperatura: { izquierda: {}, derecha: {} },
        presion: { izquierda: {}, derecha: {} }
    };

    // Sensores de temperatura
    ["heel", "side", "hallux", "met1", "met5"].forEach(loc => {
        datos.temperatura.izquierda[loc] = document.querySelector(`#left-temp-${loc} .sensor-value`)?.textContent || "";
        datos.temperatura.derecha[loc] = document.querySelector(`#right-temp-${loc} .sensor-value`)?.textContent || "";
    });

    // Sensores de presión
    ["heel", "met1", "met5", "mid"].forEach(loc => {
        datos.presion.izquierda[loc] = document.querySelector(`#left-press-${loc} .sensor-value`)?.textContent || "";
        datos.presion.derecha[loc] = document.querySelector(`#right-press-${loc} .sensor-value`)?.textContent || "";
    });

    // Guardar en historial de localStorage
    let historial = JSON.parse(localStorage.getItem("historialSensores")) || [];
    historial.push(datos);
    localStorage.setItem("historialSensores", JSON.stringify(historial));

    // Aviso visual
    alert("✅ Datos guardados correctamente");
}
function descargarCSV() {
    const historial = JSON.parse(localStorage.getItem("historialSensores")) || [];
    if (historial.length === 0) {
        alert("⚠️ No hay datos para descargar");
        return;
    }

    // Sensores a incluir
    const tempKeys = ["heel", "side", "hallux", "met1", "met5"];
    const pressKeys = ["heel", "met1", "met5", "mid"];

    // Etiquetas en español
    const labels = {
        heel: "Talón",
        side: "Lateral Externo",
        hallux: "Hallux (Dedo Gordo)",
        met1: "1er Metatarsiano",
        met5: "5to Metatarsiano",
        mid: "Zona Central"
    };

    let csv = "";

    historial.forEach(entry => {
        // --- Temperatura ---
        csv += `Fecha: ${entry.fecha}\n`;
        csv += "Sensores Temperatura," + tempKeys.map(k => labels[k]).join(",") + "\n";
        csv += "Pie Izquierdo," + tempKeys.map(k => entry.temperatura.izquierda[k] || "").join(",") + "\n";
        csv += "Pie Derecho," + tempKeys.map(k => entry.temperatura.derecha[k] || "").join(",") + "\n\n";

        // --- Presión ---
        csv += "Sensores Presión," + pressKeys.map(k => labels[k]).join(",") + "\n";
        csv += "Pie Izquierdo," + pressKeys.map(k => entry.presion.izquierda[k] || "").join(",") + "\n";
        csv += "Pie Derecho," + pressKeys.map(k => entry.presion.derecha[k] || "").join(",") + "\n\n";
    });

    const blob = new Blob([csv], { type: "text/csv;charset=utf-8;" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = `historial_sensores_${new Date().toISOString()}.csv`;
    a.click();
    URL.revokeObjectURL(url);
}



