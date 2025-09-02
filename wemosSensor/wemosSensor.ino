#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SHA256.h>  // Use only SHA256 from the Crypto library
#include <DHT.h>

#define DHTTYPE DHT11
#define DHTPIN D3

DHT dht(DHTPIN, DHTTYPE);
const char* ssid = "Bbox-A60CA003";
const char* password = "Bichon29";
WiFiUDP udp;
const IPAddress broadcastIP(255, 255, 255, 255);  // Broadcast address
IPAddress remoteIP;
const unsigned int port = 1642;
const char* secretKey = "cMsK";  // Shared secret key

const char* deviceID = "Temp1";  // Unique ID for each sensor

// Function to calculate HMAC-SHA256 manually
void calculateHMAC(uint8_t* output, const char* key, const char* message) {
  SHA256 sha256;
  const int blockSize = 64;  // Block size for HMAC with SHA256
  uint8_t keyBlock[blockSize];
  uint8_t o_key_pad[blockSize];
  uint8_t i_key_pad[blockSize];

  // If key is longer than blockSize, hash it first
  int keyLen = strlen(key);
  if (keyLen > blockSize) {
    sha256.reset();
    sha256.update((const uint8_t*)key, keyLen);
    sha256.finalize(keyBlock, sizeof(keyBlock));
    keyLen = sha256.hashSize();
  } else {
    memcpy(keyBlock, key, keyLen);
    memset(keyBlock + keyLen, 0, blockSize - keyLen);  // Zero-pad the key
  }

  // Create o_key_pad and i_key_pad
  for (int i = 0; i < blockSize; i++) {
    o_key_pad[i] = keyBlock[i] ^ 0x5C;
    i_key_pad[i] = keyBlock[i] ^ 0x36;
  }

  // Inner hash: hash(i_key_pad + message)
  sha256.reset();
  sha256.update(i_key_pad, blockSize);
  sha256.update((const uint8_t*)message, strlen(message));
  uint8_t innerHash[sha256.hashSize()];
  sha256.finalize(innerHash, sha256.hashSize());

  // Outer hash: hash(o_key_pad + innerHash)
  sha256.reset();
  sha256.update(o_key_pad, blockSize);
  sha256.update(innerHash, sha256.hashSize());
  sha256.finalize(output, sha256.hashSize());
}

bool registered = false;  // Keeps track of whether registration was acknowledged
unsigned long lastRegistrationAttempt = 0;

void sendRegistration() {
  // Skip registration if already acknowledged
  if (registered) return;

  // Construct registration message
  char message[50];
  snprintf(message, sizeof(message), "REGISTER %s", deviceID);

  // Generate HMAC for authentication
  uint8_t hmac[32];
  calculateHMAC(hmac, secretKey, message);

  // Convert HMAC to hexadecimal string
  char hmacHex[65];
  for (int i = 0; i < 32; i++) {
    sprintf(hmacHex + i * 2, "%02x", hmac[i]);
  }

  // Send message and HMAC over UDP
  udp.beginPacket(broadcastIP, port);
  udp.write(message);
  udp.write(";");
  udp.write(hmacHex);
  udp.endPacket();

  Serial.println("Registration message sent.");

  // Update the last attempt time
  lastRegistrationAttempt = millis();
}

// Function to check for acknowledgment
void checkForAck(const char* packetBuffer) {
  char* message = strtok((char*)packetBuffer, "|");
  char* receivedHMAC = strtok(NULL, "|");

  if (message && receivedHMAC) {
    uint8_t expectedHMAC[32];
    calculateHMAC(expectedHMAC, secretKey, message);

    // If "ACK" received, set registered flag to true
    if (strstr(message, "ACK") == message) {
      registered = true;
      remoteIP = udp.remoteIP();
      Serial.println("Acknowledgment received. Registration complete.");
    }
  } else {
    Serial.println("Invalid packet format.");
  }
}

void sendMessage(const char* payload) {
  // Define the message buffer size, adjusting as needed
  char message[1024];

  // Format the message with deviceID and payload
  snprintf(message, sizeof(message), "DATA %s | %s", deviceID, payload);

  // Generate HMAC for authentication
  uint8_t hmac[32];
  calculateHMAC(hmac, secretKey, message);

  // Convert HMAC to hexadecimal string
  char hmacHex[65];  // Each byte to hex requires 2 characters + null terminator
  for (int i = 0; i < 32; i++) {
    snprintf(hmacHex + i * 2, 3, "%02x", hmac[i]);  // 2 chars + null terminator
  }

  // Begin UDP packet
  udp.beginPacket(remoteIP, port);

  // Send the message and HMAC, separated by a semicolon
  udp.write(message);
  udp.write(";");
  udp.write(hmacHex);
  bool packetSent = udp.endPacket();  // Check if sending was successful

  // Debugging output to Serial
  Serial.print("Message sent: ");
  Serial.println(message);

  if (packetSent) {
    Serial.println("Packet successfully sent.");
  } else {
    Serial.println("Failed to send packet.");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  pinMode(D4, OUTPUT);
  digitalWrite(D4, HIGH);
  Serial.println("Connected to Wi-Fi.");
  udp.begin(port);
  dht.begin();
}

void loop() {
  if (!registered && millis() - lastRegistrationAttempt > 5000) {
    // Attempt registration every 5 seconds until acknowledged
    sendRegistration();
  }

  delay(1000);  // Delay before checking for response
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[255];
    int len = udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = '\0';
    checkForAck(packetBuffer);  // Process the packet
  }

  String temperature = String(dht.readTemperature());
  delay(500);
  String humidity = String(dht.readHumidity());

  if (registered) {
    // Read temperature and humidity from DHT11 sensor
    String payload = String(temperature + " | " + humidity);
    sendMessage(payload.c_str());
  }
}
