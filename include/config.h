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

// Pin which is connected via a resistor to CH_PD, to latch power on
#define EN_PIN 4
#define LED_PIN 2

#define REQUEST_BUFFER_SIZE 200

#define DRD_TIMEOUT 0.5
#define DRD_ADDRESS 0x00

// Either or neither of NETWORK_LOGGING and SERIAL_LOGGING may be enabled, not both.
#define NETWORK_LOGGING 0
#define SERIAL_LOGGING 1
#define OTA_UPDATE 0

#define MDNS_HOSTNAME "qbutton"
#define ADMIN_USERNAME "admin"
#define ADMIN_REALM "admin@qbutton"