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

#include "assistant.h"

#include "config.h"
#include "embedded_assistant.pb.h"
#include "stream_body.pb.h"
#include "streamutils.h"
#include "logging.h"
#include "webserver.h"

#include <pb_arduino.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <WiFiClientSecure.h>

static const char *oauth_host = "oauth2.googleapis.com";
static const char *host = "embeddedassistant.googleapis.com";
static const int httpsPort = 443;

const char *client_id = ASSISTANT_CLIENT_ID;
static const char *client_secret = ASSISTANT_CLIENT_SECRET;
static const char *device_id = ASSISTANT_DEVICE_ID;
static const char *device_model_id = ASSISTANT_DEVICE_MODEL_ID;

// GTS Root R1 CA certificate
static const uint8_t gts_cacert[] PROGMEM = {
  0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x42, 0x45, 0x47, 0x49, 0x4e, 0x20, 0x43,
  0x45, 0x52, 0x54, 0x49, 0x46, 0x49, 0x43, 0x41, 0x54, 0x45, 0x2d, 0x2d,
  0x2d, 0x2d, 0x2d, 0x0d, 0x0a, 0x4d, 0x49, 0x49, 0x46, 0x57, 0x6a, 0x43,
  0x43, 0x41, 0x30, 0x4b, 0x67, 0x41, 0x77, 0x49, 0x42, 0x41, 0x67, 0x49,
  0x51, 0x62, 0x6b, 0x65, 0x70, 0x78, 0x55, 0x74, 0x48, 0x44, 0x41, 0x33,
  0x73, 0x4d, 0x39, 0x43, 0x4a, 0x75, 0x52, 0x7a, 0x30, 0x34, 0x54, 0x41,
  0x4e, 0x42, 0x67, 0x6b, 0x71, 0x68, 0x6b, 0x69, 0x47, 0x39, 0x77, 0x30,
  0x42, 0x41, 0x51, 0x77, 0x46, 0x41, 0x44, 0x42, 0x48, 0x0d, 0x0a, 0x4d,
  0x51, 0x73, 0x77, 0x43, 0x51, 0x59, 0x44, 0x56, 0x51, 0x51, 0x47, 0x45,
  0x77, 0x4a, 0x56, 0x55, 0x7a, 0x45, 0x69, 0x4d, 0x43, 0x41, 0x47, 0x41,
  0x31, 0x55, 0x45, 0x43, 0x68, 0x4d, 0x5a, 0x52, 0x32, 0x39, 0x76, 0x5a,
  0x32, 0x78, 0x6c, 0x49, 0x46, 0x52, 0x79, 0x64, 0x58, 0x4e, 0x30, 0x49,
  0x46, 0x4e, 0x6c, 0x63, 0x6e, 0x5a, 0x70, 0x59, 0x32, 0x56, 0x7a, 0x49,
  0x45, 0x78, 0x4d, 0x0d, 0x0a, 0x51, 0x7a, 0x45, 0x55, 0x4d, 0x42, 0x49,
  0x47, 0x41, 0x31, 0x55, 0x45, 0x41, 0x78, 0x4d, 0x4c, 0x52, 0x31, 0x52,
  0x54, 0x49, 0x46, 0x4a, 0x76, 0x62, 0x33, 0x51, 0x67, 0x55, 0x6a, 0x45,
  0x77, 0x48, 0x68, 0x63, 0x4e, 0x4d, 0x54, 0x59, 0x77, 0x4e, 0x6a, 0x49,
  0x79, 0x4d, 0x44, 0x41, 0x77, 0x4d, 0x44, 0x41, 0x77, 0x57, 0x68, 0x63,
  0x4e, 0x4d, 0x7a, 0x59, 0x77, 0x4e, 0x6a, 0x49, 0x79, 0x0d, 0x0a, 0x4d,
  0x44, 0x41, 0x77, 0x4d, 0x44, 0x41, 0x77, 0x57, 0x6a, 0x42, 0x48, 0x4d,
  0x51, 0x73, 0x77, 0x43, 0x51, 0x59, 0x44, 0x56, 0x51, 0x51, 0x47, 0x45,
  0x77, 0x4a, 0x56, 0x55, 0x7a, 0x45, 0x69, 0x4d, 0x43, 0x41, 0x47, 0x41,
  0x31, 0x55, 0x45, 0x43, 0x68, 0x4d, 0x5a, 0x52, 0x32, 0x39, 0x76, 0x5a,
  0x32, 0x78, 0x6c, 0x49, 0x46, 0x52, 0x79, 0x64, 0x58, 0x4e, 0x30, 0x49,
  0x46, 0x4e, 0x6c, 0x0d, 0x0a, 0x63, 0x6e, 0x5a, 0x70, 0x59, 0x32, 0x56,
  0x7a, 0x49, 0x45, 0x78, 0x4d, 0x51, 0x7a, 0x45, 0x55, 0x4d, 0x42, 0x49,
  0x47, 0x41, 0x31, 0x55, 0x45, 0x41, 0x78, 0x4d, 0x4c, 0x52, 0x31, 0x52,
  0x54, 0x49, 0x46, 0x4a, 0x76, 0x62, 0x33, 0x51, 0x67, 0x55, 0x6a, 0x45,
  0x77, 0x67, 0x67, 0x49, 0x69, 0x4d, 0x41, 0x30, 0x47, 0x43, 0x53, 0x71,
  0x47, 0x53, 0x49, 0x62, 0x33, 0x44, 0x51, 0x45, 0x42, 0x0d, 0x0a, 0x41,
  0x51, 0x55, 0x41, 0x41, 0x34, 0x49, 0x43, 0x44, 0x77, 0x41, 0x77, 0x67,
  0x67, 0x49, 0x4b, 0x41, 0x6f, 0x49, 0x43, 0x41, 0x51, 0x43, 0x32, 0x45,
  0x51, 0x4b, 0x4c, 0x48, 0x75, 0x4f, 0x68, 0x64, 0x35, 0x73, 0x37, 0x33,
  0x4c, 0x2b, 0x55, 0x50, 0x72, 0x65, 0x56, 0x70, 0x30, 0x41, 0x38, 0x6f,
  0x66, 0x32, 0x43, 0x2b, 0x58, 0x30, 0x79, 0x42, 0x6f, 0x4a, 0x78, 0x39,
  0x76, 0x61, 0x4d, 0x0d, 0x0a, 0x66, 0x2f, 0x76, 0x6f, 0x32, 0x37, 0x78,
  0x71, 0x4c, 0x70, 0x65, 0x58, 0x6f, 0x34, 0x78, 0x4c, 0x2b, 0x53, 0x76,
  0x32, 0x73, 0x66, 0x6e, 0x4f, 0x68, 0x42, 0x32, 0x78, 0x2b, 0x63, 0x57,
  0x58, 0x33, 0x75, 0x2b, 0x35, 0x38, 0x71, 0x50, 0x70, 0x76, 0x42, 0x4b,
  0x4a, 0x58, 0x71, 0x65, 0x71, 0x55, 0x71, 0x76, 0x34, 0x49, 0x79, 0x66,
  0x4c, 0x70, 0x4c, 0x47, 0x63, 0x59, 0x39, 0x76, 0x58, 0x0d, 0x0a, 0x6d,
  0x58, 0x37, 0x77, 0x43, 0x6c, 0x37, 0x72, 0x61, 0x4b, 0x62, 0x30, 0x78,
  0x6c, 0x70, 0x48, 0x44, 0x55, 0x30, 0x51, 0x4d, 0x2b, 0x4e, 0x4f, 0x73,
  0x52, 0x4f, 0x6a, 0x79, 0x42, 0x68, 0x73, 0x53, 0x2b, 0x7a, 0x38, 0x43,
  0x5a, 0x44, 0x66, 0x6e, 0x57, 0x51, 0x70, 0x4a, 0x53, 0x4d, 0x48, 0x6f,
  0x62, 0x54, 0x53, 0x50, 0x53, 0x35, 0x67, 0x34, 0x4d, 0x2f, 0x53, 0x43,
  0x59, 0x65, 0x37, 0x0d, 0x0a, 0x7a, 0x55, 0x6a, 0x77, 0x54, 0x63, 0x4c,
  0x43, 0x65, 0x6f, 0x69, 0x4b, 0x75, 0x37, 0x72, 0x50, 0x57, 0x52, 0x6e,
  0x57, 0x72, 0x34, 0x2b, 0x77, 0x42, 0x37, 0x43, 0x65, 0x4d, 0x66, 0x47,
  0x43, 0x77, 0x63, 0x44, 0x66, 0x4c, 0x71, 0x5a, 0x74, 0x62, 0x42, 0x6b,
  0x4f, 0x74, 0x64, 0x68, 0x2b, 0x4a, 0x68, 0x70, 0x46, 0x41, 0x7a, 0x32,
  0x77, 0x65, 0x61, 0x53, 0x55, 0x4b, 0x4b, 0x30, 0x50, 0x0d, 0x0a, 0x66,
  0x79, 0x62, 0x6c, 0x71, 0x41, 0x6a, 0x2b, 0x6c, 0x75, 0x67, 0x38, 0x61,
  0x4a, 0x52, 0x54, 0x37, 0x6f, 0x4d, 0x36, 0x69, 0x43, 0x73, 0x56, 0x6c,
  0x67, 0x6d, 0x79, 0x34, 0x48, 0x71, 0x4d, 0x4c, 0x6e, 0x58, 0x57, 0x6e,
  0x4f, 0x75, 0x6e, 0x56, 0x6d, 0x53, 0x50, 0x6c, 0x6b, 0x39, 0x6f, 0x72,
  0x6a, 0x32, 0x58, 0x77, 0x6f, 0x53, 0x50, 0x77, 0x4c, 0x78, 0x41, 0x77,
  0x41, 0x74, 0x63, 0x0d, 0x0a, 0x76, 0x66, 0x61, 0x48, 0x73, 0x7a, 0x56,
  0x73, 0x72, 0x42, 0x68, 0x51, 0x66, 0x34, 0x54, 0x67, 0x54, 0x4d, 0x32,
  0x53, 0x30, 0x79, 0x44, 0x70, 0x4d, 0x37, 0x78, 0x53, 0x6d, 0x61, 0x38,
  0x79, 0x74, 0x53, 0x6d, 0x7a, 0x4a, 0x53, 0x71, 0x30, 0x53, 0x50, 0x6c,
  0x79, 0x34, 0x63, 0x70, 0x6b, 0x39, 0x2b, 0x61, 0x43, 0x45, 0x49, 0x33,
  0x6f, 0x6e, 0x63, 0x4b, 0x4b, 0x69, 0x50, 0x6f, 0x34, 0x0d, 0x0a, 0x5a,
  0x6f, 0x72, 0x38, 0x59, 0x2f, 0x6b, 0x42, 0x2b, 0x58, 0x6a, 0x39, 0x65,
  0x31, 0x78, 0x33, 0x2b, 0x6e, 0x61, 0x48, 0x2b, 0x75, 0x7a, 0x66, 0x73,
  0x51, 0x35, 0x35, 0x6c, 0x56, 0x65, 0x30, 0x76, 0x53, 0x62, 0x76, 0x31,
  0x67, 0x48, 0x52, 0x36, 0x78, 0x59, 0x4b, 0x75, 0x34, 0x34, 0x4c, 0x74,
  0x63, 0x58, 0x46, 0x69, 0x6c, 0x57, 0x72, 0x30, 0x36, 0x7a, 0x71, 0x6b,
  0x55, 0x73, 0x70, 0x0d, 0x0a, 0x7a, 0x42, 0x6d, 0x6b, 0x4d, 0x69, 0x56,
  0x4f, 0x4b, 0x76, 0x46, 0x6c, 0x52, 0x4e, 0x41, 0x43, 0x7a, 0x71, 0x72,
  0x4f, 0x53, 0x62, 0x54, 0x71, 0x6e, 0x33, 0x79, 0x44, 0x73, 0x45, 0x42,
  0x37, 0x35, 0x30, 0x4f, 0x72, 0x70, 0x32, 0x79, 0x6a, 0x6a, 0x33, 0x32,
  0x4a, 0x67, 0x66, 0x70, 0x4d, 0x70, 0x66, 0x2f, 0x56, 0x6a, 0x73, 0x50,
  0x4f, 0x53, 0x2b, 0x43, 0x31, 0x32, 0x4c, 0x4f, 0x4f, 0x0d, 0x0a, 0x52,
  0x63, 0x39, 0x32, 0x77, 0x4f, 0x31, 0x41, 0x4b, 0x2f, 0x31, 0x54, 0x44,
  0x37, 0x43, 0x6e, 0x31, 0x54, 0x73, 0x4e, 0x73, 0x59, 0x71, 0x69, 0x41,
  0x39, 0x34, 0x78, 0x72, 0x63, 0x78, 0x33, 0x36, 0x6d, 0x39, 0x37, 0x50,
  0x74, 0x62, 0x66, 0x6b, 0x53, 0x49, 0x53, 0x35, 0x72, 0x37, 0x36, 0x32,
  0x44, 0x4c, 0x38, 0x45, 0x47, 0x4d, 0x55, 0x55, 0x58, 0x4c, 0x65, 0x58,
  0x64, 0x59, 0x57, 0x0d, 0x0a, 0x6b, 0x37, 0x30, 0x70, 0x61, 0x44, 0x50,
  0x76, 0x4f, 0x6d, 0x62, 0x73, 0x42, 0x34, 0x6f, 0x6d, 0x33, 0x78, 0x50,
  0x58, 0x56, 0x32, 0x56, 0x34, 0x4a, 0x39, 0x35, 0x65, 0x53, 0x52, 0x51,
  0x41, 0x6f, 0x67, 0x42, 0x2f, 0x6d, 0x71, 0x67, 0x68, 0x74, 0x71, 0x6d,
  0x78, 0x6c, 0x62, 0x43, 0x6c, 0x75, 0x51, 0x30, 0x57, 0x45, 0x64, 0x72,
  0x48, 0x62, 0x45, 0x67, 0x38, 0x51, 0x4f, 0x42, 0x2b, 0x0d, 0x0a, 0x44,
  0x56, 0x72, 0x4e, 0x56, 0x6a, 0x7a, 0x52, 0x6c, 0x77, 0x57, 0x35, 0x79,
  0x30, 0x76, 0x74, 0x4f, 0x55, 0x75, 0x63, 0x78, 0x44, 0x2f, 0x53, 0x56,
  0x52, 0x4e, 0x75, 0x4a, 0x4c, 0x44, 0x57, 0x63, 0x66, 0x72, 0x30, 0x77,
  0x62, 0x72, 0x4d, 0x37, 0x52, 0x76, 0x31, 0x2f, 0x6f, 0x46, 0x42, 0x32,
  0x41, 0x43, 0x59, 0x50, 0x54, 0x72, 0x49, 0x72, 0x6e, 0x71, 0x59, 0x4e,
  0x78, 0x67, 0x46, 0x0d, 0x0a, 0x6c, 0x51, 0x49, 0x44, 0x41, 0x51, 0x41,
  0x42, 0x6f, 0x30, 0x49, 0x77, 0x51, 0x44, 0x41, 0x4f, 0x42, 0x67, 0x4e,
  0x56, 0x48, 0x51, 0x38, 0x42, 0x41, 0x66, 0x38, 0x45, 0x42, 0x41, 0x4d,
  0x43, 0x41, 0x51, 0x59, 0x77, 0x44, 0x77, 0x59, 0x44, 0x56, 0x52, 0x30,
  0x54, 0x41, 0x51, 0x48, 0x2f, 0x42, 0x41, 0x55, 0x77, 0x41, 0x77, 0x45,
  0x42, 0x2f, 0x7a, 0x41, 0x64, 0x42, 0x67, 0x4e, 0x56, 0x0d, 0x0a, 0x48,
  0x51, 0x34, 0x45, 0x46, 0x67, 0x51, 0x55, 0x35, 0x4b, 0x38, 0x72, 0x4a,
  0x6e, 0x45, 0x61, 0x4b, 0x30, 0x67, 0x6e, 0x68, 0x53, 0x39, 0x53, 0x5a,
  0x69, 0x7a, 0x76, 0x38, 0x49, 0x6b, 0x54, 0x63, 0x54, 0x34, 0x77, 0x44,
  0x51, 0x59, 0x4a, 0x4b, 0x6f, 0x5a, 0x49, 0x68, 0x76, 0x63, 0x4e, 0x41,
  0x51, 0x45, 0x4d, 0x42, 0x51, 0x41, 0x44, 0x67, 0x67, 0x49, 0x42, 0x41,
  0x44, 0x69, 0x57, 0x0d, 0x0a, 0x43, 0x75, 0x34, 0x39, 0x74, 0x4a, 0x59,
  0x65, 0x58, 0x2b, 0x2b, 0x64, 0x6e, 0x41, 0x73, 0x7a, 0x6e, 0x79, 0x76,
  0x67, 0x79, 0x76, 0x33, 0x53, 0x6a, 0x67, 0x6f, 0x66, 0x51, 0x58, 0x53,
  0x6c, 0x66, 0x4b, 0x71, 0x45, 0x31, 0x4f, 0x58, 0x79, 0x48, 0x75, 0x59,
  0x33, 0x55, 0x6a, 0x4b, 0x63, 0x43, 0x39, 0x46, 0x68, 0x48, 0x62, 0x38,
  0x6f, 0x77, 0x62, 0x5a, 0x45, 0x4b, 0x54, 0x56, 0x31, 0x0d, 0x0a, 0x64,
  0x35, 0x69, 0x79, 0x66, 0x4e, 0x6d, 0x39, 0x64, 0x4b, 0x79, 0x4b, 0x61,
  0x4f, 0x4f, 0x70, 0x4d, 0x51, 0x6b, 0x70, 0x41, 0x57, 0x42, 0x7a, 0x34,
  0x30, 0x64, 0x38, 0x55, 0x36, 0x69, 0x51, 0x53, 0x69, 0x66, 0x76, 0x53,
  0x39, 0x65, 0x66, 0x6b, 0x2b, 0x65, 0x43, 0x4e, 0x73, 0x36, 0x61, 0x61,
  0x41, 0x79, 0x43, 0x35, 0x38, 0x2f, 0x55, 0x45, 0x42, 0x5a, 0x76, 0x58,
  0x77, 0x36, 0x5a, 0x0d, 0x0a, 0x58, 0x50, 0x59, 0x66, 0x63, 0x58, 0x33,
  0x76, 0x37, 0x33, 0x73, 0x76, 0x66, 0x75, 0x6f, 0x32, 0x31, 0x70, 0x64,
  0x77, 0x43, 0x78, 0x58, 0x75, 0x31, 0x31, 0x78, 0x57, 0x61, 0x6a, 0x4f,
  0x6c, 0x34, 0x30, 0x6b, 0x34, 0x44, 0x4c, 0x68, 0x39, 0x2b, 0x34, 0x32,
  0x46, 0x70, 0x4c, 0x46, 0x5a, 0x58, 0x76, 0x52, 0x71, 0x34, 0x64, 0x32,
  0x68, 0x39, 0x6d, 0x52, 0x45, 0x72, 0x75, 0x5a, 0x52, 0x0d, 0x0a, 0x67,
  0x79, 0x46, 0x6d, 0x78, 0x68, 0x45, 0x2b, 0x38, 0x38, 0x35, 0x48, 0x37,
  0x70, 0x77, 0x6f, 0x48, 0x79, 0x58, 0x61, 0x2f, 0x36, 0x78, 0x6d, 0x6c,
  0x64, 0x30, 0x31, 0x44, 0x31, 0x7a, 0x76, 0x49, 0x43, 0x78, 0x69, 0x2f,
  0x5a, 0x47, 0x36, 0x71, 0x63, 0x7a, 0x38, 0x57, 0x70, 0x79, 0x54, 0x67,
  0x59, 0x4d, 0x70, 0x6c, 0x30, 0x70, 0x38, 0x57, 0x6e, 0x4b, 0x30, 0x4f,
  0x64, 0x43, 0x33, 0x0d, 0x0a, 0x64, 0x38, 0x74, 0x35, 0x2f, 0x57, 0x6b,
  0x36, 0x6b, 0x6a, 0x66, 0x74, 0x62, 0x6a, 0x68, 0x6c, 0x52, 0x6e, 0x37,
  0x70, 0x59, 0x4c, 0x31, 0x35, 0x69, 0x4a, 0x64, 0x66, 0x4f, 0x42, 0x4c,
  0x30, 0x37, 0x71, 0x39, 0x62, 0x67, 0x73, 0x69, 0x47, 0x31, 0x65, 0x47,
  0x5a, 0x62, 0x59, 0x77, 0x45, 0x38, 0x6e, 0x61, 0x36, 0x53, 0x66, 0x5a,
  0x75, 0x36, 0x57, 0x30, 0x65, 0x58, 0x36, 0x44, 0x76, 0x0d, 0x0a, 0x4a,
  0x34, 0x4a, 0x32, 0x51, 0x50, 0x69, 0x6d, 0x30, 0x31, 0x68, 0x63, 0x44,
  0x79, 0x78, 0x43, 0x32, 0x6b, 0x4c, 0x47, 0x65, 0x34, 0x67, 0x30, 0x78,
  0x38, 0x48, 0x59, 0x52, 0x5a, 0x76, 0x42, 0x50, 0x73, 0x56, 0x68, 0x48,
  0x64, 0x6c, 0x6a, 0x55, 0x45, 0x6e, 0x32, 0x4e, 0x49, 0x56, 0x71, 0x34,
  0x42, 0x6a, 0x46, 0x62, 0x6b, 0x65, 0x72, 0x51, 0x55, 0x49, 0x70, 0x6d,
  0x2f, 0x5a, 0x67, 0x0d, 0x0a, 0x44, 0x64, 0x49, 0x78, 0x30, 0x32, 0x4f,
  0x59, 0x49, 0x35, 0x4e, 0x61, 0x41, 0x49, 0x46, 0x49, 0x74, 0x4f, 0x2f,
  0x4e, 0x69, 0x73, 0x33, 0x4a, 0x7a, 0x35, 0x6e, 0x75, 0x32, 0x5a, 0x36,
  0x71, 0x4e, 0x75, 0x46, 0x6f, 0x53, 0x33, 0x46, 0x4a, 0x46, 0x44, 0x59,
  0x6f, 0x4f, 0x6a, 0x30, 0x64, 0x7a, 0x70, 0x71, 0x50, 0x4a, 0x65, 0x61,
  0x41, 0x63, 0x57, 0x45, 0x72, 0x74, 0x58, 0x76, 0x4d, 0x0d, 0x0a, 0x2b,
  0x53, 0x55, 0x57, 0x67, 0x65, 0x45, 0x78, 0x58, 0x36, 0x47, 0x6a, 0x66,
  0x68, 0x61, 0x6b, 0x6e, 0x42, 0x5a, 0x71, 0x6c, 0x78, 0x69, 0x39, 0x64,
  0x6e, 0x4b, 0x6c, 0x43, 0x35, 0x34, 0x64, 0x4e, 0x75, 0x59, 0x76, 0x6f,
  0x53, 0x2b, 0x2b, 0x63, 0x4a, 0x45, 0x50, 0x71, 0x4f, 0x62, 0x61, 0x2b,
  0x4d, 0x53, 0x53, 0x51, 0x47, 0x77, 0x6c, 0x66, 0x6e, 0x75, 0x7a, 0x43,
  0x64, 0x79, 0x79, 0x0d, 0x0a, 0x46, 0x36, 0x32, 0x41, 0x52, 0x50, 0x42,
  0x6f, 0x70, 0x59, 0x2b, 0x55, 0x64, 0x66, 0x39, 0x30, 0x57, 0x75, 0x69,
  0x6f, 0x41, 0x6e, 0x77, 0x4d, 0x43, 0x65, 0x4b, 0x70, 0x53, 0x77, 0x75,
  0x67, 0x68, 0x51, 0x74, 0x69, 0x75, 0x65, 0x2b, 0x68, 0x4d, 0x5a, 0x4c,
  0x37, 0x37, 0x2f, 0x5a, 0x52, 0x42, 0x49, 0x6c, 0x73, 0x36, 0x4b, 0x6c,
  0x30, 0x6f, 0x62, 0x73, 0x58, 0x73, 0x37, 0x58, 0x39, 0x0d, 0x0a, 0x53,
  0x51, 0x39, 0x38, 0x50, 0x4f, 0x79, 0x44, 0x47, 0x43, 0x42, 0x44, 0x54,
  0x74, 0x57, 0x54, 0x75, 0x72, 0x51, 0x30, 0x73, 0x52, 0x38, 0x57, 0x4e,
  0x68, 0x38, 0x4d, 0x35, 0x6d, 0x51, 0x35, 0x46, 0x6b, 0x7a, 0x63, 0x34,
  0x50, 0x34, 0x64, 0x79, 0x4b, 0x6c, 0x69, 0x50, 0x55, 0x44, 0x71, 0x79,
  0x73, 0x55, 0x30, 0x41, 0x72, 0x53, 0x75, 0x69, 0x59, 0x67, 0x7a, 0x4e,
  0x64, 0x77, 0x73, 0x0d, 0x0a, 0x45, 0x33, 0x50, 0x59, 0x4a, 0x2f, 0x48,
  0x51, 0x63, 0x75, 0x35, 0x31, 0x4f, 0x79, 0x4c, 0x65, 0x6d, 0x47, 0x68,
  0x6d, 0x57, 0x2f, 0x48, 0x47, 0x59, 0x30, 0x64, 0x56, 0x48, 0x4c, 0x71,
  0x6c, 0x43, 0x46, 0x46, 0x31, 0x70, 0x6b, 0x67, 0x6c, 0x0d, 0x0a, 0x2d,
  0x2d, 0x2d, 0x2d, 0x2d, 0x45, 0x4e, 0x44, 0x20, 0x43, 0x45, 0x52, 0x54,
  0x49, 0x46, 0x49, 0x43, 0x41, 0x54, 0x45, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
  0x0d, 0x0a
};

