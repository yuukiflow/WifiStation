# Wemos IoT Station System

A complete IoT system consisting of a **Central Station** and **Sensor Nodes** that communicate securely over WiFi using HMAC-SHA256 authentication.

## System Overview

This project implements a secure IoT network where multiple sensor nodes (Wemos D1 Mini + DHT11) communicate with a central hub (Wemos D1 Mini) to collect and monitor environmental data.

```
┌─────────────────┐    WiFi + UDP    ┌─────────────────┐
│   Central       │ ◄──────────────► │   Sensor Node   │
│   Station       │   Port 1642      │   (DHT11)       │
│                 │                  │                 │
│ • Device Mgmt   │                  │ • Temp/Humidity │
│ • Data Logger   │                  │ • Auto-register │
│ • HMAC Verify   │                  │ • Secure TX     │
└─────────────────┘                  └─────────────────┘
```

## How They Work Together

### 1. **Device Discovery & Registration**
- Sensor nodes broadcast registration messages with unique device IDs
- Central station verifies HMAC signatures using shared secret key
- Station acknowledges successful registration and stores device info
- Up to 10 sensor nodes can be registered simultaneously

### 2. **Secure Data Transmission**
- Registered sensors send temperature/humidity data every 1.5 seconds
- All messages are signed with HMAC-SHA256 for authentication
- Central station verifies each message before processing
- No acknowledgment sent for data (fire-and-forget protocol)

### 3. **Network Architecture**
- **Protocol**: UDP over WiFi for lightweight communication
- **Port**: 1642 (configurable)
- **Authentication**: Shared secret key ("cMsK" by default)
- **Message Format**: `MESSAGE;HMAC_SIGNATURE`

## Quick Start

### Prerequisites
- Arduino IDE with ESP8266 board support
- WiFi network (2.4GHz)
- Wemos D1 Mini boards
- DHT11 sensor (for sensor nodes)

### Setup Steps

1. **Configure Central Station**
   ```cpp
   // In wemosCentralStation.ino
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   const char* secretKey = "cMsK";  // Change this!
   ```

2. **Configure Sensor Nodes**
   ```cpp
   // In wemosSensor.ino
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   const char* secretKey = "cMsK";  // Must match central station
   const char* deviceID = "Temp1";  // Unique per sensor
   ```

3. **Upload Code**
   - Upload `wemosCentralStation.ino` to central station
   - Upload `wemosSensor.ino` to each sensor node

4. **Monitor Output**
   - Central station: Serial monitor at 115200 baud
   - Sensor nodes: Serial monitor at 115200 baud

## Communication Flow

```
Sensor Node                    Central Station
     │                              │
     ├── REGISTER;Temp1;HMAC ──────►│
     │                              ├── Verify HMAC
     │                              ├── Store device info
     │◄── ACK|HMAC ─────────────────┤
     │                              │
     ├── DATA;Temp1|23.5|45.2;HMAC─►│
     │                              ├── Verify HMAC
     │                              ├── Process data
     │                              └── Log to serial
     │                              │
     └── (continues every 1.5s) ───►│
```

## Security Features

- **HMAC-SHA256** message authentication
- **Shared secret key** verification
- **Device registry** with IP/port tracking
- **Message integrity** protection

## Hardware Requirements

### Central Station
- Wemos D1 Mini (ESP8266)
- USB power/connection

### Sensor Nodes
- Wemos D1 Mini (ESP8266)
- DHT11 temperature/humidity sensor
- Breadboard and jumper wires

## Pin Connections (Sensor Nodes)

| Wemos Pin | DHT11 Pin | Purpose |
|-----------|-----------|---------|
| D3 (GPIO0) | Data      | Sensor data |
| D4 (GPIO2) | -         | Status LED |
| 3.3V      | VCC       | Power |
| GND       | GND       | Ground |

## Configuration Options

- **UDP Port**: Change `port` constant in both sketches
- **Secret Key**: Modify `secretKey` for security
- **Device IDs**: Set unique `deviceID` for each sensor
- **Data Interval**: Adjust delay in sensor loop (currently 1.5s)

## Troubleshooting

- **Registration fails**: Check WiFi credentials and secret key
- **No data received**: Verify central station is running and port is correct
- **HMAC errors**: Ensure secret keys match exactly
- **WiFi issues**: Check network compatibility (2.4GHz only)

## Extending the System

- Add more sensor types (motion, light, etc.)
- Implement data persistence on central station
- Add web interface for monitoring
- Enable sensor-to-sensor communication
- Add encryption for sensitive data

## Adding New Sensor Types

**Important**: Each sensor type requires individual coding and cannot use the existing `wemosSensor.ino` directly.

### Why Individual Coding is Required:
- **Different Hardware**: Each sensor type has unique pin connections and communication protocols
- **Data Format**: Different sensors produce different data structures and units
- **Library Dependencies**: Each sensor requires specific Arduino libraries
- **Calibration**: Sensors may need different initialization and calibration procedures

### Example Sensor Types and Requirements:
- **DHT11/DHT22**: Temperature & humidity (current implementation)
- **DS18B20**: Digital temperature sensor (1-wire protocol)
- **BMP280**: Pressure & temperature (I2C communication)
- **PIR Motion**: Motion detection (digital input)
- **LDR**: Light dependent resistor (analog input)
- **MQ-2**: Gas sensor (analog input with calibration)

### Development Process for New Sensors:
1. Create new Arduino sketch based on `wemosSensor.ino`
2. Replace DHT11 library with appropriate sensor library
3. Modify pin definitions and sensor initialization
4. Update data reading and formatting functions
5. Test communication with central station
6. Ensure HMAC authentication remains intact

## Project Structure

```
WemosStation/
├── wemosCentralStation/     # Central hub for device management
│   ├── wemosCentralStation.ino
│   └── README.md
├── wemosSensor/             # Sensor node with DHT11
│   ├── wemosSensor.ino
│   └── README.md
└── README.md                # This file - system overview
```

For detailed setup instructions and technical specifications, see the individual README files in each project directory.
