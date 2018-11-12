#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <FS.h>

// Pin which is connected via a resistor to CH_PD, to latch power on
#define EN_PIN 4

const char* ssid = "";
const char* password = "";

const char *oauth_host = "www.googleapis.com";
const char* host = "embeddedassistant.googleapis.com";
const int httpsPort = 443;

const char *client_id = "";
const char *client_secret = "";

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "25 94 76 8C E2 3C 5C 74 08 A1 1F B5 F2 09 B8 19 B3 99 14 95";

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

// Read all headers from a client, printing them to Serial for debugging.
void skip_headers(WiFiClientSecure &client) {
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
}

void store_token(const char *token) {
  File tokenFile = SPIFFS.open("/token.txt", "w");
  if (!tokenFile) {
    Serial.println("Failed to open /token.txt for writing.");
    return;
  }
  tokenFile.println(token);
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
    copyStreamToPrint(client, Serial);
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
  store_token(token);
  client.stop();
  return true;
}

void wifi_connect() {
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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

  // Check for success
  String line = client.readStringUntil('\n');
  if (line.startsWith("HTTP/1.1 200 OK")) {
    Serial.println("success");
    client.stop();
    return true;
  }
  Serial.println(line);
  // Print headers
  skip_headers(client);

  client.stop();
  return false;
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

//////////////////
// Entry points //
//////////////////

void setup() {
  // Keep power on until we're done
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH);

  Serial.begin(115200);
  Serial.println();

  wifi_connect();

  SPIFFS.begin();
  auth_and_send_request();
  SPIFFS.end();

  // Power can go off, if we're wired up that way.
  digitalWrite(EN_PIN, LOW);
  // Deep sleep until RESET is taken low.
  Serial.println("sleeping");
  ESP.deepSleep(0);
  Serial.println("done sleeping");
}

void loop() {
}
