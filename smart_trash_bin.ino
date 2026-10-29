#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// ---------------- WI-FI ACCESS POINT ----------------
const char* apSSID = "SmartTrashBin";
const char* apPassword = "TrashBin123";

WebServer server(80);

// ---------------- SENSOR PINS ----------------
// Ultrasonic 1: Hand/object detection
#define LID_TRIG 23
#define LID_ECHO 22

// Ultrasonic 2: Waste-level measurement
#define WASTE_TRIG 18
#define WASTE_ECHO 19

// Servo motor
#define SERVO_PIN 21

// I2C LCD
#define LCD_SDA 16
#define LCD_SCL 17

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servoMotor;

// ---------------- BIN SETTINGS ----------------
// Measure from the waste sensor face to the bin floor with the bin empty.
const float BIN_HEIGHT_CM = 16.2;
const float HAND_DISTANCE_CM = 5.0;
const float FULL_DISTANCE_CM = 5.0;

const unsigned long LID_OPEN_TIME = 5000;
const unsigned long WASTE_UPDATE_INTERVAL = 1000;
const unsigned long LID_CHECK_INTERVAL = 150;

float lidDistance = -1;
float wasteDistance = -1;
int wastePercent = -1;

bool lidOpen = false;
unsigned long lidCloseTime = 0;
unsigned long lastWasteUpdate = 0;
unsigned long lastLidCheck = 0;

String binStatus = "SENSOR ERROR";

// ---------------- READ ULTRASONIC SENSOR ----------------
float readDistance(int trigPin, int echoPin) {
  // Ensure a clean trigger pulse.
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Wait up to 30 ms for the echo pulse (about 5 m maximum).
  unsigned long duration = pulseIn(echoPin, HIGH, 30000UL);

  // A zero duration means no echo was received.
  if (duration == 0) {
    return -1.0;
  }

  float distanceCm = (duration * 0.0343f) / 2.0f;

  // Reject impossible/invalid readings instead of treating them as data.
  if (distanceCm < 2.0f || distanceCm > 400.0f) {
    return -1.0;
  }

  return distanceCm;
}

// ---------------- LID CONTROL ----------------
void openLid() {
  if (lidOpen) return;

  lidOpen = true;
  lidCloseTime = millis() + LID_OPEN_TIME;

  servoMotor.write(90);

  Serial.println("LID OPEN");
}

void checkLid() {
  unsigned long now = millis();

  // Read the hand-detection sensor
  lidDistance = readDistance(LID_TRIG, LID_ECHO);

  if (!lidOpen &&
      lidDistance > 0 &&
      lidDistance < HAND_DISTANCE_CM) {
    openLid();
  }

  // Close the lid after the configured open time
  if (lidOpen &&
      (long)(now - lidCloseTime) >= 0) {
    lidOpen = false;
    servoMotor.write(0);

    Serial.println("LID CLOSED");
  }
}

// ---------------- WASTE-LEVEL MONITORING ----------------
void updateWasteLevel() {
  wasteDistance = readDistance(WASTE_TRIG, WASTE_ECHO);

  // Only a failed/invalid ultrasonic reading is a sensor error.
  // If the measured distance is greater than the configured bin height,
  // treat the bin as empty (0%), not as a sensor error.
  if (wasteDistance < 0) {
    wastePercent = -1;
    binStatus = "SENSOR ERROR";
    Serial.println("Waste sensor: NO VALID ECHO. Check TRIG/ECHO wiring and power.");
    return;
  }

  float filledHeight = BIN_HEIGHT_CM - wasteDistance;
  if (filledHeight < 0) filledHeight = 0;

  wastePercent = constrain(
      (int)((filledHeight / BIN_HEIGHT_CM) * 100.0f),
      0,
      100
  );

  if (wasteDistance <= FULL_DISTANCE_CM || wastePercent >= 95) {
    binStatus = "FULL";
  } else if (wastePercent >= 70) {
    binStatus = "ALMOST FULL";
  } else if (wastePercent >= 30) {
    binStatus = "HALF";
  } else {
    binStatus = "LOW";
  }

  Serial.print("Waste distance: ");
  Serial.print(wasteDistance, 1);
  Serial.print(" cm | Fill: ");
  Serial.print(wastePercent);
  Serial.print("% | Status: ");
  Serial.println(binStatus);
}

// ---------------- LCD DISPLAY ----------------
void updateLCD() {
  lcd.setCursor(0, 0);

  if (wastePercent < 0) {
    lcd.print("Waste: ERROR    ");
  } else {
    lcd.print("Waste: ");
    lcd.print(wastePercent);
    lcd.print("%       ");
  }

  lcd.setCursor(0, 1);

  if (wastePercent < 0) {
    lcd.print("Check sensor    ");
  } else if (binStatus == "FULL") {
    lcd.print("Status: FULL    ");
  } else if (binStatus == "ALMOST FULL") {
    lcd.print("Status: ALMOST  ");
  } else if (binStatus == "HALF") {
    lcd.print("Status: HALF    ");
  } else {
    lcd.print("Status: LOW     ");
  }
}

