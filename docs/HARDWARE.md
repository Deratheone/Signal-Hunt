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

## Transmitter Assembly (Arduino Uno)

### Components
- Arduino Uno board
- 433MHz RF transmitter module
- 9V battery
- 9V battery clip
- Jumper wires
- Optional: Small enclosure

### Connections
1. **Power Connection**
   - Connect 9V battery to Arduino power jack
   
2. **RF Transmitter Connection**
   - VCC → 5V on Arduino
   - GND → GND on Arduino
   - DATA → Digital Pin 10 on Arduino

## Antenna Optimization

For optimal range, we recommend:

### Receiver Antenna
- Use a 17.3cm straight wire for 433MHz (1/4 wavelength)
- Keep antenna vertical and unobstructed
- Use thicker gauge wire for better performance

### Transmitter Antenna
- Use a 17.3cm straight wire for 433MHz
- Mount vertically and as high as possible
- Keep away from metal objects

## Enclosure Considerations
- Use non-metallic enclosures to prevent signal blockage
- Ensure antenna extends outside the enclosure
- Include ventilation if running for extended periods
- Make battery accessible for replacement

## Troubleshooting Common Hardware Issues

| Issue | Possible Cause | Solution |
|-------|---------------|----------|
| Limited range | Improper antenna length | Ensure antenna is 17.3cm for 433MHz |
| No signal detection | Incorrect wiring | Double-check DATA pin connection |
| Intermittent reception | Power issues | Use stable power source, check connections |
| False readings | Interference | Move away from other RF sources |

## Testing Your Assembly

1. **Transmitter Test**:
   - Power on the Arduino
   - LED on pin 13 should blink, indicating transmission

2. **Receiver Test**:
   - Power on the ESP32
   - Connect to WiFi AP "RF-SIGNAL-HUNT"
   - Navigate to http://192.168.4.1
   - Verify signal detection from transmitter

## Additional Resources

For more details on RF optimization and antenna design, see the IEEE APS resources on antenna theory and practical applications.

Last Updated: 2025-06-26
