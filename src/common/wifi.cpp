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

#include "wifi.h"

#include "config.h"
#include "logging.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <time.h>

void sync_time() {
  // Set clock using SNTP. This is necessary to verify SSL certificates.
  LOGLN("Setting time using SNTP");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 3600 * 48) {
    delay(10);
    LOG(".");
    now = time(nullptr);
  }
  LOGLN("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  LOG("Current time: ");
  LOGLN(asctime(&timeinfo));
}

bool wifi_connect() {
  // Read SSID and password from file.
  File wifiFile = SPIFFS.open("/wifi.txt", "r");
  if (!wifiFile) {
    LOGLN("Failed to open /wifi.txt for reading.");
    return false;
  }
  String ssid = wifiFile.readStringUntil('\n');
  String password = wifiFile.readStringUntil('\n');
  wifiFile.close();

  LOG("connecting to '");
  LOG(ssid.c_str());
  LOG("' with password '");
  LOG(password.c_str());
  LOGLN("'");
  WiFi.mode(WIFI_STA);
  // Use mDNS hostname for DHCP too
  WiFi.hostname(MDNS_HOSTNAME);
  WiFi.begin(ssid.c_str(), password.c_str());
  // Try to connect for 5 seconds
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; ++i) {
    delay(500);
    LOG(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    LOGLN("Couldn't connect");
    WiFi.mode(WIFI_OFF);
    return false;
  }
  LOGLN("");
  LOGLN("WiFi connected");
  LOGLN("IP address: ");
  LOGLN(WiFi.localIP());

  sync_time();

  return true;
}

// Connect to the configured network if possible, or else run as an access point.
// Returns false if it was unable to connect to the configured network and so is in AP mode.
bool wifi_setup() {
  if (wifi_connect()) {
    #if NETWORK_LOGGING
    log_server.begin();
    log_server.setNoDelay(true);
    #endif

    return true;
  }

  // Couldn't connect, try being an AP.
  IPAddress local_ip(192, 168, 0, 1);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(MDNS_HOSTNAME);

  #if NETWORK_LOGGING
  log_server.begin();
  log_server.setNoDelay(true);
  #endif

  LOG("Running AP ");
  LOG(MDNS_HOSTNAME);
  LOGLN(". Local IP address:");
  LOGLN(WiFi.softAPIP());

  return false;
}