static WiFiClientSecure client;

// Read all remaining response from a client, printing it to the log for debugging.
void print_response(WiFiClientSecure &client) {
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    LOGLN(line);
  }
}

String load_token() {
  return read_line_from_file("/token.txt");
}

bool encode_assist_request(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
  const google_assistant_embedded_v1alpha2_AssistRequest *assist_request = static_cast<const google_assistant_embedded_v1alpha2_AssistRequest *>(*arg);
  if (!pb_encode_tag_for_field(stream, field)) {
    return false;
  }
  return pb_encode_submessage(stream, google_assistant_embedded_v1alpha2_AssistRequest_fields, assist_request);
}

// Encode the given command as the appropriate protobuf message and write it to the given Print stream.
bool encode_request(const char *command, pb_ostream_t *pb_out) {
  google_assistant_embedded_v1alpha2_AssistRequest assist_request = google_assistant_embedded_v1alpha2_AssistRequest_init_default;
  assist_request.which_type = google_assistant_embedded_v1alpha2_AssistRequest_config_tag;
  assist_request.type.config.has_audio_out_config = true;
  assist_request.type.config.audio_out_config.encoding = google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_MP3;
  assist_request.type.config.audio_out_config.sample_rate_hertz = 16000;
  assist_request.type.config.has_screen_out_config = true;
  assist_request.type.config.screen_out_config.screen_mode = google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_PLAYING;
  assist_request.type.config.has_dialog_state_in = true;
  safe_copy("en-US", assist_request.type.config.dialog_state_in.language_code);
  assist_request.type.config.has_device_config = true;
  safe_copy(device_id, assist_request.type.config.device_config.device_id);
  safe_copy(device_model_id, assist_request.type.config.device_config.device_model_id);
  assist_request.type.config.which_type = google_assistant_embedded_v1alpha2_AssistConfig_text_query_tag;
  safe_copy(command, assist_request.type.config.type.text_query);

  google_rpc_StreamBody stream_body = google_rpc_StreamBody_init_default;
  stream_body.message.funcs.encode = encode_assist_request;
  stream_body.message.arg = &assist_request;
  if (!pb_encode(pb_out, google_rpc_StreamBody_fields, &stream_body)) {
    LOG("Failed encoding StreamBody: ");
    LOGLN(PB_GET_ERROR(pb_out));
    return false;
  }

  return true;
}

