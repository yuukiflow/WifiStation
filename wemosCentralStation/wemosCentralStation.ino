#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SHA256.h>  // Use only SHA256 from the Crypto library

const char* ssid = "Bbox-A60CA003";
const char* password = "Bichon29";
WiFiUDP udp;
const unsigned int port = 1642;
const char* secretKey = "cMsK";  // Shared secret key for HMAC verification

// Define a struct to store registered device information
struct RegisteredDevice {
  IPAddress ip;
  unsigned int port;
  String deviceID;
};

RegisteredDevice devices[10];  // Array to store registered devices
int deviceCount = 0;           // Counter for the number of registered devices

// Function to calculate HMAC-SHA256 for verification
void calculateHMAC(uint8_t* output, const char* key, const char* message) {
  SHA256 sha256;
  const int blockSize = 64;
  uint8_t keyBlock[blockSize];
  uint8_t o_key_pad[blockSize];
  uint8_t i_key_pad[blockSize];

  // If key is longer than blockSize, hash it
  int keyLen = strlen(key);
  if (keyLen > blockSize) {
    sha256.reset();
    sha256.update((const uint8_t*)key, keyLen);
    sha256.finalize(keyBlock, sizeof(keyBlock));
    keyLen = sha256.hashSize();
  } else {
    memcpy(keyBlock, key, keyLen);
    memset(keyBlock + keyLen, 0, blockSize - keyLen);
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

// Function to add a device to the registered list
void addDevice(IPAddress ip, unsigned int port, const char* deviceID) {
  if (deviceCount < 10) {
    devices[deviceCount++] = { ip, port, String(deviceID) };
    Serial.print("Device ");
    Serial.print(deviceID);
    Serial.println(" registered successfully.");
  } else {
    Serial.println("Device list is full.");
  }
}

// Function to check if a device is already registered
bool isDeviceRegistered(const char* deviceID) {
  for (int i = 0; i < deviceCount; i++) {
    if (devices[i].deviceID == deviceID) return true;
  }
  return false;
}

// Function to handle incoming registration and data packets
// Helper function to convert hex string to binary
void hexToBinary(const char* hex, uint8_t* binary, int binarySize) {
  for (int i = 0; i < binarySize; i++) {
    sscanf(hex + 2 * i, "%2hhx", &binary[i]);
  }
}

void binaryToHex(const uint8_t* binary, char* hex, size_t length) {
  const char* hexChars = "0123456789ABCDEF";
  for (size_t i = 0; i < length; ++i) {
    hex[i * 2] = hexChars[(binary[i] >> 4) & 0x0F];
    hex[i * 2 + 1] = hexChars[binary[i] & 0x0F];
  }
  hex[length * 2] = '\0';  // Null-terminate the hex string
}


void handleIncomingPacket(const char* packetBuffer) {
  char* message = strtok((char*)packetBuffer, ";");
  char* receivedHMAC = strtok(NULL, ";");

  if (message && receivedHMAC) {
    uint8_t expectedHMAC[32];
    calculateHMAC(expectedHMAC, secretKey, message);

    uint8_t receivedHMACBinary[32];
    hexToBinary(receivedHMAC, receivedHMACBinary, 32);


    if (memcmp(receivedHMACBinary, expectedHMAC, 32) == 0) {
      // Check if the message is a registration or data message
      if (strstr(message, "REGISTER") == message) {
        // Handle registration
        char* deviceID = strtok(message + 9, ";");

        if (deviceID) {
          if (!isDeviceRegistered(deviceID)) {
            addDevice(udp.remoteIP(), udp.remotePort(), deviceID);
            Serial.print("Device ");
            Serial.print(deviceID);
            Serial.println(" registered successfully.");
          } else {
            Serial.print("Device ");
            Serial.print(deviceID);
            Serial.println(" is already registered.");
          }

          // Send acknowledgment back to the sensor only for registration
          udp.beginPacket(udp.remoteIP(), udp.remotePort());
          char ackMessage[64];
          uint8_t ackHMAC[32];
          calculateHMAC(ackHMAC, secretKey, "ACK");
          binaryToHex(ackHMAC, ackMessage, 32);
          udp.write("ACK|");
          udp.write(ackMessage);
          udp.endPacket();
          Serial.println("Acknowledgment sent for REGISTER.");
        }

      } else if (strstr(message, "DATA") == message) {
        // Handle data message without sending an acknowledgment
        char* dataPayload = strtok(message + 5, ";");  // Extract data payload starting after "DATA|"

        if (dataPayload) {
          Serial.print("Received data from device: ");
          Serial.println(dataPayload);

          // Process the data payload as needed (e.g., log, store, etc.)
        }
        // No acknowledgment is sent for DATA messages
      }
    } else {
      Serial.println("Unauthorized device detected (HMAC verification failed).");
    }
  } else {
    Serial.println("Invalid message format.");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi.");
  udp.begin(port);  // Start listening on the specified port
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[255];
    int len = udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = '\0';
    handleIncomingPacket(packetBuffer);  // Process the packet
  }
}
