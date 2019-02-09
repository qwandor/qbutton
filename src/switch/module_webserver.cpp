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

#include "logging.h"
#include "sinric.h"
#include "streamutils.h"

#include <Arduino.h>
#include <ESP8266WebServer.h>

void module_handle_root_args(ESP8266WebServer &server, String &error) {
  const String &new_sinric_api_key = server.arg("sinric_api_key");
  if (new_sinric_api_key.length() > 0) {
    if (write_line_to_file("/sinric_api_key.txt", new_sinric_api_key.c_str())) {
      sinric_api_key = new_sinric_api_key;
      sinric_connect();
    } else {
      LOGLN("Failed to save new API key.");
      error = "Failed to save new API key.";
    }
  }

  // Toggle switches
  for (size_t i = 0; i < num_switches; ++i) {
    if (server.hasArg(String("on") + i)) {
      switch_state[i] = true;
      update_switch(i);
      send_switch_state(i);
      break;
    } else if (server.hasArg(String("off") + i)) {
      switch_state[i] = false;
      update_switch(i);
      send_switch_state(i);
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
}

String module_root_output() {
  String page = String("<h2>Sinric</h2>") +
    "<form method=\"post\" action=\"/\">" +
    "API key: <input type=\"text\" name=\"sinric_api_key\" value=\"" + sinric_api_key + "\"/><br/>" +
    "<input type=\"submit\" value=\"Update API key\"/>" +
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
  return page;
}
