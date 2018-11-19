#include <ArduinoJson.h>
#include <DoubleResetDetect.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <WiFiClientSecure.h>

// Pin which is connected via a resistor to CH_PD, to latch power on
#define EN_PIN 4
#define LED_PIN 2

const char *oauth_host = "www.googleapis.com";
const char* host = "embeddedassistant.googleapis.com";
const int httpsPort = 443;

const char *client_id = "";
const char *client_secret = "";

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "25 94 76 8C E2 3C 5C 74 08 A1 1F B5 F2 09 B8 19 B3 99 14 95";

#define DRD_TIMEOUT 0.5
#define DRD_ADDRESS 0x00

ESP8266WebServer server(80);
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

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

// Read all remaining response from a client, printing it to Serial for debugging.
void print_response(WiFiClientSecure &client) {
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }
}

void store_token(const char *path, const char *token) {
  File tokenFile = SPIFFS.open(path, "w");
  if (!tokenFile) {
    Serial.print("Failed to open ");
    Serial.print(path);
    Serial.println(" for writing.");
    return;
  }
  // Don't use println, because it adds '\r' characters which we don't want.
  tokenFile.print(token);
  tokenFile.print('\n');
  tokenFile.close();
}

String load_token() {
  File tokenFile = SPIFFS.open("/token.txt", "r");
  if (!tokenFile) {
    Serial.println("Failed to open /token.txt for reading.");
    return String();
  }
  String token = tokenFile.readStringUntil('\n');
  tokenFile.close();
  return token;
}

