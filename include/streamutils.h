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

// Copy from a char * to a char[n] buffer without overrunning the buffer, making sure to end with a \0.
#define safe_copy(src, dest) snprintf((dest), sizeof(dest), "%s", (src))

void copyStreamToPrint(Stream &from, Print &to);
bool write_line_to_file(const char *path, const char *token);
String read_line_from_file(const char *filename);
bool write_strings_to_file(const char *path, const String values[], size_t size);
bool read_strings_from_file(const char *path, String values[], size_t size);
bool write_bools_to_file(const char *path, const bool values[], size_t size);
bool read_bools_from_file(const char *path, bool values[], size_t size);
