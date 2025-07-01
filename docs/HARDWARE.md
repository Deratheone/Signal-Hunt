# Hardware Assembly Guide

This guide provides step-by-step instructions for assembling both the receiver and transmitter hardware for the Signal Hunt project.

## Receiver Assembly (ESP32)

### Components
- ESP32 DevKit module
- lion cell and holder
- Boost converter like MT3608  (to convert 3.7v from cell to the required 5v for the esp32)
- Optional: Small enclosure


### Connections
1. **Power Connection**
   - Connect ESP32 to the lion cell through the boost converter which is set to 5v output

## Transmitter Assembly (Also ESP32)

### Components
- ESP32 DevKit board
- 3.7V Lion cell or power bank
- boost converter if using lion cell
- USB cable or direct battery connection
- Optional: Small enclosure

### Connections
1. **Power Connection**
   - Connect the esp32 to the lion cell through a boost converter set to 5v output

## Antenna Optimization

ESP32 has built-in antennas, but for optimal range:

### Range Optimization
- Keep ESP32 away from large metal objects
- Position devices with PCB antenna facing toward each other
- Elevate transmitters when possible
- Avoid placing transmitters near walls containing electrical wiring
- For maximum range, consider ESP32 modules with external antenna connectors

## Enclosure Considerations
- Use non-metallic enclosures to prevent signal blockage
- Ensure WiFi antenna area is not blocked by case
- Include ventilation if running for extended periods
- Make battery accessible for replacement
- Add a small window if using LED indicators

## Troubleshooting Common Hardware Issues

| Issue | Possible Cause | Solution |
|-------|---------------|----------|
| Limited range | Signal blockage | Reposition ESP32, avoid metal objects |
| No signal detection | Power issues | Check battery level and connections |
| Intermittent reception | Interference | Change ESP-NOW channel in code |
| ESP32 not booting | Insufficient power | Use higher capacity power source |

## Testing Your Assembly

1. **Transmitter Test**:
   - Power on the ESP32 transmitter
   - Built-in LED should blink periodically, indicating transmission
   - Serial monitor should show transmission confirmation

2. **Receiver Test**:
   - Power on the ESP32 receiver
   - Connect to WiFi AP "SIGNAL-HUNT"
   - Navigate to http://192.168.4.1
   - Verify signal detection from transmitters

## Power Consumption Considerations

For longer battery life:
- Configure deep sleep mode between transmissions for transmitters
- Reduce transmission frequency
- Use lower power ESP32 variants (ESP32-S2, ESP32-C3)
- Consider larger capacity batteries for all-day events

## Additional Resources

For more details on ESP-NOW optimization and wireless communications, see the IEEE APS resources on antenna theory and practical applications.

Last Updated: 2025-06-01
