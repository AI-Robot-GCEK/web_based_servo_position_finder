#include <Arduino.h>

#include <WiFi.h>
#include <ESP32Servo.h>
#include <WebServer.h>


const char* ssid = "pi";
const char* password = "12345678";

Servo myServo;
int servoPin = 15;
int servoPosition = 90; 

WebServer server(80);

// HTML content for the web interface
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Servo Position Control</title>
  <script>
    function updateServo(value) {
      document.getElementById('position').innerText = value;
      fetch(`/setServo?position=${value}`);
    }
  </script>
</head>
<body>
  <h1>Servo Position Control</h1>
  <input type="range" min="0" max="180" value="90" id="slider" oninput="updateServo(this.value)">
  <p>Position: <span id="position">90</span></p>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleSetServo() {
  if (server.hasArg("position")) {
    servoPosition = server.arg("position").toInt();
    myServo.write(servoPosition);
    Serial.print("Servo position set to: ");
    Serial.println(servoPosition);
    server.send(200, "text/plain", "Servo position updated");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
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

  myServo.attach(servoPin);
  myServo.write(servoPosition);

  server.on("/", handleRoot);
  server.on("/setServo", handleSetServo);

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient(); // Handle client requests
}