// ---------------- DASHBOARD HTML ----------------
const char dashboardHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Smart Trash Bin</title>

<style>
* {
  box-sizing: border-box;
}

body {
  margin: 0;
  padding: 20px 12px;
  background: #f3f4f6;
  color: #171717;
  font-family: Arial, sans-serif;
}

.dashboard {
  max-width: 460px;
  margin: 0 auto;
  padding: 18px;
  background: white;
  border: 1px solid #222;
  box-shadow: 0 2px 5px #0002;
}

header {
  text-align: center;
  margin-bottom: 16px;
}

header h2 {
  font-size: 19px;
  margin: 3px 0;
}

header p {
  font-size: 12px;
  margin: 0;
}

.waste-card {
  border: 1px solid #222;
  border-radius: 16px;
  padding: 12px;
  margin-bottom: 9px;
}

.label {
  font-size: 12px;
  font-weight: bold;
}

.percentage {
  text-align: center;
  font-size: 29px;
  font-weight: bold;
  margin: 8px 0 5px;
}

.progress {
  height: 12px;
  border: 1px solid #777;
  border-radius: 3px;
  overflow: hidden;
}

#progressFill {
  height: 100%;
  width: 0%;
  background: #ffe9a8;
  transition: width 0.3s;
}

.caption {
  font-size: 10px;
  margin-top: 5px;
}

.status-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 8px;
}

.status-card {
  min-width: 0;
  border: 1px solid #222;
  border-radius: 12px;
  padding: 13px 9px;
  display: flex;
  flex-direction: column;
  gap: 10px;
}

.status-value {
  display: inline-block;
  align-self: flex-start;
  max-width: 100%;
  padding: 7px 10px;
  border-radius: 5px;
  font-size: 11px;
  font-weight: bold;
  overflow-wrap: anywhere;
  background: #eee;
}

.bin-value {
  background: #fff2c9;
  color: #8b6500;
  border: 1px solid #e4bd54;
}

.lid-value {
  background: #fbdada;
  color: #9c1010;
  border: 1px solid #df7373;
}

.control-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 10px;
  margin-top: 12px;
}

.control-button {
  border: 0;
  border-radius: 8px;
  padding: 12px 10px;
  font-size: 13px;
  font-weight: bold;
  cursor: pointer;
  color: white;
  transition: opacity 0.2s, transform 0.1s;
}

.control-button:active {
  transform: scale(0.98);
}

.control-button:disabled {
  opacity: 0.6;
  cursor: wait;
}

.open-button {
  background: #198754;
}

.close-button {
  background: #b42318;
}

#errorMessage {
  text-align: center;
  font-size: 11px;
  color: #b42318;
  margin-top: 12px;
}

#controlMessage {
  text-align: center;
  font-size: 11px;
  color: #555;
  margin-top: 8px;
  min-height: 14px;
}

@media (min-width: 380px) {
  .status-card {
    flex-direction: row;
    align-items: center;
    justify-content: space-between;
    flex-wrap: wrap;
  }
}
</style>
</head>

<body>
<main class="dashboard">
  <header>
    <h2>SMART TRASH BIN</h2>
    <p>ESP32 Monitoring Dashboard</p>
  </header>

  <section class="waste-card">
    <div class="label">Waste Level</div>

    <div class="percentage" id="percentage">--%</div>

    <div class="progress">
      <div id="progressFill"></div>
    </div>

    <div class="caption">Estimated Fill Level</div>
  </section>

  <section class="status-row">
    <div class="status-card">
      <span class="label">Bin Status</span>
      <span class="status-value bin-value" id="binStatus">
        Loading
      </span>
    </div>

    <div class="status-card">
      <span class="label">Lid Status</span>
      <span class="status-value lid-value" id="lidStatus">
        Loading
      </span>
    </div>
  </section>

  <section class="control-row" aria-label="Lid controls">
    <button class="control-button open-button" id="openButton"
            type="button" onclick="controlLid('open')">
      OPEN LID
    </button>
    <button class="control-button close-button" id="closeButton"
            type="button" onclick="controlLid('close')">
      CLOSE LID
    </button>
  </section>

  <div id="controlMessage" role="status"></div>
  <div id="errorMessage"></div>
</main>

