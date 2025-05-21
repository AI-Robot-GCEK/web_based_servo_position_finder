#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "jioairfiber";
const char* password = "7559016538";

// MG995 servo specifications:
// - Operating angle: 0° to 180°
// - Pulse width: 500µs to 2500µs

#define SERVO_ANGLE_MIN 0
#define SERVO_ANGLE_MAX 180
#define SERVO_MIN  102   // .5ms
#define SERVO_MAX  512   // 2.5ms 
#define SERVO_FREQ 50
#define CONTROLLER_I2C_ADDR 0x41
#define SERVER_PORT 80 

uint16_t get_pulse(uint8_t _angle){
    _angle = constrain(_angle, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX);
    uint16_t _pulse = map(_angle, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX, SERVO_MIN, SERVO_MAX);
    Serial.print("Angle: "); Serial.print(_angle);
    Serial.print(" pulse: "); Serial.println(_pulse);
    return _pulse;
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
#define LA9_INITIAL_POSITION 0
#define LA10_INITIAL_POSITION 10
#define LA11_INITIAL_POSITION 141
#define LA12_INITIAL_POSITION 180
#define LA13_INITIAL_POSITION 170
#define LA14_INITIAL_POSITION 39
#define LA15_INITIAL_POSITION 99
#define LA16_INITIAL_POSITION 99

int servoPositions[16] = {
    LA1_INITIAL_POSITION, LA2_INITIAL_POSITION, LA3_INITIAL_POSITION, LA4_INITIAL_POSITION,
    LA5_INITIAL_POSITION, LA6_INITIAL_POSITION, LA7_INITIAL_POSITION, LA8_INITIAL_POSITION,
    LA9_INITIAL_POSITION, LA10_INITIAL_POSITION, LA11_INITIAL_POSITION, LA12_INITIAL_POSITION,
    LA13_INITIAL_POSITION, LA14_INITIAL_POSITION, LA15_INITIAL_POSITION, LA16_INITIAL_POSITION
};

Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(CONTROLLER_I2C_ADDR);       // called this way, it uses the default address 0x40   

WebServer server(SERVER_PORT);

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
        LA%d: <input type="range" min="0" max="180" value="%d" id="slider%d" 
        oninput="updateServo(%d, this.value)">
        <span>Position: <span id="position%d">%d</span></span>
    </div>
)rawliteral";

void handleRoot() {
    String html = htmlPage;
    char sliderHtml[1000];
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
        
        // Validate parameters
        if (id >= 0 && id < 16 && position >= SERVO_ANGLE_MIN && position <= SERVO_ANGLE_MAX) {
            servoPositions[id] = position;
            board1.setPWM(id, 0, get_pulse(position));  // Fix: Use get_pulse instead of raw position
            String response = "Updated servo " + String(id) + " to position " + String(position);
            server.send(200, "text/plain", response);
            return;
        }
    }
    server.send(400, "text/plain", "Bad Request - Use: /setServo?id=X&position=Y (where X=0-15, Y=0-180)");
}

void setup() {
    Serial.begin(115200);
    board1.begin();
    board1.setPWMFreq(SERVO_FREQ);

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
        board1.setPWM(i, 0, get_pulse(servoPositions[i]));  // Fix: Use get_pulse instead of angleToPulse
    }
    
    server.on("/", handleRoot);
    server.on("/setServo", handleSetServo);
    server.begin();
    Serial.println("Web server started");
}

void loop() {
    server.handleClient(); // Handle client requests
}
