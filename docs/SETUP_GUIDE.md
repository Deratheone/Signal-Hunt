# Setup Guide

This guide will help you set up and configure the RF Signal Hunt system for your event.

## Software Setup

### Development Environment

1. **Install Arduino IDE**
   - Download from [arduino.cc](https://www.arduino.cc/en/software)
   - Install version 1.8.19 or newer

2. **Install ESP32 Board Support**
   - Open Arduino IDE
   - Go to File → Preferences
   - Add `https://dl.espressif.com/dl/package_esp32_index.json` to Additional Boards Manager URLs
   - Go to Tools → Board → Boards Manager
   - Search for "ESP32" and install the latest version

3. **Install Required Libraries**
   - Go to Tools → Manage Libraries
   - Search and install:
     - `RCSwitch` (by sui77) - for 433MHz communication
     - `ArduinoJson` (by Benoit Blanchon) - for JSON handling

### Configuration

#### Transmitter Configuration

1. Open `transmitter/rf_signal_transmitter.ino`
2. Set unique ID for each transmitter:
   ```cpp
   // Change for each transmitter
   #define TRANSMITTER_ID 1001  // 1001-1006 for Alpha-Omega
   #define TRANSMITTER_NAME "Alpha"
   ```
3.Optional: Adjust RF settings for range vs battery life:
  ```cpp
   #define RF_PULSE_LENGTH 350  // Higher = better range but more power 
   #define BATTERY_SAVING_MODE false  // Set true for longer battery life
 ```
4.Upload to Arduino Uno

####Receiver Configuration

1. open `receiver/rf_signal_hunt_receiver.ino`
2. Modify WiFi settings if needed:
 ```cpp
  const char* ssid = "RF-SIGNAL-HUNT";
  const char* password = "ieee2024";
  ```
3.Customize transmitter database for your event:
```cpp
const struct {
  unsigned long id;
  const char* name;
  int points;
} transmitterList[] = {
  {1001, "Alpha", 10},  // Modify IDs, names, points
  // ...
};
```
4. Upload to ESP32

## Event Setup

### Before the Event

1. **Prepare Transmitters**
   - Install fresh batteries
   - Verify each transmitter is working with correct ID
   - Place in waterproof containers if outdoor event

2. **Test Receivers**
   - Verify WiFi access point creation
   - Test signal detection from all transmitters
   - Ensure scoring system works properly

3. **Map Placement Plan**
   - Create a map with planned transmitter locations
   - Ensure good distribution across campus
   - Consider difficulty levels and terrain

### Transmitter Placement

1. **Strategic Hiding**
   - Hide transmitters at varying difficulty levels
   - Ensure antenna remains unobstructed
   - Place higher-value transmitters in more challenging locations

2. **Documentation**
   - Record exact placement for future reference
   - Consider taking photos of hidden locations
   - Test signal detection range before finalizing

### Workshop Setup

1. **Workstations**
   - Prepare workstation for each participant/team
   - Provide components, breadboards, and wiring diagrams
   - Have spare components available

2. **WiFi Considerations**
   - Ensure unique SSID for each receiver if multiple teams
   - Example: `RF-SIGNAL-HUNT-TEAM1`, `RF-SIGNAL-HUNT-TEAM2`

3. **Pre-Event Testing**
   - Do a complete run-through with all transmitters
   - Verify receivers can detect all transmitters at expected ranges
   - Check battery life expectations for both transmitters and receivers

## Troubleshooting

### Common Issues

| Problem | Solution |
|---------|----------|
| ESP32 won't connect | Check power supply and USB cable |
| Transmitter not detected | Verify transmitter ID matches in database |
| Limited WiFi range | Use higher gain antenna for access point |
| Weak detection range | Check antenna lengths and orientation |
| Game progress not saved | Verify EEPROM functions properly |

### Support

For technical support or questions, please file an issue on the GitHub repository.

---

Last Updated: 2025-06-26 19:36:02  
Prepared by: Deratheone
