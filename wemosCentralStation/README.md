# Wemos Central Station

A secure Arduino-based central station for managing IoT device registrations and data collection using HMAC-SHA256 authentication.

## Overview

This project implements a central station on a Wemos D1 Mini (ESP8266) that acts as a hub for IoT devices. It provides secure device registration and data collection capabilities using HMAC-SHA256 message authentication to ensure only authorized devices can communicate with the station.

## Features

- **Secure Device Registration**: IoT devices can register with the central station using HMAC-SHA256 authentication
- **Data Collection**: Registered devices can send data packets to the central station
- **Device Management**: Tracks up to 10 registered devices with their IP addresses and ports
- **UDP Communication**: Uses UDP protocol for lightweight communication
- **WiFi Connectivity**: Connects to WiFi networks for network communication
- **Serial Debugging**: Comprehensive serial output for monitoring and debugging

## Hardware Requirements

- Wemos D1 Mini (ESP8266-based board)
- USB cable for programming and power
- WiFi network access

## Dependencies

- ESP8266WiFi library (built-in)
- WiFiUDP library (built-in)
- SHA256 library (from the Crypto library)

## Configuration

Before uploading the code, you need to configure the following parameters in `wemosCentralStation.ino`:

```cpp
const char* ssid = "*****";           // Your WiFi network name
const char* password = "*****";        // Your WiFi password
const unsigned int port = 1642;        // UDP port to listen on
const char* secretKey = "cMsK";        // Shared secret key for HMAC verification
```

## Communication Protocol

### Message Format

All messages use the following format:
```
MESSAGE;HMAC_SIGNATURE
```

Where:
- `MESSAGE` is the actual message content
- `HMAC_SIGNATURE` is the HMAC-SHA256 signature of the message using the shared secret key

### Message Types

#### 1. Registration Message
```
REGISTER;DEVICE_ID;HMAC_SIGNATURE
```

**Response**: Acknowledgment with HMAC verification
```
ACK|HMAC_SIGNATURE
```

#### 2. Data Message
```
DATA;PAYLOAD;HMAC_SIGNATURE
```

**Response**: No acknowledgment sent (fire-and-forget)

### HMAC Calculation

The HMAC-SHA256 is calculated using the standard HMAC algorithm:
1. If the key is longer than 64 bytes, hash it first
2. Create inner and outer key pads
3. Calculate inner hash: H(key_inner ⊕ 0x36 || message)
4. Calculate outer hash: H(key_outer ⊕ 0x5C || inner_hash)

## Device Management

The central station maintains a registry of up to 10 devices:

```cpp
struct RegisteredDevice {
  IPAddress ip;        // Device IP address
  unsigned int port;   // Device port
  String deviceID;     // Unique device identifier
};
```

## Setup Instructions

1. **Install Arduino IDE** with ESP8266 board support
2. **Install Dependencies**:
   - ESP8266WiFi (usually built-in)
   - WiFiUDP (usually built-in)
   - SHA256 from the Crypto library
3. **Configure WiFi Settings** in the code
4. **Upload to Wemos D1 Mini**
5. **Monitor Serial Output** at 115200 baud rate

## Usage

### Starting Up
1. Power on the Wemos D1 Mini
2. The device will attempt to connect to WiFi
3. Once connected, it will start listening on the configured UDP port
4. Monitor the serial output for connection status

### Device Registration
1. IoT devices send registration messages with their unique ID
2. The central station verifies the HMAC signature
3. If valid, the device is added to the registry
4. An acknowledgment is sent back to the device

### Data Collection
1. Registered devices can send data messages
2. Data is processed and logged to serial output
3. No acknowledgment is sent for data messages

## Security Features

- **HMAC-SHA256 Authentication**: All messages are cryptographically verified
- **Shared Secret Key**: Only devices with the correct secret key can communicate
- **Message Integrity**: HMAC ensures messages haven't been tampered with
- **Device Registry**: Tracks authorized devices by IP and port

## Limitations

- Maximum of 10 registered devices
- UDP communication (no guaranteed delivery)
- No persistent storage of device registry
- No encryption of message payloads (only authentication)

## Troubleshooting

### Common Issues

1. **WiFi Connection Failed**
   - Check SSID and password
   - Ensure WiFi network is accessible
   - Check signal strength

2. **UDP Communication Issues**
   - Verify port number configuration
   - Check firewall settings
   - Ensure devices are on the same network

3. **HMAC Verification Failed**
   - Verify secret key matches between devices
   - Check message format
   - Ensure proper HMAC calculation on device side

### Debug Information

The serial monitor provides detailed information about:
- WiFi connection status
- Device registration attempts
- Data message reception
- HMAC verification results
- Error conditions

## Example Device Communication

### Registration Example
```
Device sends: REGISTER;SENSOR_001;abc123...
Station responds: ACK|def456...
```

### Data Example
```
Device sends: DATA;TEMP:23.5;xyz789...
Station logs: "Received data from device: TEMP:23.5"
```

## Contributing

This is a basic implementation that can be extended with:
- Persistent storage of device registry
- Message encryption
- Web interface for monitoring
- Database integration
- Multiple communication protocols

## License

This project is open source. Please check the individual library licenses for dependencies.

## Support

For issues and questions, please check the serial output for debugging information and ensure all configuration parameters are correctly set.