// Given an OAuth code, get a new token and refresh token and store them.
bool oauth_with_code(const String &code) {
  if (!client.connect(oauth_host, httpsPort)) {
    LOGLN("connection failed");
    return false;
  }

  if (!client.verifyCertChain(oauth_host)) {
    LOGLN("Invalid certificate");
    client.stop();
    return false;
  }

  String body = String("client_id=") + client_id + "&" +
               "client_secret=" + client_secret + "&" +
               "grant_type=authorization_code&" +
               "redirect_uri=urn:ietf:wg:oauth:2.0:oob&" +
               "code=" + code;
  client.print(String("POST /token HTTP/1.0\r\n") +
               "Host: " + oauth_host + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Content-Length: " + body.length() + "\r\n" +
               "Connection: close\r\n\r\n");
  client.print(body);

  // Check status
  String line = client.readStringUntil('\n');
  if (!line.startsWith("HTTP/1.0 200 OK")) {
    LOGLN(line);
    print_response(client);
    client.stop();
    return false;
  }

  // Skip headers
  client.find("\r\n\r\n");

  DynamicJsonBuffer jb;
  JsonObject &root = jb.parseObject(client);
  if (!root.success()) {
    LOGLN("Failed to parse JSON response from OAuth");
    client.stop();
    return false;
  }
  const char *token = root["access_token"];
  const char *refresh_token = root["refresh_token"];
  LOG("got new access token ");
  LOGLN(token);
  write_line_to_file("/refresh_token.txt", refresh_token);
  write_line_to_file("/token.txt", token);
  client.stop();
  return true;
}