<script>
async function controlLid(action) {
  const openButton = document.getElementById('openButton');
  const closeButton = document.getElementById('closeButton');
  const message = document.getElementById('controlMessage');

  openButton.disabled = true;
  closeButton.disabled = true;
  message.textContent = action === 'open'
    ? 'Opening lid...'
    : 'Closing lid...';

  try {
    const response = await fetch('/lid/' + action, {
      method: 'POST',
      cache: 'no-store'
    });

    const result = await response.json();
    if (!response.ok || !result.success) {
      throw new Error(result.message || 'Command failed');
    }

    message.textContent = action === 'open'
      ? 'Open command sent. Lid will auto-close after 5 seconds.'
      : 'Close command sent.';
    await updateDashboard();
  } catch (error) {
    message.textContent = 'Could not control lid. Refresh and try again.';
  } finally {
    openButton.disabled = false;
    closeButton.disabled = false;
  }
}

async function updateDashboard() {
  try {
    const response = await fetch('/data', {
      cache: 'no-store'
    });

    if (!response.ok) {
      throw new Error('Request failed');
    }

    const data = await response.json();

    document.getElementById('lidStatus').textContent =
      data.lidStatus;

    document.getElementById('binStatus').textContent =
      data.binStatus;

    if (data.wastePercent < 0) {
      document.getElementById('percentage').textContent =
        '--%';

      document.getElementById('progressFill').style.width =
        '0%';
    } else {
      document.getElementById('percentage').textContent =
        data.wastePercent + '%';

      document.getElementById('progressFill').style.width =
        data.wastePercent + '%';
    }

    document.getElementById('errorMessage').textContent =
      data.wastePercent < 0
        ? 'Unable to read waste-level sensor.'
        : '';

  } catch (error) {
    document.getElementById('errorMessage').textContent =
      'Unable to retrieve data from ESP32.';
  }
}

updateDashboard();
setInterval(updateDashboard, 1000);
</script>
</body>
</html>
)rawliteral";

// ---------------- WEB SERVER HANDLERS ----------------
void handleRoot() {
  server.send_P(200, "text/html", dashboardHTML);
}

void handleData() {
  String json = "{";

  json += "\"wastePercent\":" + String(wastePercent) + ",";
  json += "\"wasteDistance\":" + String(wasteDistance, 1) + ",";
  json += "\"binStatus\":\"" + binStatus + "\",";
  json += "\"lidStatus\":\"";
  json += lidOpen ? "OPEN" : "CLOSED";
  json += "\"}";

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

// ---------------- MANUAL LID CONTROL HANDLERS ----------------
void handleOpenLid() {
  openLid();
server.send(200, "application/json",
            "{\"success\":true,\"message\":\"Open command received\"}");}

void handleCloseLid() {
  lidOpen = false;
  lidCloseTime = 0;
  servoMotor.write(0);
  Serial.println("LID CLOSED (dashboard command)");
server.send(200, "application/json",
            "{\"success\":true,\"message\":\"Close command received\"}");}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  pinMode(LID_TRIG, OUTPUT);
  pinMode(LID_ECHO, INPUT);

  pinMode(WASTE_TRIG, OUTPUT);
  pinMode(WASTE_ECHO, INPUT);

  Wire.begin(LCD_SDA, LCD_SCL);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Smart Trash Bin");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  servoMotor.setPeriodHertz(50);
  servoMotor.attach(SERVO_PIN, 500, 2400);
  servoMotor.write(0);

  // ESP32 creates its own Wi-Fi network
  WiFi.mode(WIFI_AP);

  if (WiFi.softAP(apSSID, apPassword)) {
    Serial.println("Wi-Fi access point started.");
    Serial.print("Network: ");
    Serial.println(apSSID);
    Serial.print("Connect phone to Wi-Fi: ");
    Serial.println(apSSID);
    Serial.print("Then open dashboard: http://");
    Serial.println(WiFi.softAPIP());
    Serial.println("Note: This local AP may show 'No internet'; that is expected.");
  } else {
    Serial.println("Failed to start Wi-Fi access point.");
  }

  // Initial sensor reading
  updateWasteLevel();
  updateLCD();

  // Web page and sensor-data endpoint
  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.on("/lid/open", HTTP_POST, handleOpenLid);
  server.on("/lid/close", HTTP_POST, handleCloseLid);
  server.begin();

  lcd.clear();

  Serial.println("Web server started.");
}

// ---------------- MAIN LOOP ----------------
void loop() {
  server.handleClient();

  unsigned long now = millis();

  if (now - lastLidCheck >= LID_CHECK_INTERVAL) {
    lastLidCheck = now;
    checkLid();
  }

  if (now - lastWasteUpdate >= WASTE_UPDATE_INTERVAL) {
    lastWasteUpdate = now;

    updateWasteLevel();
    updateLCD();
  }
}

