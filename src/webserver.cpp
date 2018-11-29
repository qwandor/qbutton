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

#include "assistant.h"
#include "config.h"
#include "logging.h"
#include "rf.h"
#include "streamutils.h"
#include "ButtonCommand.h"

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <Vector.h>

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
  const String &new_command = server.arg("new_command");
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
  // Delete and update commands
  bool updated_commands = false;
  for (uint i = 0; i < button_commands.size(); ++i) {
    if (server.hasArg(String("delete") + i)) {
      button_commands.remove(i);
      updated_commands = true;
      break;
    } else if (server.hasArg("update") && server.hasArg(String("command") + i)) {
      button_commands[i].command = server.arg(String("command") + i);
      updated_commands = true;
    } else if (server.hasArg(String("test") + i)) {
      // Test running the command
      auth_and_send_request(button_commands[i].command);
    }
  }
  if (updated_commands) {
    save_commands();
  }
  if (new_command.length() > 0 && button_commands.size() < MAX_COMMANDS) {
    // Add a new command: first wait for the button to be pressed
    RfCode code;
    if (learn_code(&code)) {
      button_commands.push_back(ButtonCommand(code, new_command));
      save_commands();
    } else {
      error = "Failed to learn code.";
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

  String page = String("<html><head><title>qButton config</title></head><body><h1>qButton config</h1>") +
    "<p style=\"color: red;\">" + error + "</p>" +
    "<h2>Admin password</h2>" +
    "<form method=\"post\" action=\"/\">" +
    "<input type=\"text\" name=\"admin_password\" value=\"" + admin_password + "\"/><br/>" +
    "<input type=\"submit\" value=\"Update admin password\"/>" +
    "</form>" +
    "<h2>Google account</h2>" +
    "<a href=\"https://accounts.google.com/o/oauth2/v2/auth?client_id=" + client_id +
    "&scope=https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fassistant-sdk-prototype&access_type=offline&response_type=code&redirect_uri=http://" +
    WiFi.localIP().toString() + "/oauth&device_id=device_id&device_name=device_name\">Set account</a>" +
    "<h2>WiFi config</h2>" +
    "<form method=\"post\" action=\"/\">" +
    "SSID: <input type=\"text\" name=\"ssid\" value=\"" + ssid + "\"/><br/>" +
    "Password: <input type=\"text\" name=\"password\" value=\"" + password + "\"/><br/>" +
    "<input type=\"submit\" value=\"Update WiFi config\"/>" +
    "</form><h2>Commands</h2><form method=\"post\" action=\"/\"><ul>";
  for (uint i = 0; i < button_commands.size(); ++i) {
    page = page + "<li>" + button_commands[i].code.to_hex() +
      "<input type=\"text\" name=\"command" + i + "\" value=\"" + button_commands[i].command + "\"/>" +
      "<input type=\"submit\" name=\"delete" + i + "\" value=\"Delete\"/>" +
      "<input type=\"submit\" name=\"test" + i + "\" value=\"Test command\"/></li>";
  }
  page = page + "</ul>" +
     "<input type=\"submit\" name=\"update\" value=\"Update commands\"/></form>";
  if (button_commands.size() < MAX_COMMANDS) {
    page += "<form method=\"post\" action=\"/\"><input type=\"text\" name=\"new_command\"/><input type=\"submit\" value=\"Add command\"/></form>";
  }
  page += "</body></html>";
  server.send(200, "text/html", page);
}

void handle_oauth() {
  const String &code = server.arg("code");
  if (code.length() > 0) {
    bool success = oauth_with_code(code);
    if (success) {
      server.send(200, "text/html", "<html><head><title>Auth</title></head><body><p>Success</p><a href=\"/\">Home</a></body></html>");
    } else {
      server.send(200, "text/html", "<html><head><title>Auth</title></head><body><p>Failure</p><a href=\"/\">Home</a></body></html>");
    }
  } else {
    server.send(200, "text/html", String("<html><head><title>Error</title></head><body><h1>Authentication error</h1><p>") +
      server.arg("error") + "</p></body></html>");
  }
}

// Run web server to let the user authenticate their account.
void start_webserver() {
  admin_password = read_line_from_file("/password.txt");

  server.on("/", handle_root);
  server.on("/oauth", handle_oauth);
  server.begin();
  LOGLN("HTTP server started");
}

void webserver_loop() {
  server.handleClient();
}
