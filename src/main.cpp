#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_PWMServoDriver.h>
#include <WebServer.h>


const char* ssid = "jioairfiber";
const char* password = "7559016538";

#define SERVO_MIN_PULSE_COUNT 102
#define SERVO_MAX_PULSE_COUNT 512
#define SERVO_UPDATE_FREQ 50

Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);

WebServer server(80);

// HTML content for the web interface
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Servo Position Control</title>
  <style>
    // .slider-container {
    //   margin: 10px 0;
    //   display: flex;
    //   flex-direction: column;
    //   align-items: flex-start;
    // }
    // .slider-container span {
    //   margin-left: 10px;
    // }
  </style>
  <script>
    function updateServo(id, value) {
      document.getElementById('position' + id).innerText = value;
      fetch(`/setServoAngle?id=${id}&angle=${value}`);
    }
  </script>
</head>
<body>
  <h1>Servo Position Control</h1>
)rawliteral";

const char* htmlSliders = R"rawliteral(
<div class="slider-container">
  Servo %d: <input type="range" min="0" max="180" value="%d" id="slider%d" 
  oninput="updateServo(%d, this.value)">
  <span>Position: <span id="position%d">%d</span></span>
</div>
)rawliteral";

uint8_t servoPositions[16] = {90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90};

void handleRoot() {
  String html = htmlPage;
  char sliderHtml[200];
  for (int i = 0; i < 16; i++) {
    sprintf(sliderHtml, htmlSliders, i + 1, servoPositions[i], i, i, i, servoPositions[i]);
    html += sliderHtml;
  }
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSetServoAngle() {
  if (server.hasArg("id") && server.hasArg("angle")) {
    uint8_t id = server.arg("id").toInt();
    uint8_t angle = server.arg("angle").toInt();
    if (id >= 0 && id < 16 && angle >= 0 && angle <= 180) {
      servoPositions[id] = angle;
      int pulse = map(angle, 0, 180, SERVO_MIN_PULSE_COUNT, SERVO_MAX_PULSE_COUNT);
      board1.setPWM(id, 0, pulse);
      String response = "Updated servo " + String(id) + " to angle " + String(angle);
      server.send(200, "text/plain", response);
      return;
    }
  }
  server.send(400, "text/plain", "Bad Request - Use: /setServoAngle?id=X&angle=Y (where X=0-15, Y=0-180)");
}

void handleSetServoPulse() {
  if (server.hasArg("id") && server.hasArg("pulse")) {
    uint8_t id = server.arg("id").toInt();
    uint16_t pulse = server.arg("pulse").toInt();
    if (id >= 0 && id < 16 && pulse >= SERVO_MIN_PULSE_COUNT && pulse <= SERVO_MAX_PULSE_COUNT) {
      int angle = map(pulse, SERVO_MIN_PULSE_COUNT, SERVO_MAX_PULSE_COUNT, 0, 180);
      servoPositions[id] = angle;
      board1.setPWM(id, 0, pulse);
      String response = "Updated servo " + String(id) + " to pulse " + String(pulse);
      server.send(200, "text/plain", response);
      return;
    }
  }
  server.send(400, "text/plain", "Bad Request - Use: /setServoPulse?id=X&pulse=Y (where X=0-15, Y=102-512)");
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());

  board1.begin();
  board1.setPWMFreq(SERVO_UPDATE_FREQ);

  server.on("/", handleRoot);
  server.on("/setServoAngle", handleSetServoAngle);
  server.on("/setServoPulse", handleSetServoPulse);

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient(); // Handle client requests
}