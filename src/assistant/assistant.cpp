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

static CertStore certificate_store;
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
  LOGLN("Start refresh_oauth");
  if (!client.connect(oauth_host, httpsPort)) {
    LOGLN("connection failed");
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
  LOGLN("Start send_assistant_request");
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

  int certificate_count = certificate_store.initCertStore(SPIFFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
  LOG("Read ");
  LOG(certificate_count);
  LOGLN(" CA certificates");
  if (certificate_count == 0) {
    LOGLN("Failed to load CA certificates.");
    return false;
  }
  client.setCertStore(&certificate_store);

  return true;
}
