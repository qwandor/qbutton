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

extern const uint8_t switch_pins[];
extern const char *const switch_names[];
extern const size_t num_switches;

extern bool switch_inverted[];
extern bool switch_initial_state[];
extern String switch_ids[];
extern bool switch_state[];
extern int switch_brightness[];

bool save_switch_config();
bool save_switch_ids();

void sinric_setup();
void sinric_connect();
void sinric_loop();
void update_switch(size_t i);
