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

// Either or neither of NETWORK_LOGGING and SERIAL_LOGGING may be enabled, not both.
#define NETWORK_LOGGING 0
#define SERIAL_LOGGING 1
#define OTA_UPDATE 1

#define MDNS_HOSTNAME "joist"
#define ADMIN_USERNAME "admin"
#define ADMIN_REALM "admin@qbutton"

#define LOCAL_SERVER_PORT 10158

#ifdef NODEMCU
#define LED_PIN D4
#define LEFT_PWM D1
#define RIGHT_PWM D2
#define LEFT_DIRECTION D3
#define RIGHT_DIRECTION D4

#elif defined(D1MINI)
#define LED_PIN D4
#define LEFT_PWM D1
#define LEFT_FORWARD D5
#define LEFT_REVERSE D2
#define RIGHT_PWM D7
#define RIGHT_FORWARD D6
#define RIGHT_REVERSE D8

#else
#define LED_PIN M5_LED

#endif
