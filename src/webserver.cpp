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
#include "main.h"
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
  // If a new SSID and password have been sent, save them.
  if (new_ssid.length() > 0) {
    if (!write_wifi_config(new_ssid, new_password)) {
      error = "Failed to write WiFi config.";
    }
  }

  const String &new_server_hostname = server.arg("server_hostname");
  const String &new_server_port = server.arg("server_port");
  if (new_server_hostname.length() > 0) {
    serverHostname = new_server_hostname;
    serverPort = new_server_port.toInt();
    save_server_details();
    remoteClient.connect(serverHostname, serverPort);
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

  String page = String("<html><head><title>Joist config</title></head><body><h1>Joist config</h1>") +
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
    "<h2>Joist server</h2>" +
    "<form method=\"post\" action=\"/\">" +
    "<p>" + (remoteClient.connected() ? "Connected" : "Not connected") + "</p>" +
    "Hostname: <input type=\"text\" name=\"server_hostname\" value=\"" + serverHostname + "\"/><br/>" +
    "Port: <input type=\"text\" name=\"server_port\" value=\"" + serverPort + "\"/><br/>" +
    "<input type=\"submit\" value=\"Update server details\"/>" +
    "</form>" +
    "</body></html>";
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
