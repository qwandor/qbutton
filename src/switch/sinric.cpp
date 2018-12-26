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

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <StreamString.h>
#include <WebSocketsClient.h>

const char *host = "iot.sinric.com";
const char *api_key = SINRIC_API_KEY;

const uint8_t switch_pins[] = SWITCH_PINS;
const size_t num_switches = sizeof(switch_pins);
const char *const switch_names[num_switches] = SWITCH_NAMES;
static const bool switch_inverted[num_switches] = SWITCH_INVERTED;
static const bool switch_initial_state[num_switches] = SWITCH_INITIAL_STATE;

String switch_ids[num_switches];
static bool switch_state[num_switches];
static int switch_brightness[num_switches];
static WebSocketsClient websocket;
static bool websocket_connected = false;
static uint64_t heartbeat_timestamp = 0;

bool load_switch_ids() {
  File file = SPIFFS.open("/switch_ids.txt", "r");
  if (!file) {
    LOGLN("Failed to open /switch_ids.txt for reading.");
    return false;
  }
  for (size_t i = 0; i < num_switches; ++i) {
    switch_ids[i] = file.readStringUntil('\n');
  }
  file.close();
  return true;
}

bool save_switch_ids() {
  File file = SPIFFS.open("/switch_ids.txt", "w");
  if (!file) {
    LOGLN("Failed to open /switch_ids.txt for writing.");
    return false;
  }
  for (size_t i = 0; i < num_switches; ++i) {
    file.print(switch_ids[i]);
    file.print('\n');
  }
  file.close();
  return true;
}

/**
 * Update the physical state of the switch at the given index to match the current values from `switch_state` and `switch_brightness`.
 */
void update_switch(size_t i) {
  if (switch_state[i]) {
    if (switch_brightness[i] == 100) {
      digitalWrite(switch_pins[i], switch_inverted[i] ? LOW : HIGH);
    } else {
      int scaled_value = 255 * switch_brightness[i] / 100;
      if (switch_inverted[i]) {
        scaled_value = 255 - scaled_value;
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

void send_switch_states() {
  for (size_t i = 0; i < num_switches; ++i) {
    set_power_state_on_server(switch_ids[i], (digitalRead(switch_pins[i]) == HIGH) ^ switch_inverted[i]);
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
  load_switch_ids();
  init_switches();
}

void sinric_connect() {
  websocket.begin(host, 80, "/");
  websocket.onEvent(websocket_event);
  websocket.setAuthorization("apikey", api_key);
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