// Given an OAuth code, get a new token and refresh token and store them.
bool oauth_with_code(const String &code) {
  WiFiClientSecure client;
  if (!client.connect(oauth_host, httpsPort)) {
    Serial.println("connection failed");
    return false;
  }

  if (client.verify(fingerprint, oauth_host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
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
    Serial.println(line);
    print_response(client);
    client.stop();
    return false;
  }

  // Skip headers
  client.find("\r\n\r\n");

  DynamicJsonBuffer jb;
  JsonObject &root = jb.parseObject(client);
  if (!root.success()) {
    Serial.println("Failed to parse JSON response from OAuth");
    return false;
  }
  const char *token = root["access_token"];
  const char *refresh_token = root["refresh_token"];
  Serial.print("got new access token ");
  Serial.println(token);
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
    Serial.println("connection failed");
    return false;
  }

  if (client.verify(fingerprint, oauth_host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  File refreshTokenFile = SPIFFS.open("/refresh_token.txt", "r");
  if (!refreshTokenFile) {
    Serial.println("failed to read /refresh_token.txt");
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
    Serial.println(line);
    print_response(client);
    client.stop();
    return false;
  }

  // Skip headers
  client.find("\r\n\r\n");

  DynamicJsonBuffer jb;
  JsonObject &root = jb.parseObject(client);
  if (!root.success()) {
    Serial.println("Failed to parse JSON response from OAuth refresh");
    return false;
  }
  const char *token = root["access_token"];
  Serial.print("got new access token ");
  Serial.println(token);
  store_token("/token.txt", token);
  client.stop();
  return true;
}

bool wifi_connect() {
  // Read SSID and password from file.
  File wifiFile = SPIFFS.open("/wifi.txt", "r");
  if (!wifiFile) {
    Serial.println("Failed to open /wifi.txt for reading.");
    return false;
  }
  String ssid = wifiFile.readStringUntil('\n');
  String password = wifiFile.readStringUntil('\n');
  wifiFile.close();

  Serial.print("connecting to '");
  Serial.print(ssid.c_str());
  Serial.print("' with password '");
  Serial.print(password.c_str());
  Serial.println("'");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  //WiFi.begin(ssid, password);
  // Try to connect for 5 seconds
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; ++i) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Couldn't connect");
    return false;
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  return true;
}

// Connect to the configured network if possible, or else run as an access point.
// Returns false if it was unable to connect to the configured network and so is in AP mode.
bool wifi_setup() {
  if (wifi_connect()) {
    return true;
  }

  // Couldn't connect, try being an AP.
  IPAddress local_ip(192, 168, 0, 1);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP("qButton");
  Serial.println("Running AP qButton. Local IP address:");
  Serial.println(WiFi.softAPIP());

  return false;
}

// Return true on success, false on failure for any reason.
bool send_assistant_request() {
  String token = load_token();
  File requestFile = SPIFFS.open("/request.pb", "r");
  if (!requestFile) {
    Serial.println("failed to read /request.pb");
    SPIFFS.end();
    return false;
  }

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return false;
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  String path = "/$rpc/google.assistant.embedded.v1alpha2.EmbeddedAssistant/Assist";
  Serial.print("requesting path: ");
  Serial.println(path);

  client.print(String("POST ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Content-Type: application/x-protobuf\r\n" +
               "Authorization: Bearer " + token + "\r\n" +
               "Content-Length: " + requestFile.size() + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("headers sent");
  copyStreamToPrint(requestFile, client);
  Serial.println("body sent");
  unsigned long sent_request_time = millis();

  // Check status
  String line = client.readStringUntil('\n');
  if (!line.startsWith("HTTP/1.1 200 OK")) {
    Serial.println(line);
    print_response(client);
    client.stop();
    return false;
  }

  Serial.print("success after ");
  Serial.print(millis() - sent_request_time);
  Serial.println(" ms");
  client.stop();
  return true;
}

// Send the request to Google Assistant, refreshing the auth token if necessary.
void auth_and_send_request() {
  if (send_assistant_request()) {
    return;
  }
  Serial.println("First request failed, refreshing token");
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
  if (new_ssid.length() > 0) {
    File wifiFile = SPIFFS.open("/wifi.txt", "w");
    if (!wifiFile) {
      Serial.println("Failed to open /wifi.txt for writing.");
      server.send(500, "text/plain", "Failed to open /wifi.txt for writing");
      return;
    }
    // Don't use println, because it adds '\r' characters which we don't want.
    wifiFile.print(new_ssid);
    wifiFile.print('\n');
    wifiFile.print(new_password);
    wifiFile.print('\n');
    wifiFile.close();
    Serial.println("wrote new SSID and password");
    Serial.println(new_ssid);
  }

  // Read whatever is on disk.
  String ssid, password;
  File wifiFile = SPIFFS.open("/wifi.txt", "r");
  if (!wifiFile) {
    Serial.println("Failed to open /wifi.txt for reading.");
  } else {
    ssid = wifiFile.readStringUntil('\n');
    password = wifiFile.readStringUntil('\n');
    wifiFile.close();
  }

  server.send(200, "text/html", String("<html><head><title>qButton config</title></head><body><h1>qButton config</h1>") +
    "<a href=\"https://accounts.google.com/o/oauth2/v2/auth?client_id=" + client_id +
    "&scope=https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fassistant-sdk-prototype&access_type=offline&response_type=code&redirect_uri=http://" +
    WiFi.localIP().toString() + "/oauth&device_id=device_id&device_name=device_name\">Set account</a>" +
    "<form method=\"post\" action=\"/\">" +
    "SSID: <input type=\"text\" name=\"ssid\" value=\"" + ssid + "\"/><br/>" +
    "Password: <input type=\"text\" name=\"password\" value=\"" + password + "\"/><br/>" +
    "<input type=\"submit\" value=\"Update\"/>" +
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
  Serial.println("HTTP server started");
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

  if (double_reset) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    Serial.println("detected double reset");
  }

  SPIFFS.begin();

  // No point trying to send the Google Assistant request if we are running in AP mode.
  if (wifi_setup()) {
    auth_and_send_request();

    // If the user double-presses the reset button, skip sleeping so that they can reconfigure it.
    if (!double_reset) {
      // Go to sleep and/or turn off.
      SPIFFS.end();
      // Power can go off, if we're wired up that way.
      digitalWrite(EN_PIN, LOW);
      // Deep sleep until RESET is taken low.
      Serial.println("sleeping");
      ESP.deepSleep(0);

      Serial.println("done sleeping");
      SPIFFS.begin();
    }
  }

  if (!MDNS.begin("qbutton")) {
    Serial.println("Error starting mDNS");
  }

  start_webserver();
}

void loop() {
  server.handleClient();
}
