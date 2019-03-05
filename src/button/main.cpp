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
#include "streamutils.h"
#include "webserver.h"
#include "wifi.h"

#include <ArduinoOTA.h>
#include <DoubleResetDetect.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <FS.h>

DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

//////////////////
// Entry points //
//////////////////

void setup() {
  // Keep power on until we're done
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH);

  bool double_reset = drd.detect();

  Serial.begin(115200);
  Serial.println();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  system_update_cpu_freq(160);

  if (double_reset) {
    LOGLN("detected double reset");
  }

  SPIFFS.begin();

  assistant_init();

  // No point trying to send the Google Assistant request if we are running in AP mode.
  if (wifi_setup()) {
    auth_and_send_request(load_command());

    // If the user double-presses the reset button, skip sleeping so that they can reconfigure it.
    if (!double_reset) {
      // Go to sleep and/or turn off.
      SPIFFS.end();
      LOGLN("sleeping");
      // Power can go off, if we're wired up that way.
      digitalWrite(EN_PIN, LOW);
      // Deep sleep until RESET is taken low.
      ESP.deepSleep(0);

      LOGLN("done sleeping");
      digitalWrite(EN_PIN, HIGH);
      SPIFFS.begin();
    }
  }

  if (!MDNS.begin(MDNS_HOSTNAME)) {
    LOGLN("Error starting mDNS");
  }

  start_webserver();
  #if OTA_UPDATE
  ArduinoOTA.setHostname(MDNS_HOSTNAME);
  ArduinoOTA.begin();
  #endif
}

void loop() {
  webserver_loop();
  #if OTA_UPDATE
  ArduinoOTA.handle();
  #endif
}
