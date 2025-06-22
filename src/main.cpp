#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>
#if defined(ESP32)
#include <WebServer.h>
#elif defined(ESP8266)
#include <ESP8266WebServer.h>
#endif
#if defined(ESP32)
#include <WiFi.h>
#endif
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <initial-positions.h>

const char *ssid = "jioairfiber";
const char *password = "7559016538";

// MG995 servo specifications:
// - Operating angle: 0° to 180°
// - Pulse width: 500µs to 2500µs

#define SERVO_ANGLE_MIN 0
#define SERVO_ANGLE_MAX 180
#define SERVO_MIN 102 // .5ms
#define SERVO_MAX 512 // 2.5ms
#define SERVO_FREQ 50
#define CONTROLLER_I2C_ADDR 0x40
#define SERVER_PORT 80

uint16_t get_pulse(uint8_t _angle) {
  _angle = constrain(_angle, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX);
  uint16_t _pulse =
      map(_angle, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX, SERVO_MIN, SERVO_MAX);
  Serial.print("Angle: ");
  Serial.print(_angle);
  Serial.print(" pulse: ");
  Serial.println(_pulse);
  return _pulse;
}

int servoPositions[16] = {
    LA1_INITIAL_POSITION, LA2_INITIAL_POSITION, LA3_INITIAL_POSITION,
    RA1_INITIAL_POSITION, RA2_INITIAL_POSITION, RA3_INITIAL_POSITION,
    LH_INITIAL_POSITION,  RH_INITIAL_POSITION,  LL1_INITIAL_POSITION,
    LL2_INITIAL_POSITION, LL3_INITIAL_POSITION, RL1_INITIAL_POSITION,
    RL2_INITIAL_POSITION, RL3_INITIAL_POSITION, LF_INITIAL_POSITION,
    RF_INITIAL_POSITION,
};

Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(
    CONTROLLER_I2C_ADDR); // called this way, it uses the default address 0x40

#if defined(ESP32)
WebServer server(SERVER_PORT);
#elif defined(ESP8266)
ESP8266WebServer server(SERVER_PORT);
#endif

void handleSetServo() {
  if (server.hasArg("id") && server.hasArg("angle")) {
    int id = server.arg("id").toInt();
    int position = server.arg("angle").toInt();

    // Validate parameters
    if (id >= 0 && id < 16 && position >= SERVO_MIN && position <= SERVO_MAX) {
      servoPositions[id] = position;
      board1.setPWM(id, 0, position);
      String response =
          "Updated servo " + String(id) + " to position " + String(position);
      server.send(200, "text/plain", response);
      return;
    }
  }
  server.send(
      400, "text/plain",
      "Bad Request - Use: /setServo?id=X&anglee=Y (where X=0-15, Y=0-180)");
}

void setup() {
  Serial.begin(115200);
  board1.begin();
  board1.setPWMFreq(SERVO_FREQ);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());

  // Initialize all servos to their initial positions
  for (int i = 0; i < 16; i++) {
    board1.setPWM(i, 0, get_pulse(servoPositions[i]));
  }

  server.on("/setServo", handleSetServo);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
  delay(10);
}
