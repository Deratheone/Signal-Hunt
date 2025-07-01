# ESP-NOW Signal Hunt Setup Guide

This guide will help you set up and configure the ESP-NOW Signal Hunt system for your event.

## 📋 Table of Contents

- [Software Setup](#software-setup)
- [Configuration](#configuration)
- [ESP-NOW Configuration](#esp-now-configuration)
- [Event Setup](#event-setup)
- [Workshop Setup](#workshop-setup)
- [Troubleshooting](#troubleshooting)
- [Support](#support)

## 🛠️ Software Setup

### Development Environment

#### 1. Install Arduino IDE
- Download from [arduino.cc](https://www.arduino.cc/en/software)
- Install version 2.0.0 or newer (recommended)

#### 2. Install ESP32 Board Support
1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add the following URL to **Additional Boards Manager URLs**:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for "ESP32" and install the latest version

#### 3. Install Required Libraries
1. Go to **Tools → Manage Libraries**
2. Search and install:
   - `ArduinoJson` (by Benoit Blanchon) - for JSON handling

## ⚙️ Configuration

### Transmitter Configuration

1. Open `transmitter/espnow_signal_transmitter.ino`
2. Set unique ID for each transmitter:

```cpp
// Change for each transmitter
#define TRANSMITTER_ID 1001  // 1001-1006 for Alpha-Omega
#define TRANSMITTER_NAME "Alpha"
#define TRANSMITTER_POINTS 10  // Change based on difficulty
```

3. **Optional**: Adjust transmission settings for range vs battery life:

```cpp
#define TRANSMIT_INTERVAL 3000  // Milliseconds between transmissions
```

4. Upload to ESP32

### Receiver Configuration

1. Open `receiver/espnow_signal_hunt_receiver.ino`
2. Modify WiFi settings if needed:

```cpp
const char* ssid = "RF-SIGNAL-HUNT";
const char* password = "ieee2024";
```

3. Customize transmitter database for your event:

```cpp
const struct {
  unsigned long id;
  const char* name;
  int points;
} transmitterList[] = {
  {1001, "Alpha", 10},  // Modify IDs, names, points
  // Add more transmitters as needed
};
```

4. Upload to ESP32

## 📡 ESP-NOW Configuration

ESP-NOW is a connectionless communication protocol that enables direct device-to-device communication without establishing a traditional WiFi connection.

### Important ESP-NOW Settings

#### Broadcast Communication
- Transmitters are configured to broadcast to all devices
- No pairing required - any receiver can pick up signals

#### Adjusting Transmission Power
In transmitter code, you can adjust power level:

```cpp
// For lower power consumption but shorter range:
esp_wifi_set_max_tx_power(52); // 8dBm (lower)

// For maximum range:
esp_wifi_set_max_tx_power(84); // 20dBm (maximum)
```

#### Channel Selection
If experiencing interference, change the WiFi channel:

```cpp
peerInfo.channel = 1; // Try channels 1, 6, or 11 for least interference
```

## 🎯 Event Setup

### Before the Event

#### Prepare Transmitters
- [ ] Install fresh batteries
- [ ] Verify each transmitter is working with correct ID
- [ ] Place in waterproof containers if outdoor event

#### Test Receivers
- [ ] Verify WiFi access point creation
- [ ] Test signal detection from all transmitters
- [ ] Ensure scoring system works properly

#### Map Placement Plan
- [ ] Create a map with planned transmitter locations
- [ ] Ensure good distribution across campus
- [ ] Consider difficulty levels and terrain

### Transmitter Placement

#### Strategic Hiding
- Hide transmitters at varying difficulty levels
- Ensure antenna area remains unobstructed
- Place higher-value transmitters in more challenging locations

#### Range Optimization
- Position transmitters elevated from ground level
- Avoid placing near large metal objects
- Consider line-of-sight limitations

#### Documentation
- Record exact placement for future reference
- Consider taking photos of hidden locations
- Test signal detection range before finalizing
  

## 🔧 Troubleshooting

### Common Issues

| Problem | Solution |
|---------|----------|
| ESP32 won't connect | Check power supply |
| Transmitter not detected | Verify transmitter ID matches in database |
| Limited WiFi range | Reposition receiver for better AP coverage |
| Weak detection range | Ensure transmitters aren't blocked by obstacles |
| Game progress not saved | Verify EEPROM functions properly |
| ESP-NOW not working | Try different WiFi channel (1, 6, or 11) |

### Advanced Troubleshooting

- **Serial Monitoring**: Both transmitters and receivers output detailed debug information via Serial (115200 baud)
- **Flash Issues**: If unable to flash, hold BOOT button while connecting power
- **Range Testing**: Use serial output to monitor actual RSSI values for range calibration

##  Support

For technical support or questions, please file an issue on the GitHub repository.

---

**Last Updated**: 2025-06-01  
**Prepared by**: Deratheone

