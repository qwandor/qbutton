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

#include "webserver.h"

#include "config.h"
#include "logging.h"
#include "sinric.h"
#include "streamutils.h"

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <time.h>

static ESP8266WebServer server(80);
static String admin_password;

bool write_wifi_config(const String &ssid, const String &password) {
  File wifiFile = SPIFFS.open("/wifi.txt", "w");
  if (!wifiFile) {
    LOGLN("Failed to open /wifi.txt for writing.");
    return false;
  }
  // Don't use println, because it adds '\r' characters which we don't want.
  wifiFile.print(ssid);
  wifiFile.print('\n');
  wifiFile.print(password);
  wifiFile.print('\n');
  wifiFile.close();
  LOGLN("wrote new SSID and password");
  LOGLN(ssid);
  return true;
}

///////////////////////////
// HTTP request handlers //
///////////////////////////

void handle_root() {
  if (admin_password.length() > 0 && !server.authenticate(ADMIN_USERNAME, admin_password.c_str())) {
    server.requestAuthentication(DIGEST_AUTH, ADMIN_REALM);
    return;
  }

  // If a new SSID and password have been sent, save them.
  const String &new_admin_password = server.arg("admin_password");
  const String &new_ssid = server.arg("ssid");
  const String &new_password = server.arg("password");
  String error;
  if (new_admin_password.length() > 0) {
    if (write_line_to_file("/password.txt", new_admin_password.c_str())) {
      admin_password = new_admin_password;
    } else {
      LOGLN("Failed to save new admin password.");
      error = "Failed to save new admin password.";
    }
  }
  if (new_ssid.length() > 0) {
    if (!write_wifi_config(new_ssid, new_password)) {
      error = "Failed to write WiFi config.";
    }
  }

  // Toggle switches
  for (size_t i = 0; i < num_switches; ++i) {
    if (server.hasArg(String("on") + i)) {
      switch_state[i] = true;
      update_switch(i);
      break;
    } else if (server.hasArg(String("off") + i)) {
      switch_state[i] = false;
      update_switch(i);
      break;
    }
  }

  // Update switch IDs.
  if (server.hasArg("update")) {
    bool updated_switch_ids = false;
    for (size_t i = 0; i < num_switches; ++i) {
      if (server.hasArg(String("switch_id") + i)) {
        switch_ids[i] = server.arg(String("switch_id") + i);
        updated_switch_ids = true;
      }
      switch_initial_state[i] = server.hasArg(String("switch_initial") + i);
      switch_inverted[i] = server.hasArg(String("switch_inverted") + i);
      update_switch(i);
    }
    save_switch_config();
    if (updated_switch_ids) {
      save_switch_ids();
    }
  }

  // Read whatever is on disk.
  String ssid, password;
  File wifiFile = SPIFFS.open("/wifi.txt", "r");
  if (!wifiFile) {
    LOGLN("Failed to open /wifi.txt for reading.");
  } else {
    ssid = wifiFile.readStringUntil('\n');
    password = wifiFile.readStringUntil('\n');
    wifiFile.close();
  }

  time_t now = time(nullptr);
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);

  String page = String("<html><head><title>qSwitch config</title></head><body><h1>qSwitch config</h1>") +
    "<p style=\"color: red;\">" + error + "</p>" +
    "<p>Device time: " + asctime(&timeinfo) + "UTC</p>" +
    "<h2>WiFi config</h2>" +
    "<form method=\"post\" action=\"/\">" +
    "SSID: <input type=\"text\" name=\"ssid\" value=\"" + ssid + "\"/><br/>" +
    "Password: <input type=\"text\" name=\"password\" value=\"" + password + "\"/><br/>" +
    "<input type=\"submit\" value=\"Update WiFi config\"/></form>" +
    "<h2>Admin password</h2>" +
    "<form method=\"post\" action=\"/\">" +
    "<input type=\"text\" name=\"admin_password\" value=\"" + admin_password + "\"/><br/>" +
    "<input type=\"submit\" value=\"Update admin password\"/>" +
    "</form>" +
    "<h2>Switch IDs</h2>" +
    "<form method=\"post\" action=\"/\"><table>" +
    "<tr><th>Pin</th><th>Sinric ID</th><th>Initial</th><th>Inverted</th><th>State</th><th>Toggle</th></tr>";
  for (size_t i = 0; i < num_switches; ++i) {
    page = page + "<tr><td>" + switch_names[i] + " (pin" + switch_pins[i] + ")</td>" +
      "<td><input type=\"text\" name=\"switch_id" + i + "\" value=\"" + switch_ids[i] + "\"/></td>" +
      "<td><input type=\"checkbox\" name=\"switch_initial" + i + "\" value=\"1\"" + (switch_initial_state[i] ? " checked" : "") + "/></td>" +
      "<td><input type=\"checkbox\" name=\"switch_inverted" + i + "\" value=\"1\"" + (switch_inverted[i] ? " checked" : "") + "/></td>" +
      "<td>" + (switch_state[i] ? "on" : "off") + " (" + switch_brightness[i] + "%)</td>" +
      "<td><input type=\"submit\" name=\"" + (switch_state[i] ? "off" : "on") + i + "\" value=\"Switch " + (switch_state[i] ? "off" : "on") + "\"/></td></tr>";
  }
  page += "</table><input type=\"submit\" name=\"update\" value=\"Update switches\"/></form>";
  page += "</body></html>";
  server.send(200, "text/html", page);
}

// Run web server to let the user authenticate their account.
void start_webserver() {
  admin_password = read_line_from_file("/password.txt");

  server.on("/", handle_root);
  server.begin();
  LOGLN("HTTP server started");
}

void webserver_loop() {
  server.handleClient();
}
