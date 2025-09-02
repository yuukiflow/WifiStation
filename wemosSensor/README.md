# Wemos D1 Mini Sensor Station

A secure, authenticated temperature and humidity sensor station built with a Wemos D1 Mini (ESP8266) that communicates over UDP with HMAC-SHA256 authentication.

## Overview

This project implements a wireless sensor node that:
- Reads temperature and humidity data from a DHT11 sensor
- Connects to WiFi and registers with a central station
- Sends authenticated sensor data using HMAC-SHA256
- Implements a secure handshake protocol for device registration

## Hardware Requirements

- **Wemos D1 Mini** (ESP8266-based development board)
- **DHT11** temperature and humidity sensor
- **Breadboard and jumper wires** for connections
- **USB cable** for programming and power

## Pin Connections

| Wemos D1 Mini Pin | DHT11 Pin | Description |
|-------------------|-----------|-------------|
| D3 (GPIO0)       | Data      | DHT11 data line |
| D4 (GPIO2)       | -         | Status LED indicator |
| 3.3V             | VCC       | Power supply |
| GND              | GND       | Ground |

## Features

### 🔐 Security
- **HMAC-SHA256 authentication** for all communications
- **Shared secret key** for device verification
- **Message integrity** protection against tampering

### 📡 Communication Protocol
- **UDP broadcast** for device discovery
- **Registration handshake** with acknowledgment
- **Authenticated data transmission** to registered station
- **Automatic re-registration** if connection is lost

### 🌡️ Sensor Support
- **DHT11 sensor** for temperature and humidity readings
- **Real-time data** transmission every 1.5 seconds
- **Error handling** for sensor read failures

## Configuration

Before uploading the code, modify these constants in `wemosSensor.ino`:

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* secretKey = "YOUR_SECRET_KEY";  // Must match the central station
const char* deviceID = "Temp1";             // Unique identifier for this sensor
const unsigned int port = 1642;             // UDP port for communication
```

## How It Works

### 1. Initialization
- Connects to WiFi network
- Initializes DHT11 sensor
- Starts UDP listener on specified port

### 2. Registration Process
- Broadcasts registration message with device ID
- Includes HMAC-SHA256 signature for authentication
- Waits for acknowledgment from central station
- Retries every 5 seconds until acknowledged

### 3. Data Transmission
- Once registered, reads sensor data every 1.5 seconds
- Formats data as: `"DATA Temp1 | 23.50 | 45.20"`
- Signs each message with HMAC-SHA256
- Sends to registered central station IP address

### 4. Message Format
```
DATA Temp1 | 23.50 | 45.20;abc123def456...
[Message Content];[HMAC-SHA256 Signature]
```

## Dependencies

This project requires the following Arduino libraries:
- `ESP8266WiFi` - WiFi functionality for ESP8266
- `WiFiUdp` - UDP communication
- `SHA256` - Cryptographic hashing (from Crypto library)
- `DHT` - DHT sensor library

## Installation

1. **Install Arduino IDE** and ESP8266 board support
2. **Install required libraries** via Library Manager:
   - DHT sensor library
   - Crypto library (for SHA256)
3. **Connect hardware** according to pin diagram
4. **Configure WiFi credentials** and secret key
5. **Upload code** to Wemos D1 Mini

## Central Station Requirements

To receive data from this sensor, you need a central station that:
- Listens on UDP port 1642
- Implements the same HMAC-SHA256 verification
- Sends acknowledgment messages in format: `"ACK|HMAC"`
- Processes incoming sensor data

## Troubleshooting

### Common Issues

**WiFi Connection Failed**
- Verify SSID and password
- Check WiFi signal strength
- Ensure 2.4GHz network (ESP8266 doesn't support 5GHz)

**Sensor Readings Failed**
- Check DHT11 connections
- Verify power supply (3.3V)
- Replace sensor if readings are consistently invalid

**Registration Not Acknowledged**
- Verify central station is running
- Check UDP port configuration
- Ensure secret keys match between devices

### Debug Output

The device provides Serial output at 115200 baud:
- WiFi connection status
- Registration attempts
- Data transmission confirmations
- Error messages

## Security Considerations

- **Change default secret key** before deployment
- **Use unique device IDs** for each sensor
- **Consider network isolation** for sensitive deployments
- **Regular key rotation** for production environments

## License

This project is open source. Please ensure compliance with any library licenses used.

## Contributing

Feel free to submit issues, feature requests, or pull requests to improve this sensor station.

---

**Note**: This sensor station is designed for educational and prototyping purposes. For production deployments, consider additional security measures and error handling.
