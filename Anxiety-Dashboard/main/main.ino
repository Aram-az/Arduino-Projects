#include <Wire.h>
#include <math.h>

// ------------------ MPU6050 CONFIG ------------------
#define MPU 0x68

#define MA_WINDOW 20
float ax, ay, az;
float gx, gy, gz;

float maBuffer[MA_WINDOW];
int   maIndex      = 0;
float tremorLevel  = 0.0f;

// ------------------ PINS ------------------
const int REDledPin  = 4;
const int YELledPin  = 3;
const int GREledPin  = 2;
const int relayPin   = 5;
const int buttonPin  = 6;

// ------------------ GSR CONFIG ------------------
const int   GSR_PIN   = A0;
const unsigned long DT_MS = 20;   // ~50 Hz update for GSR

// Time constants in seconds
const float TAU_FAST = 1.0f;      // phasic ~1 s
const float TAU_SLOW = 30.0f;     // tonic ~30 s

float alpha_fast, alpha_slow;
float ema_fast = NAN, ema_slow = NAN;
unsigned long last_gsr_ms = 0;

// Feature exposed to rest of code
float gsr_raw      = 0.0f;
float gsr_tonic    = 0.0f;
float gsr_peak_vis = 0.0f;

// ------------------ STATE ------------------
bool  buttonState       = false;
bool  tremorDetected    = false;
int   severity          = 0;      // 0 = low, 1 = med, 2 = high

