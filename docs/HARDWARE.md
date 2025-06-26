# Hardware Assembly Guide

This guide provides step-by-step instructions for assembling both the receiver and transmitter hardware for the RF Signal Hunt project.

## Receiver Assembly (ESP32)

### Components
- ESP32 DevKit board
- 433MHz RF receiver module
- Mini USB cable
- Power bank (5V output)
- Jumper wires
- Optional: Small enclosure
- Optional: Battery voltage divider (2x 100kΩ resistors)

### Connections
1. **Power Connection**
   - Connect ESP32 to power bank using USB cable

2. **RF Receiver Connection**
   - VCC → 3.3V on ESP32
   - GND → GND on ESP32 
   - DATA → GPIO2 on ESP32

3. **Optional: Battery Monitoring**
   - Create voltage divider with two 100kΩ resistors
   - Connect divider between power source and GND
   - Connect middle point to GPIO34 (ADC pin)

### Diagram
