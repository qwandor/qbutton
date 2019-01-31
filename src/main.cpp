/*
Copyright 2018 Google LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */

#include "config.h"
#include "logging.h"
#include "streamutils.h"
#include "webserver.h"
#include "wifi.h"

#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <FS.h>

class Motor {
public:
  Motor(int forwardPin, int reversePin, int pwmPin): _forwardPin(forwardPin), _reversePin(reversePin), _pwmPin(pwmPin) {}

  void brake() {
    analogWrite(_pwmPin, 1023);
    digitalWrite(_forwardPin, LOW);
    digitalWrite(_reversePin, LOW);
  }

  void turn(bool forwards, float speed) {
    digitalWrite(_forwardPin, forwards ? HIGH : LOW);
    digitalWrite(_reversePin, forwards ? LOW : HIGH);
    analogWrite(_pwmPin, speed * 1023);
  }

private:
  int _forwardPin;
  int _reversePin;
  int _pwmPin;
};

WiFiServer local_server(LOCAL_SERVER_PORT);
Motor leftWheel(LEFT_FORWARD, LEFT_REVERSE, LEFT_PWM);
Motor rightWheel(RIGHT_FORWARD, RIGHT_REVERSE, RIGHT_PWM);

//////////////////
// Entry points //
//////////////////

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(LED_PIN, OUTPUT);
  pinMode(LEFT_PWM, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(LEFT_REVERSE, OUTPUT);
  pinMode(RIGHT_PWM, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(RIGHT_REVERSE, OUTPUT);

  digitalWrite(LEFT_PWM, LOW);
  digitalWrite(RIGHT_PWM, LOW);

  digitalWrite(LED_PIN, LOW);

  SPIFFS.begin();

  // No point trying to connect to the server if we are running in AP mode.
  if (wifi_setup()) {
    // TODO: Connect to server if configured
  }

  local_server.begin();
  local_server.setNoDelay(true);

  if (!MDNS.begin(MDNS_HOSTNAME)) {
    LOGLN("Error starting mDNS");
  }

  start_webserver();
  #if OTA_UPDATE
  ArduinoOTA.setHostname(MDNS_HOSTNAME);
  ArduinoOTA.begin();
  #endif
}

void drive(String direction, float speed) {
  if (direction == "brake") {
    leftWheel.brake();
    rightWheel.brake();
  } else if (direction == "f") {
		leftWheel.turn(true, speed);
		rightWheel.turn(true, speed);
  } else if (direction == "b") {
		leftWheel.turn(false, speed);
		rightWheel.turn(false, speed);
  } else if (direction == "l") {
		leftWheel.turn(false, speed);
		rightWheel.turn(true, speed);
  } else if (direction == "r") {
		leftWheel.turn(true, speed);
		rightWheel.turn(false, speed);
  } else if (direction == "fl") {
		leftWheel.brake();
		rightWheel.turn(true, speed);
  } else if (direction == "fr") {
		leftWheel.turn(true, speed);
		rightWheel.brake();
  } else if (direction == "bl") {
		leftWheel.brake();
		rightWheel.turn(false, speed);
  } else if (direction == "br") {
    leftWheel.turn(false, speed);
    rightWheel.brake();
  } else {
    LOG("Unrecognised direction '");
    LOG(direction);
    LOGLN("'");
  }
}

void runCommand(String direction, float speed, int duration) {
  // Turn LED off while we drive.
  digitalWrite(LED_PIN, HIGH);

  drive(direction, speed);
  delay(duration);
  drive("brake", 0);

  digitalWrite(LED_PIN, LOW);
}

void processCommand(String line) {
  LOG("Got command '");
  LOG(line);
  LOGLN("'");
  int firstSpace = line.indexOf(' ');
  int secondSpace = line.indexOf(' ', firstSpace + 1);
  if (firstSpace == -1 || secondSpace == -1) {
    LOGLN("Invalid command");
    return;
  }
  String direction = line.substring(0, firstSpace);
  float speed = line.substring(firstSpace + 1, secondSpace).toFloat();
  int duration = line.substring(secondSpace + 1).toInt();
  LOG("direction: '");
  LOG(direction);
  LOG("' speed: '");
  LOG(speed);
  LOG("' duration: '");
  LOG(duration);
  LOGLN("'");

  runCommand(direction, speed, duration);
}

WiFiClient local_client;

void loop() {
  webserver_loop();
  #if OTA_UPDATE
  ArduinoOTA.handle();
  #endif

  if (!local_client) {
    local_client = local_server.available();
    if (local_client) {
      LOG("Got new client ");
      LOGLN(local_client.remoteIP());
    }
  } else if (local_client.available() > 5) {
    String line = local_client.readStringUntil('\n');
    processCommand(line);
  }
}