// -------------------------------------------------
// SETUP
// -------------------------------------------------
void setup() {
  // LED, button, relay setup
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(REDledPin, OUTPUT);
  pinMode(YELledPin, OUTPUT);
  pinMode(GREledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(REDledPin, LOW);
  digitalWrite(YELledPin, LOW);
  digitalWrite(GREledPin, LOW);
  digitalWrite(relayPin, LOW);

  // Serial + I2C
  Serial.begin(115200);
  Wire.begin();

  // Wake MPU6050
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  for (int i = 0; i < MA_WINDOW; i++) maBuffer[i] = 0.0f;

  // GSR analog reference + alpha precompute
  analogReference(INTERNAL);    // 1.1 V reference (as in your original GSR code)

  alpha_fast = 1.0f - expf(-(DT_MS / 1000.0f) / TAU_FAST);
  alpha_slow = 1.0f - expf(-(DT_MS / 1000.0f) / TAU_SLOW);
}

// -------------------------------------------------
// MAIN LOOP
// -------------------------------------------------
void loop() {
  unsigned long now = millis();

  // 1) Update GSR at ~50 Hz
  if (now - last_gsr_ms >= DT_MS) {
    last_gsr_ms = now;
    updateGSR();
  }

  // 2) Update tremor level from MPU
  float tremorMag = getTremorLevel();          // updates tremorLevel global
  tremorDetected  = (tremorMag > 1.0f);

  // 3) Simple severity logic
  // Tune these thresholds as needed based on your data
  bool strongGsr = (fabs(gsr_peak_vis) > 45.0f);   // strong spike
  bool mildGsr   = (fabs(gsr_peak_vis) > 25.0f);   // moderate spike

  if (strongGsr && tremorDetected) {
    severity = 2;  // high
  } else if (strongGsr || tremorDetected || mildGsr) {
    severity = 1;  // medium
  } else {
    severity = 0;  // low
  }

  // 4) If strong tremor, run motor & breathing pattern
  if (tremorMag > 1.0f) {
    runMotor();

    for (int i = 0; i < 10; i++) {
      breathePattern();

      buttonState = (digitalRead(buttonPin) == LOW);
      if (buttonState) {
        break;      // user pressed button to stop breathing early
      }
    }
  }

  // 5) Send JSON sample for the website
  sendJsonSample(now);

  delay(10);
}

// -------------------------------------------------
// MOTOR CONTROL
// -------------------------------------------------
void runMotor() {
  digitalWrite(relayPin, HIGH);
  delay(3000);
  digitalWrite(relayPin, LOW);
}

// -------------------------------------------------
// LED BREATHING PATTERN
// (unchanged from your original, just renamed)
// -------------------------------------------------
void breathePattern() {
  digitalWrite(REDledPin, HIGH);
  delay(1000);
  digitalWrite(YELledPin, HIGH);
  delay(1000);
  digitalWrite(GREledPin, HIGH);
  delay(500);

  digitalWrite(REDledPin, LOW);
  digitalWrite(YELledPin, LOW);
  digitalWrite(GREledPin, LOW);

  digitalWrite(REDledPin, HIGH);
  digitalWrite(YELledPin, HIGH);
  digitalWrite(GREledPin, HIGH);
  delay(500);

  digitalWrite(REDledPin, LOW);
  delay(1000);
  digitalWrite(YELledPin, LOW);
  delay(1000);
  digitalWrite(GREledPin, LOW);
  delay(1000);
}

// -------------------------------------------------
// GSR UPDATE (from your second sketch, wrapped)
// -------------------------------------------------
void updateGSR() {
  long acc = 0;
  for (int i = 0; i < 6; i++) {
    acc += analogRead(GSR_PIN);
    delayMicroseconds(500);
  }
  float raw = acc / 6.0f;    // ADC counts 0..1023

  gsr_raw = raw;

  if (isnan(ema_fast)) {
    ema_fast = raw;
    ema_slow = raw;
  }

  ema_fast += alpha_fast * (raw - ema_fast);
  ema_slow += alpha_slow * (raw - ema_slow);

  float tonic = ema_slow;
  float peak  = ema_fast - ema_slow;

  gsr_tonic    = tonic;
  gsr_peak_vis = peak * 8.0f;    // scaled for visualization
}

// -------------------------------------------------
// MOVING AVERAGE FOR TREMOR
// -------------------------------------------------
float movingAverage(float v) {
  maBuffer[maIndex] = v;
  maIndex = (maIndex + 1) % MA_WINDOW;

  float sum = 0.0f;
  for (int i = 0; i < MA_WINDOW; i++) sum += maBuffer[i];

  return sum / MA_WINDOW;
}

// -------------------------------------------------
// READ MPU6050
// -------------------------------------------------
void readMPU() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU, 14);  // AccelXYZ, Temp, GyroXYZ

  int16_t rawAx = (Wire.read() << 8) | Wire.read();
  int16_t rawAy = (Wire.read() << 8) | Wire.read();
  int16_t rawAz = (Wire.read() << 8) | Wire.read();
  Wire.read(); Wire.read();   // ignore temperature
  int16_t rawGx = (Wire.read() << 8) | Wire.read();
  int16_t rawGy = (Wire.read() << 8) | Wire.read();
  int16_t rawGz = (Wire.read() << 8) | Wire.read();

  ax = rawAx / 16384.0f;
  ay = rawAy / 16384.0f;
  az = rawAz / 16384.0f;

  gx = rawGx / 131.0f;
  gy = rawGy / 131.0f;
  gz = rawGz / 131.0f;
}

// -------------------------------------------------
// TREMOR DETECTOR
// -------------------------------------------------
float getTremorLevel() {
  readMPU();

  float accMag      = sqrt(ax * ax + ay * ay + az * az);
  float vibration   = accMag - 1.0f;           // remove gravity
  float smooth      = movingAverage(vibration);
  float tremorSignal = vibration - smooth;

  tremorSignal *= 5.0f;

  tremorLevel = 0.7f * tremorLevel + 0.3f * fabs(tremorSignal);

  return tremorLevel;
}

// -------------------------------------------------
// JSON OUTPUT FOR WEBSITE
// -------------------------------------------------
void sendJsonSample(unsigned long nowMs) {
  int tremorFlag = tremorDetected ? 1 : 0;

  Serial.print("{\"t\":");
  Serial.print(nowMs);
  Serial.print(",\"gsr\":");
  Serial.print(gsr_peak_vis, 2);
  Serial.print(",\"tremor\":");
  Serial.print(tremorFlag);
  Serial.print(",\"severity\":");
  Serial.print(severity);
  Serial.println("}");
}
