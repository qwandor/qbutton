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

#include "module_webserver.h"

#include "assistant.h"
#include "logging.h"
#include "streamutils.h"

#include <Arduino.h>
#include <ESP8266WebServer.h>

// Write the given command to /command.txt.
bool update_command(const char *command) {
  LOG("Updating command to \"");
  LOG(command);
  LOGLN("\"");

  return write_line_to_file("/command.txt", command);
}

String load_command() {
  return read_line_from_file("/command.txt");
}

void module_handle_root_args(ESP8266WebServer &server, String &error) {
  const String &new_command = server.arg("command");
  if (server.hasArg("update") && new_command.length() > 0) {
    update_command(new_command.c_str());
  } else if (server.hasArg("test")) {
    update_command(new_command.c_str());
    auth_and_send_request(new_command);
  }
}

String module_root_output() {
  return String("<h2>Google account</h2>") +
    "<a href=\"https://accounts.google.com/o/oauth2/v2/auth?client_id=" + client_id +
    "&scope=https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fassistant-sdk-prototype&access_type=offline&response_type=code&redirect_uri=http://" +
    WiFi.localIP().toString() + "/oauth&device_id=device_id&device_name=device_name\">Set account</a>" +
    "<h2>Command</h2>" +
    "<form method=\"post\" action=\"/\">" +
    "Command: <input type=\"text\" name=\"command\" value=\"" + load_command() + "\">" +
    "<input type=\"submit\" name=\"update\" value=\"Update command\"/>" +
    "<input type=\"submit\" name=\"test\" value=\"Update and test command\"/></form>";
}
