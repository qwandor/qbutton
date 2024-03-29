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

#include "sinric.h"

#include "config.h"
#include "logging.h"
#include "streamutils.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <StreamString.h>
#include <WebSocketsClient.h>

const char *host = "iot.sinric.com";

const uint8_t switch_pins[] = SWITCH_PINS;
const size_t num_switches = sizeof(switch_pins);
const char *const switch_names[num_switches] = SWITCH_NAMES;

String sinric_api_key;
bool switch_inverted[num_switches];
bool switch_initial_state[num_switches];
String switch_ids[num_switches];
bool switch_state[num_switches];
int switch_brightness[num_switches];

static WebSocketsClient websocket;
static bool websocket_connected = false;
static uint64_t heartbeat_timestamp = 0;

bool save_switch_config() {
  bool status1 = write_bools_to_file("/switch_inverted.txt", switch_inverted, num_switches);
  bool status2 = write_bools_to_file("/switch_initial_state.txt", switch_initial_state, num_switches);
  return status1 && status2;
}

bool load_switch_config() {
  bool status1 = read_bools_from_file("/switch_inverted.txt", switch_inverted, num_switches);
  bool status2 = read_bools_from_file("/switch_initial_state.txt", switch_initial_state, num_switches);
  return status1 && status2;
}

bool load_switch_ids() {
  return read_strings_from_file("/switch_ids.txt", switch_ids, num_switches);
}

bool save_switch_ids() {
  return write_strings_to_file("/switch_ids.txt", switch_ids, num_switches);
}

/**
 * Update the physical state of the switch at the given index to match the current values from `switch_state` and `switch_brightness`.
 */
void update_switch(size_t i) {
  if (switch_state[i]) {
    if (switch_brightness[i] == 100) {
      digitalWrite(switch_pins[i], switch_inverted[i] ? LOW : HIGH);
    } else {
      // ESP8266 uses a 0-1023 range for PWM rather than the standard Arduino 0-255.
      int scaled_value = 1023 * switch_brightness[i] / 100;
      if (switch_inverted[i]) {
        scaled_value = 1023 - scaled_value;
      }
      analogWrite(switch_pins[i], scaled_value);
    }
  } else {
    digitalWrite(switch_pins[i], switch_inverted[i] ? HIGH : LOW);
  }
}

void init_switches() {
  for (size_t i = 0; i < num_switches; ++i) {
    pinMode(switch_pins[i], OUTPUT);
    switch_state[i] = switch_initial_state[i];
    switch_brightness[i] = 100;
    update_switch(i);
  }
}

void switch_switch(const String &device_id, bool state) {
  for (size_t i = 0; i < num_switches; ++i) {
    if (device_id == switch_ids[i]) {
      switch_state[i] = state;
      update_switch(i);
    }
  }
}

void set_brightness(const String &device_id, int brightness) {
  for (size_t i = 0; i < num_switches; ++i) {
    if (device_id == switch_ids[i]) {
      switch_brightness[i] = brightness;
      update_switch(i);
    }
  }
}

void set_power_state_on_server(String device_id, bool state) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["deviceId"] = device_id;
  root["action"] = "setPowerState";
  root["value"] = state ? "ON" : "OFF";
  StreamString databuf;
  root.printTo(databuf);

  websocket.sendTXT(databuf);
  LOG("Sending state for ");
  LOG(device_id);
  LOG(": ");
  LOGLN(state);
}

void send_switch_state(size_t i) {
  if (switch_ids[i].length() > 0) {
    set_power_state_on_server(switch_ids[i], (digitalRead(switch_pins[i]) == HIGH) ^ switch_inverted[i]);
  }
}

void send_switch_states() {
  for (size_t i = 0; i < num_switches; ++i) {
    send_switch_state(i);
  }
}

void websocket_event(WStype_t type, uint8_t *payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      websocket_connected = false;
      LOGLN("Websocket disconnected");
      break;
    case WStype_CONNECTED: {
        websocket_connected = true;
        LOG("Websocket connected: ");
        LOGLN((const char *) payload);
        send_switch_states();
      }
      break;
    case WStype_TEXT: {
        LOG("Websocket got: ");
        LOGLN((const char *) payload);

        DynamicJsonBuffer json_buffer;
        JsonObject& json = json_buffer.parseObject((char*)payload);
        String device_id = json["deviceId"];
        String action = json["action"];

        if (action == "action.devices.commands.OnOff") {
          String value = json["value"]["on"];
          LOGLN(value);
          switch_switch(device_id, value == "true");
        } else if (action == "action.devices.commands.BrightnessAbsolute") {
          String value = json["value"]["brightness"];
          set_brightness(device_id, value.toInt());
        } else if (action == "test") {
          LOGLN("Websocket got test command");
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("Websocket got binary of length: %u\n", length);
      break;
  }
}

void sinric_setup() {
  sinric_api_key = read_line_from_file("/sinric_api_key.txt");
  load_switch_config();
  load_switch_ids();
  init_switches();
}

void sinric_connect() {
  websocket.begin(host, 80, "/");
  websocket.onEvent(websocket_event);
  websocket.setAuthorization("apikey", sinric_api_key.c_str());
  websocket.setReconnectInterval(5000);
}

void sinric_loop() {
  websocket.loop();

  if (websocket_connected) {
    uint64_t now = millis();

    // Send heartbeat in order to avoid disconnections.
    if ((now - heartbeat_timestamp) > HEARTBEAT_INTERVAL) {
      heartbeat_timestamp = now;
      websocket.sendTXT("H");
    }
  }
}
