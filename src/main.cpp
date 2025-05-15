#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "pi";
const char* password = "12345678";

// MG995 servo specifications:
// - Operating angle: 0° to 180°
// - Pulse width: 544µs to 2400µs
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 160
#define SERVOMIN  50    // 544 microseconds
#define SERVOMAX  2400   // 2400 microseconds 

int angleToPulse(int ang) {
    // Constrain input angle to valid range
    ang = constrain(ang, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    int pulse = map(ang, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVOMIN, SERVOMAX);
    Serial.print("Angle: "); Serial.print(ang);
    Serial.print(" pulse: "); Serial.println(pulse);
    return pulse;
}

// Define initial positions for all 16 servos
#define LA1_INITIAL_POSITION 10
#define LA2_INITIAL_POSITION 0
#define LA3_INITIAL_POSITION 16
#define LA4_INITIAL_POSITION 10
#define LA5_INITIAL_POSITION 10
#define LA6_INITIAL_POSITION 10
#define LA7_INITIAL_POSITION  90
#define LA8_INITIAL_POSITION 96
#define LA9_INITIAL_POSITION 170
#define LA10_INITIAL_POSITION 24
#define LA11_INITIAL_POSITION 159
#define LA12_INITIAL_POSITION 13
#define LA13_INITIAL_POSITION 31
#define LA14_INITIAL_POSITION 153
#define LA15_INITIAL_POSITION 89
#define LA16_INITIAL_POSITION 80

int servoPositions[16] = {
    LA1_INITIAL_POSITION, LA2_INITIAL_POSITION, LA3_INITIAL_POSITION, LA4_INITIAL_POSITION,
    LA5_INITIAL_POSITION, LA6_INITIAL_POSITION, LA7_INITIAL_POSITION, LA8_INITIAL_POSITION,
    LA9_INITIAL_POSITION, LA10_INITIAL_POSITION, LA11_INITIAL_POSITION, LA12_INITIAL_POSITION,
    LA13_INITIAL_POSITION, LA14_INITIAL_POSITION, LA15_INITIAL_POSITION, LA16_INITIAL_POSITION
};

Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);       // called this way, it uses the default address 0x40   

WebServer server(80);

// HTML content for the web interface
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Servo Position Control</title>
    <style>
        .slider-container { margin: 10px 0; }
    </style>
    <script>
        function updateServo(id, value) {
            document.getElementById('position' + id).innerText = value;
            fetch(`/setServo?id=${id}&position=${value}`);
        }
    </script>
</head>
<body>
    <h1>Servo Position Control</h1>
)rawliteral";

const char* htmlSliders = R"rawliteral(
    <div class="slider-container">
        LA%d: <input type="range" min="0" max="120" value="%d" id="slider%d" 
        oninput="updateServo(%d, this.value)">
        <span>Position: <span id="position%d">%d</span></span>
    </div>
)rawliteral";

void handleRoot() {
    String html = htmlPage;
    char sliderHtml[500];
    for (int i = 0; i < 16; i++) {
        sprintf(sliderHtml, htmlSliders, i+1, servoPositions[i], i, i, i, servoPositions[i]);
        html += sliderHtml;
    }
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleSetServo() {
    if (server.hasArg("id") && server.hasArg("position")) {
        int id = server.arg("id").toInt();
        int position = server.arg("position").toInt();
        if (id >= 0 && id < 16) {
            servoPositions[id] = position;
            board1.setPWM(id, 0, angleToPulse(position));
            server.send(200, "text/plain", "Updated");
        }
    }
    server.send(400, "text/plain", "Bad Request");
}

void setup() {
    Serial.begin(115200);
    board1.begin();
    board1.setPWMFreq(50);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.println(WiFi.localIP());

    // Initialize all servos to their initial positions
    for (int i = 0; i < 16; i++) {
        board1.setPWM(i, 0, angleToPulse(servoPositions[i]));
    }

    server.on("/", handleRoot);
    server.on("/setServo", handleSetServo);
    server.begin();
    Serial.println("Web server started");
}

void loop() {
    server.handleClient(); // Handle client requests
}