void handle_oauth() {
  LOGLN("Start handle_oauth");
  const String &code = server.arg("code");
  if (code.length() > 0) {
    bool success = oauth_with_code(code);
    if (success) {
      server.send(200, "text/html", "<html><head><title>Auth</title></head><body><p>Success</p><a href=\"/\">Home</a></body></html>");
    } else {
      server.send(200, "text/html", "<html><head><title>Auth</title></head><body><p>Failure</p><a href=\"/\">Home</a></body></html>");
    }
  } else {
    server.send(200, "text/html", String("<html><head><title>Error</title></head><body><h1>Authentication error</h1><p>") +
      server.arg("error") + "</p></body></html>");
  }
}

// Use the refresh token to get a new auth token.
// Return true on success.
bool refresh_oauth() {
  if (!client.connect(oauth_host, httpsPort)) {
    LOGLN("connection failed");
    return false;
  }

  if (!client.verifyCertChain(oauth_host)) {
    LOGLN("Invalid certificate");
    client.stop();
    return false;
  }

  File refreshTokenFile = SPIFFS.open("/refresh_token.txt", "r");
  if (!refreshTokenFile) {
    LOGLN("failed to read /refresh_token.txt");
    client.stop();
    return false;
  }

  String body = String("client_id=") + client_id + "&" +
               "client_secret=" + client_secret + "&" +
               "grant_type=refresh_token&" +
               "refresh_token=";
  client.print(String("POST /token HTTP/1.0\r\n") +
               "Host: " + oauth_host + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Content-Length: " + (body.length() + refreshTokenFile.size()) + "\r\n" +
               "Connection: close\r\n\r\n");
  client.print(body);
  copyStreamToPrint(refreshTokenFile, client);
  refreshTokenFile.close();

  // Check status
  String line = client.readStringUntil('\n');
  if (!line.startsWith("HTTP/1.0 200 OK")) {
    LOGLN(line);
    print_response(client);
    client.stop();
    return false;
  }

  // Skip headers
  client.find("\r\n\r\n");

  DynamicJsonBuffer jb;
  JsonObject &root = jb.parseObject(client);
  if (!root.success()) {
    LOGLN("Failed to parse JSON response from OAuth refresh");
    client.stop();
    return false;
  }
  const char *token = root["access_token"];
  LOG("got new access token ");
  LOGLN(token);
  write_line_to_file("/token.txt", token);
  client.stop();
  return true;
}

