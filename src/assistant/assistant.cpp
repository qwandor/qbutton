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

static const char *oauth_host = "www.googleapis.com";
static const char *host = "embeddedassistant.googleapis.com";
static const int httpsPort = 443;

const char *client_id = ASSISTANT_CLIENT_ID;
static const char *client_secret = ASSISTANT_CLIENT_SECRET;
static const char *device_id = ASSISTANT_DEVICE_ID;
static const char *device_model_id = ASSISTANT_DEVICE_MODEL_ID;

// GlobalSign Root CA certificate
static const uint8_t globalsign_cacert[] PROGMEM = {
  0x30, 0x82, 0x03, 0xba, 0x30, 0x82, 0x02, 0xa2, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x0b, 0x04, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0f, 0x86, 0x26,
  0xe6, 0x0d, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
  0x01, 0x01, 0x05, 0x05, 0x00, 0x30, 0x4c, 0x31, 0x20, 0x30, 0x1e, 0x06,
  0x03, 0x55, 0x04, 0x0b, 0x13, 0x17, 0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c,
  0x53, 0x69, 0x67, 0x6e, 0x20, 0x52, 0x6f, 0x6f, 0x74, 0x20, 0x43, 0x41,
  0x20, 0x2d, 0x20, 0x52, 0x32, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55,
  0x04, 0x0a, 0x13, 0x0a, 0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x53, 0x69,
  0x67, 0x6e, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13,
  0x0a, 0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x53, 0x69, 0x67, 0x6e, 0x30,
  0x1e, 0x17, 0x0d, 0x30, 0x36, 0x31, 0x32, 0x31, 0x35, 0x30, 0x38, 0x30,
  0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d, 0x32, 0x31, 0x31, 0x32, 0x31, 0x35,
  0x30, 0x38, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x30, 0x4c, 0x31, 0x20, 0x30,
  0x1e, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x17, 0x47, 0x6c, 0x6f, 0x62,
  0x61, 0x6c, 0x53, 0x69, 0x67, 0x6e, 0x20, 0x52, 0x6f, 0x6f, 0x74, 0x20,
  0x43, 0x41, 0x20, 0x2d, 0x20, 0x52, 0x32, 0x31, 0x13, 0x30, 0x11, 0x06,
  0x03, 0x55, 0x04, 0x0a, 0x13, 0x0a, 0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c,
  0x53, 0x69, 0x67, 0x6e, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04,
  0x03, 0x13, 0x0a, 0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x53, 0x69, 0x67,
  0x6e, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48,
  0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f,
  0x00, 0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xa6, 0xcf,
  0x24, 0x0e, 0xbe, 0x2e, 0x6f, 0x28, 0x99, 0x45, 0x42, 0xc4, 0xab, 0x3e,
  0x21, 0x54, 0x9b, 0x0b, 0xd3, 0x7f, 0x84, 0x70, 0xfa, 0x12, 0xb3, 0xcb,
  0xbf, 0x87, 0x5f, 0xc6, 0x7f, 0x86, 0xd3, 0xb2, 0x30, 0x5c, 0xd6, 0xfd,
  0xad, 0xf1, 0x7b, 0xdc, 0xe5, 0xf8, 0x60, 0x96, 0x09, 0x92, 0x10, 0xf5,
  0xd0, 0x53, 0xde, 0xfb, 0x7b, 0x7e, 0x73, 0x88, 0xac, 0x52, 0x88, 0x7b,
  0x4a, 0xa6, 0xca, 0x49, 0xa6, 0x5e, 0xa8, 0xa7, 0x8c, 0x5a, 0x11, 0xbc,
  0x7a, 0x82, 0xeb, 0xbe, 0x8c, 0xe9, 0xb3, 0xac, 0x96, 0x25, 0x07, 0x97,
  0x4a, 0x99, 0x2a, 0x07, 0x2f, 0xb4, 0x1e, 0x77, 0xbf, 0x8a, 0x0f, 0xb5,
  0x02, 0x7c, 0x1b, 0x96, 0xb8, 0xc5, 0xb9, 0x3a, 0x2c, 0xbc, 0xd6, 0x12,
  0xb9, 0xeb, 0x59, 0x7d, 0xe2, 0xd0, 0x06, 0x86, 0x5f, 0x5e, 0x49, 0x6a,
  0xb5, 0x39, 0x5e, 0x88, 0x34, 0xec, 0xbc, 0x78, 0x0c, 0x08, 0x98, 0x84,
  0x6c, 0xa8, 0xcd, 0x4b, 0xb4, 0xa0, 0x7d, 0x0c, 0x79, 0x4d, 0xf0, 0xb8,
  0x2d, 0xcb, 0x21, 0xca, 0xd5, 0x6c, 0x5b, 0x7d, 0xe1, 0xa0, 0x29, 0x84,
  0xa1, 0xf9, 0xd3, 0x94, 0x49, 0xcb, 0x24, 0x62, 0x91, 0x20, 0xbc, 0xdd,
  0x0b, 0xd5, 0xd9, 0xcc, 0xf9, 0xea, 0x27, 0x0a, 0x2b, 0x73, 0x91, 0xc6,
  0x9d, 0x1b, 0xac, 0xc8, 0xcb, 0xe8, 0xe0, 0xa0, 0xf4, 0x2f, 0x90, 0x8b,
  0x4d, 0xfb, 0xb0, 0x36, 0x1b, 0xf6, 0x19, 0x7a, 0x85, 0xe0, 0x6d, 0xf2,
  0x61, 0x13, 0x88, 0x5c, 0x9f, 0xe0, 0x93, 0x0a, 0x51, 0x97, 0x8a, 0x5a,
  0xce, 0xaf, 0xab, 0xd5, 0xf7, 0xaa, 0x09, 0xaa, 0x60, 0xbd, 0xdc, 0xd9,
  0x5f, 0xdf, 0x72, 0xa9, 0x60, 0x13, 0x5e, 0x00, 0x01, 0xc9, 0x4a, 0xfa,
  0x3f, 0xa4, 0xea, 0x07, 0x03, 0x21, 0x02, 0x8e, 0x82, 0xca, 0x03, 0xc2,
  0x9b, 0x8f, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x81, 0x9c, 0x30, 0x81,
  0x99, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04,
  0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13,
  0x01, 0x01, 0xff, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30, 0x1d,
  0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x9b, 0xe2, 0x07,
  0x57, 0x67, 0x1c, 0x1e, 0xc0, 0x6a, 0x06, 0xde, 0x59, 0xb4, 0x9a, 0x2d,
  0xdf, 0xdc, 0x19, 0x86, 0x2e, 0x30, 0x36, 0x06, 0x03, 0x55, 0x1d, 0x1f,
  0x04, 0x2f, 0x30, 0x2d, 0x30, 0x2b, 0xa0, 0x29, 0xa0, 0x27, 0x86, 0x25,
  0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x63, 0x72, 0x6c, 0x2e, 0x67,
  0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x73, 0x69, 0x67, 0x6e, 0x2e, 0x6e, 0x65,
  0x74, 0x2f, 0x72, 0x6f, 0x6f, 0x74, 0x2d, 0x72, 0x32, 0x2e, 0x63, 0x72,
  0x6c, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16,
  0x80, 0x14, 0x9b, 0xe2, 0x07, 0x57, 0x67, 0x1c, 0x1e, 0xc0, 0x6a, 0x06,
  0xde, 0x59, 0xb4, 0x9a, 0x2d, 0xdf, 0xdc, 0x19, 0x86, 0x2e, 0x30, 0x0d,
  0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05,
  0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x99, 0x81, 0x53, 0x87, 0x1c, 0x68,
  0x97, 0x86, 0x91, 0xec, 0xe0, 0x4a, 0xb8, 0x44, 0x0b, 0xab, 0x81, 0xac,
  0x27, 0x4f, 0xd6, 0xc1, 0xb8, 0x1c, 0x43, 0x78, 0xb3, 0x0c, 0x9a, 0xfc,
  0xea, 0x2c, 0x3c, 0x6e, 0x61, 0x1b, 0x4d, 0x4b, 0x29, 0xf5, 0x9f, 0x05,
  0x1d, 0x26, 0xc1, 0xb8, 0xe9, 0x83, 0x00, 0x62, 0x45, 0xb6, 0xa9, 0x08,
  0x93, 0xb9, 0xa9, 0x33, 0x4b, 0x18, 0x9a, 0xc2, 0xf8, 0x87, 0x88, 0x4e,
  0xdb, 0xdd, 0x71, 0x34, 0x1a, 0xc1, 0x54, 0xda, 0x46, 0x3f, 0xe0, 0xd3,
  0x2a, 0xab, 0x6d, 0x54, 0x22, 0xf5, 0x3a, 0x62, 0xcd, 0x20, 0x6f, 0xba,
  0x29, 0x89, 0xd7, 0xdd, 0x91, 0xee, 0xd3, 0x5c, 0xa2, 0x3e, 0xa1, 0x5b,
  0x41, 0xf5, 0xdf, 0xe5, 0x64, 0x43, 0x2d, 0xe9, 0xd5, 0x39, 0xab, 0xd2,
  0xa2, 0xdf, 0xb7, 0x8b, 0xd0, 0xc0, 0x80, 0x19, 0x1c, 0x45, 0xc0, 0x2d,
  0x8c, 0xe8, 0xf8, 0x2d, 0xa4, 0x74, 0x56, 0x49, 0xc5, 0x05, 0xb5, 0x4f,
  0x15, 0xde, 0x6e, 0x44, 0x78, 0x39, 0x87, 0xa8, 0x7e, 0xbb, 0xf3, 0x79,
  0x18, 0x91, 0xbb, 0xf4, 0x6f, 0x9d, 0xc1, 0xf0, 0x8c, 0x35, 0x8c, 0x5d,
  0x01, 0xfb, 0xc3, 0x6d, 0xb9, 0xef, 0x44, 0x6d, 0x79, 0x46, 0x31, 0x7e,
  0x0a, 0xfe, 0xa9, 0x82, 0xc1, 0xff, 0xef, 0xab, 0x6e, 0x20, 0xc4, 0x50,
  0xc9, 0x5f, 0x9d, 0x4d, 0x9b, 0x17, 0x8c, 0x0c, 0xe5, 0x01, 0xc9, 0xa0,
  0x41, 0x6a, 0x73, 0x53, 0xfa, 0xa5, 0x50, 0xb4, 0x6e, 0x25, 0x0f, 0xfb,
  0x4c, 0x18, 0xf4, 0xfd, 0x52, 0xd9, 0x8e, 0x69, 0xb1, 0xe8, 0x11, 0x0f,
  0xde, 0x88, 0xd8, 0xfb, 0x1d, 0x49, 0xf7, 0xaa, 0xde, 0x95, 0xcf, 0x20,
  0x78, 0xc2, 0x60, 0x12, 0xdb, 0x25, 0x40, 0x8c, 0x6a, 0xfc, 0x7e, 0x42,
  0x38, 0x40, 0x64, 0x12, 0xf7, 0x9e, 0x81, 0xe1, 0x93, 0x2e
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
  assist_request.type.config.audio_out_config.encoding = google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_MP3;
  assist_request.type.config.audio_out_config.sample_rate_hertz = 16000;
  assist_request.type.config.which_type = google_assistant_embedded_v1alpha2_AssistConfig_text_query_tag;
  assist_request.type.config.screen_out_config.screen_mode = google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_PLAYING;
  safe_copy("en-US", assist_request.type.config.dialog_state_in.language_code);
  safe_copy(device_id, assist_request.type.config.device_config.device_id);
  safe_copy(device_model_id, assist_request.type.config.device_config.device_model_id);
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
    return false;
  }

  String body = String("client_id=") + client_id + "&" +
               "client_secret=" + client_secret + "&" +
               "grant_type=authorization_code&" +
               "redirect_uri=http://" + WiFi.localIP().toString() + "/oauth&" +
               "code=" + code;
  client.print(String("POST /oauth2/v4/token HTTP/1.1\r\n") +
               "Host: " + oauth_host + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Content-Length: " + body.length() + "\r\n" +
               "Connection: close\r\n\r\n");
  client.print(body);

  // Check status
  String line = client.readStringUntil('\n');
  if (!line.startsWith("HTTP/1.1 200 OK")) {
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
    return false;
  }

  File refreshTokenFile = SPIFFS.open("/refresh_token.txt", "r");
  if (!refreshTokenFile) {
    LOGLN("failed to read /refresh_token.txt");
    return false;
  }

  String body = String("client_id=") + client_id + "&" +
               "client_secret=" + client_secret + "&" +
               "grant_type=refresh_token&" +
               "refresh_token=";
  client.print(String("POST /oauth2/v4/token HTTP/1.1\r\n") +
               "Host: " + oauth_host + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Content-Length: " + (body.length() + refreshTokenFile.size()) + "\r\n" +
               "Connection: close\r\n\r\n");
  client.print(body);
  copyStreamToPrint(refreshTokenFile, client);
  refreshTokenFile.close();

  // Check status
  String line = client.readStringUntil('\n');
  if (!line.startsWith("HTTP/1.1 200 OK")) {
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
    return false;
  }

  String path = "/$rpc/google.assistant.embedded.v1alpha2.EmbeddedAssistant/Assist";
  LOG("requesting path: ");
  LOGLN(path);

  client.print(String("POST ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
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
  send_assistant_request(command);
}

bool assistant_init() {
  server.on("/oauth", handle_oauth);
  if (!client.setCACert_P(globalsign_cacert, sizeof(globalsign_cacert))) {
    LOGLN("Failed to load root CA certificate.");
    return false;
  }

  return true;
}
