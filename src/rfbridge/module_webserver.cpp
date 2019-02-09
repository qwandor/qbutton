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
#include "config.h"
#include "logging.h"
#include "rf.h"
#include "streamutils.h"
#include "ButtonCommand.h"

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <Vector.h>

void module_handle_root_args(ESP8266WebServer &server, String &error) {
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

  const String &new_command = server.arg("new_command");
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
}

String module_root_output() {
  String page = String("<h2>Google account</h2>") +
    "<a href=\"https://accounts.google.com/o/oauth2/v2/auth?client_id=" + client_id +
    "&scope=https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fassistant-sdk-prototype&access_type=offline&response_type=code&redirect_uri=http://" +
    WiFi.localIP().toString() + "/oauth&device_id=device_id&device_name=device_name\">Set account</a>" +
    "<h2>Commands</h2><form method=\"post\" action=\"/\"><ul>";
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
  return page;
}
