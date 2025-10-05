/**
 * Sensor Monitoring Dashboard
 * JavaScript module for managing real-time sensor data display
 */

// ============================================
// Configuration
// ============================================
const CONFIG = {
  updateInterval: 2000, // Update interval in milliseconds
  apiEndpoint: '/api/sensors', // API endpoint (to be configured)
  autoRefresh: true
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
      met3: 0,
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
      met3: 0,
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
  met3: '3er Metatarsiano',
  met5: '5to Metatarsiano',
  mid: 'Zona Central'
};

// ============================================
// Initialization
// ============================================
document.addEventListener('DOMContentLoaded', () => {
  console.log('Sensor Dashboard Initialized');
  
  // Initialize with sample data
  generateSampleData();
  
  // Update display
  updateDisplay();
  
  // Setup event listeners
  setupEventListeners();
  
  // Start auto-refresh if enabled
  if (CONFIG.autoRefresh) {
    startAutoRefresh();
  }
  
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
    refreshData();
    
    setTimeout(() => {
      refreshBtn.classList.remove('rotating');
    }, 1000);
  });
}

// ============================================
// Data Management
// ============================================

/**
 * Generate sample sensor data for demonstration
 */
function generateSampleData() {
  // Left foot temperature
  sensorData.left.temperature.heel = (30 + Math.random() * 5).toFixed(1);
  sensorData.left.temperature.side = (31 + Math.random() * 5).toFixed(1);
  sensorData.left.temperature.hallux = (32 + Math.random() * 5).toFixed(1);
  sensorData.left.temperature.met1 = (31 + Math.random() * 5).toFixed(1);
  sensorData.left.temperature.met3 = (30 + Math.random() * 5).toFixed(1);
  sensorData.left.temperature.met5 = (29 + Math.random() * 5).toFixed(1);
  
  // Left foot pressure
  sensorData.left.pressure.heel = (100 + Math.random() * 40).toFixed(1);
  sensorData.left.pressure.met1 = (110 + Math.random() * 40).toFixed(1);
  sensorData.left.pressure.met5 = (90 + Math.random() * 30).toFixed(1);
  sensorData.left.pressure.mid = (95 + Math.random() * 40).toFixed(1);
  
  // Right foot temperature
  sensorData.right.temperature.heel = (30 + Math.random() * 5).toFixed(1);
  sensorData.right.temperature.side = (31 + Math.random() * 5).toFixed(1);
  sensorData.right.temperature.hallux = (32 + Math.random() * 5).toFixed(1);
  sensorData.right.temperature.met1 = (31 + Math.random() * 5).toFixed(1);
  sensorData.right.temperature.met3 = (30 + Math.random() * 5).toFixed(1);
  sensorData.right.temperature.met5 = (29 + Math.random() * 5).toFixed(1);
  
  // Right foot pressure
  sensorData.right.pressure.heel = (100 + Math.random() * 40).toFixed(1);
  sensorData.right.pressure.met1 = (110 + Math.random() * 40).toFixed(1);
  sensorData.right.pressure.met5 = (90 + Math.random() * 30).toFixed(1);
  sensorData.right.pressure.mid = (95 + Math.random() * 40).toFixed(1);
}

/**
 * Fetch data from API or sensor source
 * This is a placeholder - replace with actual API call
 */
async function fetchSensorData() {
  // TODO: Implement actual API call
  // Example:
  // try {
  //   const response = await fetch(CONFIG.apiEndpoint);
  //   const data = await response.json();
  //   return data;
  // } catch (error) {
  //   console.error('Error fetching sensor data:', error);
  //   updateConnectionStatus(false);
  //   return null;
  // }
  
  // For now, generate sample data
  generateSampleData();
  updateConnectionStatus(true);
}

/**
 * Refresh all sensor data
 */
function refreshData() {
  fetchSensorData();
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
  const tempLocations = ['heel', 'side', 'hallux', 'met1', 'met3', 'met5'];
  tempLocations.forEach(location => {
    if (sensorData.left.temperature[location] !== undefined) {
      const row = createTableRow(
        sensorLabels[location] || location,
        'Temperatura',
        `${sensorData.left.temperature[location]} °C`,
        `${sensorData.right.temperature[location]} °C`,
        'temp'
      );
      tableBody.appendChild(row);
    }
  });
  
  // Pressure rows
  const pressLocations = ['heel', 'met1', 'met5', 'mid'];
  pressLocations.forEach(location => {
    if (sensorData.left.pressure[location] !== undefined) {
      const row = createTableRow(
        sensorLabels[location] || location,
        'Presión',
        `${sensorData.left.pressure[location]} N`,
        `${sensorData.right.pressure[location]} N`,
        'press'
      );
      tableBody.appendChild(row);
    }
  });
}

/**
 * Create a table row element
 */
function createTableRow(location, type, leftValue, rightValue, badgeType) {
  const row = document.createElement('tr');
  row.innerHTML = `
    <td><strong>${location}</strong></td>
    <td><span class="sensor-type-badge badge-${badgeType}">${type}</span></td>
    <td>${leftValue}</td>
    <td>${rightValue}</td>
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
// Auto-refresh
// ============================================
let refreshInterval = null;

function startAutoRefresh() {
  if (refreshInterval) {
    clearInterval(refreshInterval);
  }
  
  refreshInterval = setInterval(() => {
    refreshData();
  }, CONFIG.updateInterval);
  
  console.log(`Auto-refresh started (${CONFIG.updateInterval}ms interval)`);
}

function stopAutoRefresh() {
  if (refreshInterval) {
    clearInterval(refreshInterval);
    refreshInterval = null;
    console.log('Auto-refresh stopped');
  }
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
  startAutoRefresh,
  stopAutoRefresh,
  CONFIG
};