// Return true on success, false on failure for any reason.
bool send_assistant_request(const String &command) {
  String token = load_token();
  if (token.length() == 0 || command.length() == 0) {
    return false;
  }

  pb_byte_t request_buffer[REQUEST_BUFFER_SIZE];
  pb_ostream_t request_buffer_stream = pb_ostream_from_buffer(request_buffer, REQUEST_BUFFER_SIZE);
  if (!encode_request(command.c_str(), &request_buffer_stream)) {
    LOGLN("Failed to encode request.");
    return false;
  }

  // Use WiFiClientSecure class to create TLS connection
  LOG("connecting to ");
  LOGLN(host);
  if (!client.connect(host, httpsPort)) {
    LOGLN("connection failed");
    return false;
  }

  if (!client.verifyCertChain(host)) {
    LOGLN("Invalid certificate");
    client.stop();
    return false;
  }

  String path = "/$rpc/google.assistant.embedded.v1alpha2.EmbeddedAssistant/Assist";
  LOG("requesting path: ");
  LOGLN(path);

  client.print(String("POST ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: qbutton\r\n" +
               "Content-Type: application/x-protobuf\r\n" +
               "Authorization: Bearer " + token + "\r\n" +
               "Content-Length: " + request_buffer_stream.bytes_written + "\r\n" +
               "Connection: close\r\n\r\n");

  LOGLN("headers sent");
  size_t bytes_sent = client.write(request_buffer, request_buffer_stream.bytes_written);
  if (bytes_sent != request_buffer_stream.bytes_written) {
    LOG("Tried to send ");
    LOG(request_buffer_stream.bytes_written);
    LOG(" bytes of request to server but only sent ");
    LOGLN(bytes_sent);
  }
  LOGLN("body sent");
  unsigned long sent_request_time = millis();

  // Check status
  String line = client.readStringUntil('\n');
  if (!line.startsWith("HTTP/1.1 200 OK")) {
    LOGLN(line);
    client.stop();
    return false;
  }

  LOG("success after ");
  LOG(millis() - sent_request_time);
  LOGLN(" ms");
  client.stop();
  return true;
}

// Send the request to Google Assistant, refreshing the auth token if necessary.
void auth_and_send_request(const String &command) {
  if (send_assistant_request(command)) {
    return;
  }
  LOGLN("First request failed, refreshing token");
  if (!refresh_oauth()) {
    return;
  }
  if (!send_assistant_request(command)) {
    LOGLN("Second request failed");
  }
}

bool assistant_init() {
  server.on("/oauth", handle_oauth);
  if (!client.setCACert_P(gts_cacert, sizeof(gts_cacert))) {
    LOGLN("Failed to load root CA certificate.");
    return false;
  }

  return true;
}
