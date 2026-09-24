// node-server/server.js

const FAKE_MODE = false;          // false = use Arduino serial, true = fake data
const ARDUINO_PORT = 'COM3';      // update if your board is on a different COM
const BAUD = 115200;

const WebSocket = require('ws');
let SerialPort, ReadlineParser;
if (!FAKE_MODE) {
  ({ SerialPort } = require('serialport'));
  ({ ReadlineParser } = require('@serialport/parser-readline'));
}

const wss = new WebSocket.Server({ port: 8080 });
console.log('WebSocket server listening on ws://localhost:8080');

function broadcast(sampleObj) {
  const msg = JSON.stringify(sampleObj);
  wss.clients.forEach(client => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(msg);
    }
  });
}

wss.on('connection', ws => {
  console.log('Browser connected');
});

// ---- FAKE MODE (no Arduino needed) ----
if (FAKE_MODE) {
  console.log('Running in FAKE_MODE (no Arduino required).');
  let t0 = Date.now();
  setInterval(() => {
    const now = Date.now();
    const dt = now - t0;
    const gsr = 20 + 5 * Math.sin(dt / 2000);
    const tremor = Math.random() < 0.2 ? 1 : 0;
    const severity = Math.floor(Math.random() * 3); // 0,1,2

    const sample = { t: dt, gsr, tremor, severity };
    broadcast(sample);
  }, 200);
}

// ---- REAL SERIAL MODE ----
if (!FAKE_MODE) {
  console.log('Running in REAL SERIAL MODE (expects Arduino on ' + ARDUINO_PORT + ')');

  const port = new SerialPort({
    path: ARDUINO_PORT,
    baudRate: BAUD,
  });
  const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

  port.on('open', () => console.log('Serial port opened'));
  port.on('error', err => console.error('Serial error:', err));

  parser.on('data', line => {
    const trimmed = line.trim();
    if (!trimmed) return;
    try {
      const sample = JSON.parse(trimmed);
      broadcast(sample);
    } catch (e) {
      console.error('Bad JSON from Arduino:', trimmed);
    }
  });
}

wss.on('error', err => console.error('WebSocket error:', err));