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

#include <ArduinoJson.h>
#include <DoubleResetDetect.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <pb_arduino.h>
#include <WiFiClientSecure.h>

#include "embedded_assistant.pb.h"
#include "stream_body.pb.h"

// Pin which is connected via a resistor to CH_PD, to latch power on
#define EN_PIN 4
#define LED_PIN 2
#define REQUEST_BUFFER_SIZE 200

const char *oauth_host = "www.googleapis.com";
const char* host = "embeddedassistant.googleapis.com";
const int httpsPort = 443;

const char *client_id = "";
const char *client_secret = "";
const char *device_id = "my_device_id";
const char *device_model_id = "";

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "25 94 76 8C E2 3C 5C 74 08 A1 1F B5 F2 09 B8 19 B3 99 14 95";

#define DRD_TIMEOUT 0.5
#define DRD_ADDRESS 0x00

#define NETWORK_LOGGING 0

// Change this to disable logging
#if NETWORK_LOGGING
WiFiServer log_server(2222);
WiFiClient log_client;

WiFiClient get_log_client() {
  if (log_client.status() == CLOSED) {
    log_client = log_server.available();
  }
  return log_client;
}

#define LOG(x) get_log_client().print(x)
#define LOGH(x) get_log_client().print(x, HEX)
#define LOGLN(x) get_log_client().println(x)
#elif 1
#define LOG(x) Serial.print(x)
#define LOGH(x) Serial.print(x, HEX)
#define LOGLN(x) Serial.println(x)
#else
#define LOG(x)
#define LOGH(x)
#define LOGLN(x)
#endif

ESP8266WebServer server(80);
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

// Copy from a char * to a char[n] buffer without overrunning the buffer, making sure to end with a \0.
#define safe_copy(src, dest) snprintf((dest), sizeof(dest), "%s", (src))

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

// Read all remaining response from a client, printing it to the log for debugging.
void print_response(WiFiClientSecure &client) {
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    LOGLN(line);
  }
}

bool store_token(const char *path, const char *token) {
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
  assist_request.config.audio_out_config.encoding = google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_MP3;
  assist_request.config.audio_out_config.sample_rate_hertz = 16000;
  assist_request.config.audio_in_config.encoding = google_assistant_embedded_v1alpha2_AudioInConfig_Encoding_LINEAR16;
  assist_request.config.audio_in_config.sample_rate_hertz = 16000;
  assist_request.config.screen_out_config.screen_mode = google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_PLAYING;
  safe_copy("en-US", assist_request.config.dialog_state_in.language_code);
  safe_copy(device_id, assist_request.config.device_config.device_id);
  safe_copy(device_model_id, assist_request.config.device_config.device_model_id);
  safe_copy(command, assist_request.config.text_query);

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

// Encode the given command as the appropriate protobuf and write it to /request.pb.
bool update_command(const char *command) {
  LOG("Updating command to \"");
  LOG(command);
  LOGLN("\"");

  return store_token("/command.txt", command);
}

String read_line_from_file(const char *filename) {
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    LOG("Failed to open ");
    LOG(file);
    LOGLN(" for reading.");
    return String();
  }
  String token = file.readStringUntil('\n');
  file.close();
  return token;
}

String load_token() {
  return read_line_from_file("/token.txt");
}

String load_command() {
  return read_line_from_file("/command.txt");
}

