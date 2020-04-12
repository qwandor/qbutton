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

#include "main.h"

#include "config.h"
#include "logging.h"
#include "streamutils.h"
#include "webserver.h"
#include "wifi.h"
#include "bugC.h"

#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <M5StickC.h>
#include <SPIFFS.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

class Motor {
public:
  Motor(uint8_t posA, uint8_t posB, bool inverted): _posA(posA), _posB(posB), _inverted(inverted) {}

  void brake() {
    BugCSetSpeed(_posA, 0);
    BugCSetSpeed(_posB, 0);
  }

  void turn(bool forwards, float speed) {
    int8_t speedScaled = (forwards != _inverted) ? speed * 100 : speed * -100;
    BugCSetSpeed(_posA, speedScaled);
    BugCSetSpeed(_posB, speedScaled);
  }

private:
  uint8_t _posA;
  uint8_t _posB;
  bool _inverted;
};

String serverHostname;
uint16_t serverPort;

WiFiServer localServer(LOCAL_SERVER_PORT);
WiFiClient localClient;
WiFiClient remoteClient;

Motor leftWheel(FRONT_LEFT, REAR_LEFT, false);
Motor rightWheel(FRONT_RIGHT, REAR_RIGHT, true);

bool load_server_details() {
  File file = SPIFFS.open("/server.txt", "r");
  if (!file) {
    LOGLN("Failed to open /server.txt for reading.");
    return false;
  }
  serverHostname = file.readStringUntil(' ');
  serverPort = file.readStringUntil('\n').toInt();
  file.close();
  return true;
}

bool save_server_details() {
  File file = SPIFFS.open("/server.txt", "w");
  if (!file) {
    LOGLN("Failed to open /server.txt for writing.");
    return false;
  }
  file.print(serverHostname);
  file.print(' ');
  file.print(serverPort);
  // Don't use println, because it adds '\r' characters which we don't want.
  file.print('\n');
  file.close();
  return true;
}

//////////////////
// Entry points //
//////////////////

void setup() {
  M5.begin();
  Wire.begin(0, 26, 400000);
  M5.Axp.SetChargeCurrent(CURRENT_360MA);

  Serial.begin(115200);
  Serial.println();
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  SPIFFS.begin();
  load_server_details();

  // No point trying to connect to the server if we are running in AP mode.
  if (wifi_setup() && serverHostname.length() > 0) {
    remoteClient.connect(serverHostname.c_str(), serverPort);
  }

  localServer.begin();
  localServer.setNoDelay(true);

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
    BugCSetColor(0, 0);
  } else if (direction == "f") {
    BugCSetColor(0x00ff00, 0x00ff00);
		leftWheel.turn(true, speed);
		rightWheel.turn(true, speed);
  } else if (direction == "b") {
    BugCSetColor(0x0000ff, 0x0000ff);
		leftWheel.turn(false, speed);
		rightWheel.turn(false, speed);
  } else if (direction == "l") {
    BugCSetColor(0x0000ff, 0x00ff00);
		leftWheel.turn(false, speed);
		rightWheel.turn(true, speed);
  } else if (direction == "r") {
    BugCSetColor(0x00ff00, 0x0000ff);
		leftWheel.turn(true, speed);
		rightWheel.turn(false, speed);
  } else if (direction == "fl") {
    BugCSetColor(0x000000, 0x00ff00);
		leftWheel.brake();
		rightWheel.turn(true, speed);
  } else if (direction == "fr") {
    BugCSetColor(0x00ff00, 0x000000);
		leftWheel.turn(true, speed);
		rightWheel.brake();
  } else if (direction == "bl") {
    BugCSetColor(0x000000, 0x0000ff);
		leftWheel.brake();
		rightWheel.turn(false, speed);
  } else if (direction == "br") {
    BugCSetColor(0x0000ff, 0x000000);
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

void loop() {
  M5.update();
  webserver_loop();
  #if OTA_UPDATE
  ArduinoOTA.handle();
  #endif

  if (!remoteClient) {
    if (serverHostname.length() > 0) {
      remoteClient.connect(serverHostname.c_str(), serverPort);
    }
  } else if (remoteClient.available() > 5) {
    String line = remoteClient.readStringUntil('\n');
    processCommand(line);
  }

  if (!localClient) {
    localClient = localServer.available();
    if (localClient) {
      LOG("Got new client ");
      LOGLN(localClient.remoteIP());
    }
  } else if (localClient.available() > 5) {
    String line = localClient.readStringUntil('\n');
    processCommand(line);
  }
}
