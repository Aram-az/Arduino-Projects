// ===== WebSocket connection =====
const socket = new WebSocket('ws://localhost:8080');

const eventsCountEl = document.getElementById('eventsCount');
const lastTimeEl = document.getElementById('lastTime');
const lastSeverityEl = document.getElementById('lastSeverity');
const tableBody = document.getElementById('eventsTableBody');

// Emergency / breathing elements
const overlayEl = document.getElementById('emergencyOverlay');
const modalTitleEl = document.getElementById('modalTitle');
const modalTextEl = document.getElementById('modalText');
const modalButtonsEl = document.getElementById('modalButtons');
const detectionStatusEl = document.getElementById('detectionStatus');
const btnYesAnxiety = document.getElementById('btnYesAnxiety');
const btnNoAnxiety = document.getElementById('btnNoAnxiety');

// Demo trigger button
const demoTriggerBtn = document.getElementById('demoTrigger');

let eventsCount = 0;

// Detection state
let detectionEnabled = true;
let modalActive = false;

// Track last real sample so demo can use it
let lastSample = null;

// ===== Time window chips =====
const chips = document.querySelectorAll('.chip');
let maxPoints = 100;

chips.forEach(chip => {
  chip.addEventListener('click', () => {
    chips.forEach(c => c.classList.remove('active'));
    chip.classList.add('active');
    const w = chip.getAttribute('data-window');
    if (w === 'session') maxPoints = 300;
    else if (w === 'short') maxPoints = 30;
    else if (w === 'long') maxPoints = 100;
  });
});

// ===== Chart.js setup =====
const ctx = document.getElementById('gsrChart').getContext('2d');
const gsrData = {
  labels: [],
  datasets: [{
    label: 'GSR peak',
    data: [],
    borderWidth: 2,
    tension: 0.2,
    pointRadius: 0
  }]
};

const gsrChart = new Chart(ctx, {
  type: 'line',
  data: gsrData,
  options: {
    responsive: true,
    animation: false,
    interaction: {
      mode: 'index',
      intersect: false
    },
    plugins: {
      legend: {
        labels: {
          color: '#e5e7eb',
          font: { size: 11 }
        }
      },
      title: { display: false }
    },
    scales: {
      x: {
        ticks: { color: '#9ca3af', maxTicksLimit: 8 },
        grid: { color: 'rgba(148, 163, 184, 0.16)' },
        title: {
          display: true,
          text: 'Time (s)',
          color: '#9ca3af',
          font: { size: 11 }
        }
      },
      y: {
        ticks: { color: '#9ca3af', maxTicksLimit: 6 },
        grid: { color: 'rgba(148, 163, 184, 0.18)' },
        title: {
          display: true,
          text: 'GSR',
          color: '#9ca3af',
          font: { size: 11 }
        }
      }
    }
  }
});

// ===== Table rendering =====

function severityBadgeClass(sev) {
  if (sev === 0) return 'low';
  if (sev === 1) return 'medium';
  return 'high';
}

function severityLabel(sev) {
  if (sev === 0) return 'LOW';
  if (sev === 1) return 'MED';
  return 'HIGH';
}

function addRow(sample) {
  const tr = document.createElement('tr');
  tr.innerHTML = `
    <td>${(sample.t / 1000).toFixed(1)}</td>
    <td>${sample.gsr.toFixed(2)}</td>
    <td>${sample.tremor ? 'Yes' : 'No'}</td>
    <td>
      <span class="badge ${severityBadgeClass(sample.severity)}">
        ${severityLabel(sample.severity)}
      </span>
    </td>
  `;
  tableBody.prepend(tr);
  while (tableBody.rows.length > 50) {
    tableBody.deleteRow(-1);
  }
}

// ===== Emergency / breathing logic =====

function showEmergencyPrompt(sample) {
  modalActive = true;
  overlayEl.classList.add('visible');
  overlayEl.classList.remove('breathing');

  modalTitleEl.textContent = 'Possible Anxiety Episode Detected';
  modalTextEl.textContent =
    'We detected a strong anxiety spike from the sensor. Are you experiencing significant anxiety right now?';

  modalButtonsEl.innerHTML = '';
  modalButtonsEl.appendChild(btnYesAnxiety);
  modalButtonsEl.appendChild(btnNoAnxiety);

  detectionStatusEl.textContent = detectionEnabled ? 'Enabled' : 'Disabled';
}

function showBreathingInstructions() {
  overlayEl.classList.add('visible', 'breathing');

  modalTitleEl.textContent = 'Guided Breathing';
  modalTextEl.textContent =
    'Follow the breathing LED pattern on the Arduino until you feel your anxiety easing.';

  modalButtonsEl.innerHTML = '';
  const btnDone = document.createElement('button');
  btnDone.textContent = 'Done / Close';
  btnDone.className = 'btn btn-primary';
  btnDone.addEventListener('click', () => {
    overlayEl.classList.remove('visible', 'breathing');
    modalActive = false;
  });
  modalButtonsEl.appendChild(btnDone);
}

btnYesAnxiety.addEventListener('click', () => {
  showBreathingInstructions();
});

btnNoAnxiety.addEventListener('click', () => {
  detectionEnabled = false;
  detectionStatusEl.textContent = 'Disabled (until re-enabled)';
  overlayEl.classList.remove('visible', 'breathing');
  modalActive = false;
});

// ===== Demo episode injection =====

function injectDemoEpisode() {
  const base = lastSample || {
    t: 0,
    gsr: 20,
    tremor: 0,
    severity: 0
  };

  const demoSample = {
    t: base.t + 1000,
    gsr: base.gsr + 10,
    tremor: 1,
    severity: 2
  };

  eventsCount++;
  eventsCountEl.textContent = eventsCount;
  lastTimeEl.textContent = (demoSample.t / 1000).toFixed(1);
  lastSeverityEl.textContent = demoSample.severity;

  const tSeconds = (demoSample.t / 1000).toFixed(1);
  gsrData.labels.push(tSeconds);
  gsrData.datasets[0].data.push(demoSample.gsr);
  if (gsrData.labels.length > maxPoints) {
    gsrData.labels.shift();
    gsrData.datasets[0].data.shift();
  }
  gsrChart.update('none');

  addRow(demoSample);

  showEmergencyPrompt(demoSample);
}

if (demoTriggerBtn) {
  demoTriggerBtn.addEventListener('click', injectDemoEpisode);
}

// ===== WebSocket events =====

socket.onopen = () => {
  console.log('Connected to WebSocket server');
};

socket.onmessage = (event) => {
  const sample = JSON.parse(event.data);

  lastSample = sample;

  eventsCount++;
  eventsCountEl.textContent = eventsCount;
  lastTimeEl.textContent = (sample.t / 1000).toFixed(1);
  lastSeverityEl.textContent = sample.severity;

  const tSeconds = (sample.t / 1000).toFixed(1);
  gsrData.labels.push(tSeconds);
  gsrData.datasets[0].data.push(sample.gsr);
  if (gsrData.labels.length > maxPoints) {
    gsrData.labels.shift();
    gsrData.datasets[0].data.shift();
  }
  gsrChart.update('none');

  addRow(sample);

  if (!detectionEnabled || modalActive) return;
  if (sample.severity >= 2) {
    showEmergencyPrompt(sample);
  }
};

socket.onerror = (e) => console.error('WebSocket error', e);
socket.onclose = () => console.log('WebSocket closed');