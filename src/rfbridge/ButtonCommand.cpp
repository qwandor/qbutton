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

#include "ButtonCommand.h"

#include <Arduino.h>

static uint8_t hex_char_to_nibble(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else {
    return 0;
  }
}

RfCode RfCode::from_hex(const String &hex_string) {
  RfCode result;
  if (hex_string.length() != RF_CODE_LENGTH * 2) {
    return result;
  }
  for (int i = 0; i < RF_CODE_LENGTH; ++i) {
    result.bytes[i] = (hex_char_to_nibble(hex_string.charAt(2 * i)) << 4) + hex_char_to_nibble(hex_string.charAt(2 * i + 1));
  }
  return result;
}

void RfCode::copy_from(const uint8_t *buffer) {
  memcpy(bytes, buffer, RF_CODE_LENGTH);
}

String RfCode::to_hex() const {
  String result;
    for (int i = 0; i < RF_CODE_LENGTH; ++i) {
      if (bytes[i] < 16) {
        result += '0';
      }
      result += String(bytes[i], HEX);
  }
  return result;
}