// Given an OAuth code, get a new token and refresh token and store them.
bool oauth_with_code(const String &code) {
  WiFiClientSecure client;
  if (!client.connect(oauth_host, httpsPort)) {
    LOGLN("connection failed");
    return false;
  }

  if (client.verify(fingerprint, oauth_host)) {
    LOGLN("certificate matches");
  } else {
    LOGLN("certificate doesn't match");
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
  store_token("/refresh_token.txt", refresh_token);
  store_token("/token.txt", token);
  client.stop();
  return true;
}

// Use the refresh token to get a new auth token.
// Return true on success.
bool refresh_oauth() {
  WiFiClientSecure client;
  if (!client.connect(oauth_host, httpsPort)) {
    LOGLN("connection failed");
    return false;
  }

  if (client.verify(fingerprint, oauth_host)) {
    LOGLN("certificate matches");
  } else {
    LOGLN("certificate doesn't match");
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
  store_token("/token.txt", token);
  client.stop();
  return true;
}

bool wifi_connect() {
  // Read SSID and password from file.
  File wifiFile = SPIFFS.open("/wifi.txt", "r");
  if (!wifiFile) {
    LOGLN("Failed to open /wifi.txt for reading.");
    return false;
  }
  String ssid = wifiFile.readStringUntil('\n');
  String password = wifiFile.readStringUntil('\n');
  wifiFile.close();

  LOG("connecting to '");
  LOG(ssid.c_str());
  LOG("' with password '");
  LOG(password.c_str());
  LOGLN("'");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  //WiFi.begin(ssid, password);
  // Try to connect for 5 seconds
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; ++i) {
    delay(500);
    LOG(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    LOGLN("Couldn't connect");
    return false;
  }
  LOGLN("");
  LOGLN("WiFi connected");
  LOGLN("IP address: ");
  LOGLN(WiFi.localIP());

  return true;
}

// Connect to the configured network if possible, or else run as an access point.
// Returns false if it was unable to connect to the configured network and so is in AP mode.
bool wifi_setup() {
  if (wifi_connect()) {
    #if NETWORK_LOGGING
    log_server.begin();
    log_server.setNoDelay(true);
    #endif

    return true;
  }

  // Couldn't connect, try being an AP.
  IPAddress local_ip(192, 168, 0, 1);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP("qButton");

  #if NETWORK_LOGGING
  log_server.begin();
  log_server.setNoDelay(true);
  #endif

  LOGLN("Running AP qButton. Local IP address:");
  LOGLN(WiFi.softAPIP());

  return false;
}

// Return true on success, false on failure for any reason.
bool send_assistant_request() {
  String token = load_token();
  String command = load_command();
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
  WiFiClientSecure client;
  LOG("connecting to ");
  LOGLN(host);
  if (!client.connect(host, httpsPort)) {
    LOGLN("connection failed");
    return false;
  }

  if (client.verify(fingerprint, host)) {
    LOGLN("certificate matches");
  } else {
    LOGLN("certificate doesn't match");
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
void auth_and_send_request() {
  if (send_assistant_request()) {
    return;
  }
  LOGLN("First request failed, refreshing token");
  if (!refresh_oauth()) {
    return;
  }
  send_assistant_request();
}

///////////////////////////
// HTTP request handlers //
///////////////////////////

void handle_root() {
  // If a new SSID and password have been sent, save them.
  const String &new_ssid = server.arg("ssid");
  const String &new_password = server.arg("password");
  const String &new_command = server.arg("command");
  String error;
  if (new_ssid.length() > 0) {
    File wifiFile = SPIFFS.open("/wifi.txt", "w");
    if (!wifiFile) {
      LOGLN("Failed to open /wifi.txt for writing.");
      error = "Failed to open /wifi.txt for writing";
    }
    // Don't use println, because it adds '\r' characters which we don't want.
    wifiFile.print(new_ssid);
    wifiFile.print('\n');
    wifiFile.print(new_password);
    wifiFile.print('\n');
    wifiFile.close();
    LOGLN("wrote new SSID and password");
    LOGLN(new_ssid);
  }
  if (new_command.length() > 0) {
    update_command(new_command.c_str());
    auth_and_send_request();
  }

  // Read whatever is on disk.
  String ssid, password;
  File wifiFile = SPIFFS.open("/wifi.txt", "r");
  if (!wifiFile) {
    LOGLN("Failed to open /wifi.txt for reading.");
  } else {
    ssid = wifiFile.readStringUntil('\n');
    password = wifiFile.readStringUntil('\n');
    wifiFile.close();
  }

  server.send(200, "text/html", String("<html><head><title>qButton config</title></head><body><h1>qButton config</h1>") +
    "<p style=\"color: red;\">" + error + "</p>" +
    "<a href=\"https://accounts.google.com/o/oauth2/v2/auth?client_id=" + client_id +
    "&scope=https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fassistant-sdk-prototype&access_type=offline&response_type=code&redirect_uri=http://" +
    WiFi.localIP().toString() + "/oauth&device_id=device_id&device_name=device_name\">Set account</a>" +
    "<form method=\"post\" action=\"/\">" +
    "SSID: <input type=\"text\" name=\"ssid\" value=\"" + ssid + "\"/><br/>" +
    "Password: <input type=\"text\" name=\"password\" value=\"" + password + "\"/><br/>" +
    "<input type=\"submit\" value=\"Update WiFi config\"/>" +
    "</form><form method=\"post\" action=\"/\">" +
    "Command: <input type=\"text\" name=\"command\" value=\"" + load_command() + "\">" +
    "<input type=\"submit\" value=\"Update command\"/>" +
     "</form></body></html>");
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

// Run web server to let the user authenticate their account.
void start_webserver() {
  server.on("/", handle_root);
  server.on("/oauth", handle_oauth);
  server.begin();
  LOGLN("HTTP server started");
}

//////////////////
// Entry points //
//////////////////

void setup() {
  // Keep power on until we're done
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH);

  bool double_reset = drd.detect();

  Serial.begin(115200);
  Serial.println();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  if (double_reset) {
    LOGLN("detected double reset");
  }

  SPIFFS.begin();

  // No point trying to send the Google Assistant request if we are running in AP mode.
  if (wifi_setup()) {
    auth_and_send_request();

    // If the user double-presses the reset button, skip sleeping so that they can reconfigure it.
    if (!double_reset) {
      // Go to sleep and/or turn off.
      SPIFFS.end();
      LOGLN("sleeping");
      // Power can go off, if we're wired up that way.
      digitalWrite(EN_PIN, LOW);
      // Deep sleep until RESET is taken low.
      ESP.deepSleep(0);

      LOGLN("done sleeping");
      digitalWrite(EN_PIN, HIGH);
      SPIFFS.begin();
    }
  }

  if (!MDNS.begin("qbutton")) {
    LOGLN("Error starting mDNS");
  }

  start_webserver();
}

void loop() {
  server.handleClient();
}
