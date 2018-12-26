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

#include "assistant.h"
#include "config.h"
#include "logging.h"
#include "rf.h"
#include "streamutils.h"
#include "webserver.h"
#include "wifi.h"

#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <FS.h>

//////////////////
// Entry points //
//////////////////

void setup() {
  Serial.begin(19200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  SPIFFS.begin();

  if (wifi_setup()) {
    // Turn off the LED if we have successfully connected to WiFi
    digitalWrite(LED_PIN, HIGH);
  }

  if (!MDNS.begin(MDNS_HOSTNAME)) {
    LOGLN("Error starting mDNS");
  }

  load_commands();
  start_webserver();
  #if OTA_UPDATE
  ArduinoOTA.setHostname(MDNS_HOSTNAME);
  ArduinoOTA.begin();
  #endif
  assistant_init();
}

void loop() {
  webserver_loop();
  #if OTA_UPDATE
  ArduinoOTA.handle();
  #endif

  size_t available = Serial.available();
  if (available >= 3) {
    handle_message();
  }
}
