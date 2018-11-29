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

#pragma once

#include <Arduino.h>

#define RF_CODE_LENGTH 3

struct RfCode {
  uint8_t bytes[RF_CODE_LENGTH];

  RfCode() {
    memset(bytes, 0, RF_CODE_LENGTH);
  }

  RfCode(const RfCode &other) {
    copy_from(other.bytes);
  }

  bool operator==(const RfCode &other) {
    return memcmp(bytes, other.bytes, RF_CODE_LENGTH) == 0;
  }

  static RfCode from_hex(const String &hex_string);
  void copy_from(const uint8_t *buffer);
  String to_hex() const;
};

struct ButtonCommand {
  RfCode code;
  String command;

  ButtonCommand() {}
  ButtonCommand(RfCode &code, const String &command): code(code), command(command) {}
};
