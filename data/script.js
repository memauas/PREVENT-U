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
const TEMP_MIN = 20; // azul
const TEMP_MAX = 40; // rojo
const PRESS_MIN = 0; // Azul
const PRESS_MAX = 200; // Rojo

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
 hallux: 0,
 heel: 0,
 met1: 0,
 met5: 0,
 side: 0
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
 hallux: 0,
 heel: 0,
 met1: 0,
 met5: 0,
 side: 0
 }
 }
};

// Sensor location labels
const sensorLabels = {
 heel: 'Talón',
 hallux: 'Hallux (Dedo Gordo)',
 met1: '1er Metatarsiano',
 met5: '5to Metatarsiano',
 side: 'Zona Lateral'
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
 document.getElementById("tendencias-btn").addEventListener("click", () => {
 mostrarTendencias();
 const modal = new bootstrap.Modal(document.getElementById("tendenciasModal"));
 modal.show();
});
 document.getElementById("informe-btn")?.addEventListener("click", exportInforme);



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


function getThermoColor(value) {
 const stepped = Math.round(value / 2) * 2;

 if (stepped <= 20) return "rgb(40, 60, 170)"; // Azul oscuro
 if (stepped <= 26) return "rgb(60, 80, 200)"; // Azul medio oscuro
 if (stepped <= 30) return "rgb(80, 120, 220)"; 
 if (value <= 35) return "rgb(25, 103, 79)"; // Azul-celeste oscuro
 if (stepped <= 37) return "rgb(60, 160, 130)"; // Verde agua oscuro
 if (stepped <= 38) return "rgb(200, 200, 60)"; // Amarillo oscuro
 if (stepped <= 39) return "rgb(220, 120, 40)"; // Naranja oscuro
 return "rgb(180, 40, 40)"; // Rojo oscuro
}



function updateTempColor(elementId, value) {
 const box = document.getElementById(elementId);
 if (!box) return;

 const color = getThermoColor(value);
 box.style.backgroundColor = color;
 box.style.borderColor = "rgba(0,0,0,0.3)";
}

function getPressureColor(value) {
 if (value <= 5) return "rgb(40, 60, 170)"; // Azul oscuro
 if (value <= 10) return "rgb(60, 80, 200)"; // Azul medio
 if (value <= 20) return "rgb(80, 120, 220)"; // Azul celeste
 if (value <= 40) return "rgb(25, 103, 79)"; // Verde agua oscuro
 if (value <= 120) return "rgb(60, 160, 130)"; // Verde agua claro
 if (value <= 140) return "rgb(200, 200, 60)"; // Amarillo
 if (value <= 170) return "rgb(220, 120, 40)"; // Naranja
 return "rgb(180, 40, 40)"; // Rojo
}


function updatePressColor(elementId, value) {
 const box = document.getElementById(elementId);
 if (!box) return;

 const color = getPressureColor(value);
 box.style.backgroundColor = color;
 box.style.borderColor = "rgba(0,0,0,0.3)";
}


/**function getPressureColor(value) {
 return "#780518"; // Color fijo pastel
}

function updatePressColor(elementId, value) {
 const box = document.getElementById(elementId);
 if (!box) return;

 const color = getPressureColor(value);
 box.style.backgroundColor = color;
 box.style.borderColor = "rgba(0,0,0,0.3)";
}
*/
function updateSensorIndicators() {

 ['left', 'right'].forEach(side => {

 // ---------------- TEMPERATURA ----------------
 Object.keys(sensorData[side].temperature).forEach(location => {
 const value = sensorData[side].temperature[location];

 // Actualizar número
 const element = document.querySelector(`#${side}-temp-${location} .sensor-value`);
 if (element) element.textContent = value;

 // Pintar color
 updateTempColor(`${side}-temp-${location}`, value);
 });

 // ---------------- PRESIÓN ----------------
 Object.keys(sensorData[side].pressure).forEach(location => {
 const value = sensorData[side].pressure[location];

 // Actualizar número
 const element = document.querySelector(`#${side}-press-${location} .sensor-value`);
 if (element) element.textContent = value;

 // Pintar color presión
 updatePressColor(`${side}-press-${location}`, value);
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
 
 document.getElementById('avg-press').textContent = `${avgPress} KPa`;
 document.getElementById('max-press').textContent = `${maxPress} KPa`;
 document.getElementById('min-press').textContent = `${minPress} KPa`;
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
 const pressLocations = ['hallux', 'heel', 'met1', 'met5', 'side'];
 pressLocations.forEach(location => {
 if (sensorData.left.pressure[location] !== undefined) {
 const leftPress = sensorData.left.pressure[location];
 const rightPress = sensorData.right.pressure[location];
 const leftCritical = leftPress > 200;
 const rightCritical = rightPress > 200;
 const leftClass = leftCritical ? 'value-critical' : 'value-normal';
 const rightClass = rightCritical ? 'value-critical' : 'value-normal';
 
 const leftDisplay = `${leftPress} kPa${leftCritical ? ' ⚠️' : ''}`;
 const rightDisplay = `${rightPress} kPa${rightCritical ? ' ⚠️' : ''}`;
 
 
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
 ["hallux","heel", "met1", "met5", "side"].forEach(loc => {
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

 const tempKeys = ["heel", "side", "hallux", "met1", "met5"];
 const pressKeys = ["hallux", "heel", "met1", "met5", "side"];

 const labels = {
 heel: "Talón",
 side: "Lateral Externo",
 hallux: "Hallux (Dedo Gordo)",
 met1: "1er Metatarsiano",
 met5: "5to Metatarsiano"
 };

 // Evita romper columnas por comas internas
 const wrap = v => `"${(v ?? "").toString().replace(/"/g, '""')}"`;

 let csv = "";

 historial.forEach(entry => {
 csv += `Fecha:,${wrap(entry.fecha)}\n`;

 // --- Temperatura ---
 csv += "Sensores Temperatura," + tempKeys.map(k => wrap(labels[k])).join(",") + "\n";
 csv += "Pie Izquierdo," + tempKeys.map(k => wrap(entry.temperatura.izquierda[k])).join(",") + "\n";
 csv += "Pie Derecho," + tempKeys.map(k => wrap(entry.temperatura.derecha[k])).join(",") + "\n\n";

 // --- Presión ---
 csv += "Sensores Presión," + pressKeys.map(k => wrap(labels[k])).join(",") + "\n";
 csv += "Pie Izquierdo," + pressKeys.map(k => wrap(entry.presion.izquierda[k])).join(",") + "\n";
 csv += "Pie Derecho," + pressKeys.map(k => wrap(entry.presion.derecha[k])).join(",") + "\n\n";
 });

 const blob = new Blob([csv], { type: "text/csv;charset=utf-8;" });
 const url = URL.createObjectURL(blob);
 const a = document.createElement("a");
 a.href = url;
 a.download = `historial_sensores_${new Date().toISOString()}.csv`;
 a.click();
 URL.revokeObjectURL(url);
}


function mostrarTendencias() {
 const historial = JSON.parse(localStorage.getItem("historialSensores")) || [];

 const chartsContainer = document.getElementById("charts-container");
 chartsContainer.innerHTML = "";

 if (historial.length === 0) {
 chartsContainer.innerHTML = "<p>No hay datos guardados.</p>";
 return;
 }

 const tiempo = historial.map(r => r.fecha);

 const sensores = [
 // Temperatura
 { side: "izquierda", type: "temperatura", id: "heel", label: "Temp Talón Izq" },
 { side: "izquierda", type: "temperatura", id: "side", label: "Temp Lateral Izq" },
 { side: "izquierda", type: "temperatura", id: "hallux", label: "Temp Hallux Izq" },
 { side: "izquierda", type: "temperatura", id: "met1", label: "Temp Met1 Izq" },
 { side: "izquierda", type: "temperatura", id: "met5", label: "Temp Met5 Izq" },

 { side: "derecha", type: "temperatura", id: "heel", label: "Temp Talón Der" },
 { side: "derecha", type: "temperatura", id: "side", label: "Temp Lateral Der" },
 { side: "derecha", type: "temperatura", id: "hallux", label: "Temp Hallux Der" },
 { side: "derecha", type: "temperatura", id: "met1", label: "Temp Met1 Der" },
 { side: "derecha", type: "temperatura", id: "met5", label: "Temp Met5 Der" },

 // Presión
 { side: "izquierda", type: "presion", id: "hallux", label: "Presión Hallux Izq" },
 { side: "izquierda", type: "presion", id: "heel", label: "Presión Talón Izq" },
 { side: "izquierda", type: "presion", id: "met1", label: "Presión Met1 Izq" },
 { side: "izquierda", type: "presion", id: "met5", label: "Presión Met5 Izq" },
 { side: "izquierda", type: "presion", id: "side", label: "Presión Lateral Izq" },
 
 { side: "derecha", type: "presion", id: "hallux", label: "Presión Hallux Der" },
 { side: "derecha", type: "presion", id: "heel", label: "Presión Talón Der" },
 { side: "derecha", type: "presion", id: "met1", label: "Presión Met1 Der" },
 { side: "derecha", type: "presion", id: "met5", label: "Presión Met5 Der" },
 { side: "derecha", type: "presion", id: "side", label: "Presión Lateral Der" },
 ];

 sensores.forEach(s => {
 const values = historial.map(r => {
 const raw = r[s.type][s.side][s.id];
 const num = parseFloat(raw.replace("°C", "").trim());
 return isNaN(num) ? null : num;
 });

 const wrapper = document.createElement("div");
 wrapper.classList.add("mb-4");
 wrapper.innerHTML = `
 <h5>${s.label}</h5>
 <canvas id="chart_${s.side}_${s.type}_${s.id}" height="100"></canvas>
 `;
 chartsContainer.appendChild(wrapper);

 const ctx = document.getElementById(`chart_${s.side}_${s.type}_${s.id}`);

 new Chart(ctx, {
 type: 'line',
 data: {
 labels: tiempo,
 datasets: [{
 label: s.label,
 data: values,
 borderWidth: 2,
 tension: 0.3
 }]
 }
 });
 });
}

function enviarInformePorMail(pdfBase64) {
 fetch("https://spring-rice-5628.memauas.workers.dev", {
 method: "POST",
 headers: { "Content-Type": "application/json" },
 body: JSON.stringify({
 to_email: "memauas@itba.edu.ar",
 subject: "Informe PREVENT-U",
 message: "Adjunto el informe generado automáticamente desde PREVENT-U.",
 pdfBase64: pdfBase64
 })
 })
 .then(res => res.json())
 .then(res => {
 console.log("✅ Correo enviado", res);
 alert("📧 Informe enviado por mail correctamente.");
 })
 .catch(err => {
 console.error("❌ Error enviando el correo", err);
 alert("⚠️ Error al enviar el informe por mail.");
 });
}


async function exportInforme() {
 const historial = JSON.parse(localStorage.getItem("historialSensores")) || [];
 const sensoresLista = ["heel", "side", "hallux", "met1", "met5"];
 const labels = {
 heel: "Talón",
 side: "Lateral",
 hallux: "Hallux",
 met1: "1er Met",
 met5: "5to Met"
 };
 function normalizeValue(v) {
 if (v === undefined || v === null) return NaN;

 // Aceptar strings tipo "25 kg", "32°C", " 45 "
 const n = parseFloat(String(v).replace(/[^\d.-]/g, ""));

 return isNaN(n) ? NaN : n;
 }


 function extraerSerie(lado, sensor, tipo) {
 return historial.map(h => normalizeValue(h[tipo][lado][sensor]));
 }

 function calcStats(arr) {
 // arr puede contener NaN o números; filtramos
 const valid = arr.filter(n => typeof n === "number" && !isNaN(n));
 if (valid.length === 0) return { min: NaN, max: NaN, prom: NaN };

 const min = Math.min(...valid);
 const max = Math.max(...valid);
 const prom = valid.reduce((a, b) => a + b, 0) / valid.length;
 return { min, max, prom: Number(prom.toFixed(2)) };

 }
 /**const extraerSerie = (lado, sensor, tipo) =>
 historial.map(h => normalizeValue(h[tipo][lado][sensor]));

 function calcStats(arr) {
 const valid = arr.filter(n => !isNaN(n));
 if (valid.length === 0) return { min: "-", max: "-", prom: "-" };

 const min = Math.min(...valid);
 const max = Math.max(...valid);
 const prom = valid.reduce((a, b) => a + b, 0) / valid.length;
 return { min, max, prom: prom.toFixed(2) };
 }*/


 if (historial.length === 0) {
 alert("⚠️ No hay datos suficientes para el informe.");
 return;
 }

 // --- CREAR PDF ---
 const { jsPDF } = window.jspdf;
 const pdf = new jsPDF();

 // =============================
 // PORTADA
 // =============================
 try {
 const logoImg = await loadImageToBase64("logo.png");

 // Logo grande centrado
 pdf.addImage(logoImg, "PNG", 40, 20, 130, 130);

 } catch (e) {
 console.warn("⚠️ No se pudo cargar el logo en portada.");
 }

 pdf.setFontSize(26);
 pdf.text("Informe de Sensores PREVENT-U", 35, 170);
// --- FECHAS DEL PERIODO ---
 const primeraFecha = historial[0].fecha;
 const ultimaFecha = historial[historial.length - 1].fecha;
 pdf.setFontSize(14);
 pdf.text(`Período analizado:`, 15, 190);
 pdf.text(`Inicio: ${primeraFecha}`, 15, 198);
 pdf.text(`Fin: ${ultimaFecha}`, 15, 206);

// ---------------------------------------------------------
// SECCIÓN ULTIMA MEDICIÓN
// ---------------------------------------------------------

 pdf.addPage();
 pdf.setFontSize(18);
 pdf.text("Última Medición", 10, 20);
 pdf.setFontSize(12);
 const ultima = historial[historial.length - 1];
 pdf.text(`Fecha: ${ultima.fecha}`, 10, 28);

 

 const sensores = ["heel", "side", "hallux", "met1", "met5"];

 // Generar tabla con T° y Presión de los 5 sensores por pie
 const tablaUltima = sensores.map(s => [
 labels[s],
 ultima.temperatura.izquierda[s] || "0",
 ultima.temperatura.derecha[s] || "0",
 ultima.presion.izquierda[s] || "0",
 ultima.presion.derecha[s] || "0"
 ]);

 pdf.autoTable({
 head: [["Sensor", "Temp IZQ", "Temp DER", "Pres IZQ", "Pres DER"]],
 body: tablaUltima,
 startY: 30
 });

 // ---------------------------------------------------------
 // SECCION ANALISIS ESTADISTICO
 // ---------------------------------------------------------

 pdf.addPage();
 pdf.setFontSize(18);
 pdf.text("Análisis Estadístico", 10, 20);

 const sensoresTemp = ["heel", "side", "hallux", "met1", "met5"];
 const sensoresPress = ["heel", "side", "hallux", "met1", "met5"];

 
 // --- TABLA 1: TEMPERATURA ---
 pdf.setFontSize(16);
 pdf.text("Temperatura", 10, 30);

 const tablaTemp = sensoresTemp.map(s => {
 const izq = calcStats(extraerSerie("izquierda", s, "temperatura"));
 const der = calcStats(extraerSerie("derecha", s, "temperatura"));
 return [
 `${labels[s]}`,
 izq.min, izq.max, izq.prom,
 der.min, der.max, der.prom
 ];
 });

 // Render de primera tabla y guardamos la posición Y final
 const tempTable = pdf.autoTable({
 head: [["Sensor", "Min IZQ", "Max IZQ", "Prom IZQ", "Min DER", "Max DER", "Prom DER"]],
 body: tablaTemp,
 startY: 38,
 margin: { bottom: 10 }
 });

 let yAfterTemp = tempTable.lastAutoTable.finalY + 10;

 // --- TABLA 2: PRESIÓN ---
 pdf.setFontSize(16);
 pdf.text("Presión", 10, yAfterTemp);

 const tablaPress = sensoresPress.map(s => {
 const izq = calcStats(extraerSerie("izquierda", s, "presion"));
 const der = calcStats(extraerSerie("derecha", s, "presion"));
 return [
 `${labels[s]}`,
 izq.min, izq.max, izq.prom,
 der.min, der.max, der.prom
 ];
 });

 pdf.autoTable({
 head: [["Sensor", "Min IZQ", "Max IZQ", "Prom IZQ", "Min DER", "Max DER", "Prom DER"]],
 body: tablaPress,
 startY: yAfterTemp + 10
 });


 // ---------------------------------------------------------
 // SECCIÓN DE GRÁFICOS (Temperatura y Presión)
 // ---------------------------------------------------------
 pdf.addPage();
 pdf.setFontSize(20);
 pdf.text("Tendencias de Temperatura y Presión", 10, 20);

 const fechas = historial.map(h => h.fecha);

 const etiquetasSensores = {
 heel: "Talón",
 side: "Lateral",
 hallux: "Hallux",
 met1: "1er Met",
 met5: "5to Met"
 };

 for (const s of sensoresLista) {

 // Crear NUEVA PÁGINA para cada sensor (2 gráficos)
 pdf.addPage();
 pdf.setFontSize(18);
 pdf.text(`${etiquetasSensores[s]}`, 10, 20);

 // ------------------ TEMPERATURA ------------------
 const tempIzq = historial.map(h => normalizeValue(h.temperatura.izquierda[s] || 0));
 const tempDer = historial.map(h => normalizeValue(h.temperatura.derecha[s] || 0));

 const imgTemp = await createChartImage(
 fechas,
 tempIzq,
 tempDer,
 `Temperatura - ${etiquetasSensores[s]}`,
 "Temp Izquierda",
 "Temp Derecha"
 );

 // Temperatura arriba
 pdf.setFontSize(14);
 pdf.text(`Temperatura`, 10, 32);
 pdf.addImage(imgTemp, "PNG", 10, 40, 180, 70);

 // ------------------ PRESIÓN ------------------
 const presIzq = historial.map(h => normalizeValue(h.presion.izquierda[s] || 0));
 const presDer = historial.map(h => normalizeValue(h.presion.derecha[s] || 0));

 const imgPres = await createChartImage(
 fechas,
 presIzq,
 presDer,
 `Presión - ${etiquetasSensores[s]}`,
 "Pres Izquierda",
 "Pres Derecha"
 );

 // Presión abajo
 pdf.setFontSize(14);
 pdf.text(`Presión`, 10, 120);
 pdf.addImage(imgPres, "PNG", 10, 130, 180, 70);
 }
 // ---------------------------------------------------------
 // INTERPRETACIÓN AUTOMÁTICA
 // ---------------------------------------------------------
 pdf.addPage();
 pdf.setFontSize(20);
 pdf.text("Interpretación Automática", 10, 20);
 pdf.setFontSize(12);

// --- construir interpretacion ---
 let interpretacion = [];

 const UMB_TEMP_DIFF = 2.2;
 const UMB_PRESS_DIFF = 15;
 const UMB_TEMP_HIGH = 40;
 const UMB_PRESS_HIGH = 200;
 const UMB_TEMP_SPIKE = 5;
 const UMB_PRESS_SPIKE = 50;

 function getSpikes(arr, umbral) {
 let spikes = [];
 for (let i = 1; i < arr.length; i++) {
 const a = Number(arr[i-1]), b = Number(arr[i]);
 if (!isNaN(a) && !isNaN(b)) {
 const diff = b - a;
 if (diff > umbral) spikes.push({ salto: diff.toFixed(2), idx: i });
 }
 }
 return spikes;
 }

 sensoresLista.forEach(s => {
 const tempsI = extraerSerie("izquierda", s, "temperatura");
 const tempsD = extraerSerie("derecha", s, "temperatura");
 const presI = extraerSerie("izquierda", s, "presion");
 const presD = extraerSerie("derecha", s, "presion");

 const avgI = Number(calcStats(tempsI).prom);
 const avgD = Number(calcStats(tempsD).prom);
 const avgPI = Number(calcStats(presI).prom);
 const avgPD = Number(calcStats(presD).prom);

 // DEBUG: ver arrays y promedios
 console.log(s, {tempsI, tempsD, avgI, avgD, presI, presD, avgPI, avgPD});

 if (!isNaN(avgI) && !isNaN(avgD)) {
 const diffT = avgD - avgI;
 if (Math.abs(diffT) > UMB_TEMP_DIFF) interpretacion.push(
 `• Temperatura – ${labels[s]}: diferencia IZQ–DER de ${diffT.toFixed(2)}°C (posible inflamación).`
 );
 if (avgI > UMB_TEMP_HIGH || avgD > UMB_TEMP_HIGH) interpretacion.push(
 `• Temperatura – ${labels[s]}: valores elevados (> ${UMB_TEMP_HIGH}°C).`
 );
 const spikesTempI = getSpikes(tempsI, UMB_TEMP_SPIKE);
 const spikesTempD = getSpikes(tempsD, UMB_TEMP_SPIKE);
 if (spikesTempI.length) interpretacion.push(`• Temperatura – ${labels[s]} IZQ: aumento brusco (+${spikesTempI[0].salto}°C).`);
 if (spikesTempD.length) interpretacion.push(`• Temperatura – ${labels[s]} DER: aumento brusco (+${spikesTempD[0].salto}°C).`);
 }

 if (!isNaN(avgPI) && !isNaN(avgPD)) {
 const diffP = avgPD - avgPI;
 if (Math.abs(diffP) > UMB_PRESS_DIFF) interpretacion.push(
 `• Presión – ${labels[s]}: asimetría IZQ–DER significativa (${diffP.toFixed(2)}).`
 );
 if (avgPI > UMB_PRESS_HIGH || avgPD > UMB_PRESS_HIGH) interpretacion.push(
 `• Presión – ${labels[s]}: valores elevados (> ${UMB_PRESS_HIGH}).`
 );
 const spikesPressI = getSpikes(presI, UMB_PRESS_SPIKE);
 const spikesPressD = getSpikes(presD, UMB_PRESS_SPIKE);
 if (spikesPressI.length) interpretacion.push(`• Presión – ${labels[s]} IZQ: incremento brusco (+${spikesPressI[0].salto}).`);
 if (spikesPressD.length) interpretacion.push(`• Presión – ${labels[s]} DER: incremento brusco (+${spikesPressD[0].salto}).`);
 }
 });

 if (interpretacion.length === 0) interpretacion.push("No se detectaron anomalías relevantes en temperatura o presión.");

 console.log("Interpretacion final:", interpretacion);

 // --- escribir interpretación en PDF ---
 let y = 30;
 pdf.setFontSize(12);
 interpretacion.forEach(line => {
 if (y > 270) { pdf.addPage(); y = 20; }
 pdf.text(line, 10, y);
 y += 8;
 });

 // Función auxiliar que convierte un Blob a Base64 usando una Promise
 function blobToBase64(blob) {
 return new Promise((resolve, reject) => {
 const reader = new FileReader();
 // Cuando termina de leer, resolvemos la promesa con el Base64 puro (sin el prefijo)
 reader.onloadend = () => resolve(reader.result.split(',')[1]);
 reader.onerror = reject;
 reader.readAsDataURL(blob);
 });
 }

 // 1. Convertimos el PDF a blob
 const pdfBlob = pdf.output("blob");

 // 2. Usamos 'await' para PAUSAR la función exportInforme() hasta que 
 // la función asíncrona 'blobToBase64' termine y devuelva el resultado.
 console.log("Esperando a convertir PDF a Base64...");
 const base64PDF = await blobToBase64(pdfBlob);
 console.log("Conversión completa. Llamando a enviarInformePorMail.");

 // 3. Ahora que 'base64PDF' tiene el valor correcto, llamamos a la función de envío.
 enviarInformePorMail(base64PDF);

 // ---------------------------------------------------------
 // DESCARGA FINAL
 // ---------------------------------------------------------

 pdf.save(`Informe_Prevent_${new Date().toISOString().slice(0, 10)}.pdf`);
}


// ----------------------------------------------------------------------
// UTILIDADES PARA CARGAR IMÁGENES Y CREAR GRÁFICOS
// ----------------------------------------------------------------------

// Cargar logo como base64
function loadImageToBase64(url) {
 return new Promise((resolve, reject) => {
 const img = new Image();
 img.crossOrigin = "Anonymous";
 img.onload = () => {
 const canvas = document.createElement("canvas");
 canvas.width = img.width;
 canvas.height = img.height;
 const ctx = canvas.getContext("2d");
 ctx.drawImage(img, 0, 0);
 resolve(canvas.toDataURL("image/png"));
 };
 img.onerror = reject;
 img.src = url;
 });
}

// Crear gráfico con Chart.js y devolver imagen base64
function createChartImage(labels, serieIzq, serieDer, titulo, labelIzq, labelDer) {
 return new Promise(resolve => {
 const canvas = document.getElementById("chartCanvas");
 const ctx = canvas.getContext("2d");

 // Destruir gráfico previo si existe (evita acumulación)
 if (window.lastChart) {
 window.lastChart.destroy();
 }

 window.lastChart = new Chart(ctx, {
 type: "line",
 data: {
 labels,
 datasets: [
 { 
 label: labelIzq, 
 data: serieIzq, 
 borderWidth: 2 
 },
 { 
 label: labelDer, 
 data: serieDer, 
 borderWidth: 2 
 }
 ]
 },
 options: {
 responsive: false,
 plugins: {
 title: {
 display: true,
 text: titulo
 }
 }
 }
 });

 setTimeout(() => {
 resolve(canvas.toDataURL("image/png"));
 }, 500);
 });
}