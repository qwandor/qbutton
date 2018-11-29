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

#include "rf.h"

#include "assistant.h"
#include "config.h"
#include "logging.h"
#include "ButtonCommand.h"

#include <Arduino.h>
#include <FS.h>
#include <Vector.h>

static ButtonCommand storage_array[MAX_COMMANDS];
Vector<ButtonCommand> button_commands(storage_array);

// Save all commands to a file.
bool save_commands() {
  File file = SPIFFS.open("/commands.txt", "w");
  if (!file) {
    LOGLN("Failed to open /commands.txt for writing.");
    return false;
  }
  for (uint i = 0; i < button_commands.size(); ++i) {
    file.print(button_commands[i].code.to_hex());
    file.print(' ');
    file.print(button_commands[i].command);
    // Don't use println, because it adds '\r' characters which we don't want.
    file.print('\n');
  }
  file.close();
  return true;
}

bool load_commands() {
  File file = SPIFFS.open("/commands.txt", "r");
  if (!file) {
    LOGLN("Failed to open /commands.txt for reading.");
    return false;
  }
  button_commands.clear();
  while (file.available() > 0) {
    String code_hex = file.readStringUntil(' ');
    String command = file.readStringUntil('\n');
    if (code_hex.length() == 0 || command.length() == 0) {
      break;
    }
    RfCode code = RfCode::from_hex(code_hex);
    button_commands.push_back(ButtonCommand(code, command));
  }
  file.close();
  return true;
}

/////////////////////////
// RF module interface //
/////////////////////////

void send_ack() {
  Serial.write(0xaa);
  Serial.write(0xa0);
  Serial.write(0x55);
  Serial.flush();
}

bool learn_code(RfCode *code) {
  LOGLN("Going into learning mode");
  Serial.write(0xaa);
  Serial.write(0xa1);
  Serial.write(0x55);
  Serial.flush();
  uint8_t buffer[12];
  size_t length = Serial.readBytesUntil(0x55, buffer, 3);
  if (length != 2 || buffer[0] != 0xaa || buffer[1] != 0xa0) {
    LOGLN("Didn't get expected ack, length was ");
    LOGLN(length);
    return false;
  }
  LOGLN("Waiting");
  while (Serial.available() == 0) {
  }
  length = Serial.readBytesUntil(0x55, buffer, 12);
  if (length == 2 && buffer[0] == 0xaa && buffer[1] == 0xa2) {
    LOGLN("Learning timed out");
    send_ack();
    return false;
  } else if (length == 11 && buffer[0] == 0xaa && buffer[1] == 0xa3) {
    LOG("Learned code:");
    for (int i = 2; i < 11; ++i) {
      LOG(" 0x");
      LOGH(buffer[i]);
    }
    LOGLN("");
    // Ignore Tsyn, Tlow, Thigh
    code->copy_from(&buffer[8]);
    send_ack();
    return true;
  } else {
    LOG("Got unexpected response to learn, of length ");
    LOGLN(length);
    return false;
  }
}

void handle_button(const RfCode &code) {
  LOG("Got code ");
  LOGLN(code.to_hex());
  digitalWrite(LED_PIN, LOW);

  // Find any matching commands and run them.
  for (size_t i = 0; i < button_commands.size(); ++i) {
    if (button_commands[i].code == code) {
      LOG("Code matched, sending request: ");
      LOGLN(button_commands[i].command);
      auth_and_send_request(button_commands[i].command);
    }
  }

  digitalWrite(LED_PIN, HIGH);
}

void handle_message() {
  uint8_t buffer[12];
  size_t length = Serial.readBytesUntil(0x55, buffer, 12);
  if (length == 11 && buffer[0] == 0xaa && buffer[1] == 0xa4) {
    send_ack();
    RfCode code;
    // Ignore Tsyn, Tlow, Thigh
    code.copy_from(&buffer[8]);
    handle_button(code);
  } else {
    LOG("Got unexpected message of length ");
    LOGLN(length);
  }
}
