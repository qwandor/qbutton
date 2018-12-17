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

#include "config.h"

#include <WiFiClient.h>
#include <WiFiServer.h>

#if NETWORK_LOGGING
extern WiFiServer log_server;

WiFiClient get_log_client();

#define LOG(x) get_log_client().print(x)
#define LOGH(x) get_log_client().print(x, HEX)
#define LOGLN(x) get_log_client().println(x)
#elif SERIAL_LOGGING
#define LOG(x) Serial.print(x)
#define LOGH(x) Serial.print(x, HEX)
#define LOGLN(x) Serial.println(x)
#else
#define LOG(x)
#define LOGH(x)
#define LOGLN(x)
#endif
