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

#include "streamutils.h"

#include "logging.h"

#include <Arduino.h>
#include <FS.h>

// Copy all available bytes from the given Stream to the given Print
void copyStreamToPrint(Stream &from, Print &to) {
  while (true) {
    uint8_t buffer[64] = {0};
    size_t size = from.readBytes(buffer, 64);
    if (size <= 0) {
      break;
    }
    to.write(buffer, size);
  }
}

bool write_line_to_file(const char *path, const char *token) {
  File tokenFile = SPIFFS.open(path, "w");
  if (!tokenFile) {
    LOG("Failed to open ");
    LOG(path);
    LOGLN(" for writing.");
    return false;
  }
  // Don't use println, because it adds '\r' characters which we don't want.
  tokenFile.print(token);
  tokenFile.print('\n');
  tokenFile.close();
  return true;
}

String read_line_from_file(const char *filename) {
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    LOG("Failed to open ");
    LOG(filename);
    LOGLN(" for reading.");
    return String();
  }
  String token = file.readStringUntil('\n');
  file.close();
  return token;
}

bool write_strings_to_file(const char *path, const String values[], size_t size) {
  File file = SPIFFS.open(path, "w");
  if (!file) {
    LOG("Failed to open ");
    LOG(path);
    LOGLN(" for writing.");
    return false;
  }
  for (size_t i = 0; i < size; ++i) {
    file.print(values[i]);
    file.print('\n');
  }
  file.close();
  return true;
}

bool read_strings_from_file(const char *path, String values[], size_t size) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    LOG("Failed to open ");
    LOG(path);
    LOGLN(" for reading.");
    return false;
  }
  for (size_t i = 0; i < size; ++i) {
    values[i] = file.readStringUntil('\n');
  }
  file.close();
  return true;
}

bool write_bools_to_file(const char *path, const bool values[], size_t size) {
  File file = SPIFFS.open(path, "w");
  if (!file) {
    LOG("Failed to open ");
    LOG(path);
    LOGLN(" for writing.");
    return false;
  }
  for (size_t i = 0; i < size; ++i) {
    file.print(values[i]);
    file.print('\n');
  }
  file.close();
  return true;
}

bool read_bools_from_file(const char *path, bool values[], size_t size) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    LOG("Failed to open ");
    LOG(path);
    LOGLN(" for reading.");
    return false;
  }
  for (size_t i = 0; i < size; ++i) {
    values[i] = file.readStringUntil('\n') == "1";
  }
  file.close();
  return true;
}
