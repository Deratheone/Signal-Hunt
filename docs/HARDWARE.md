# Workshop Guide

This document outlines how to organize and run the Signal Hunt workshop and competition.

## Workshop Overview

The Signal Hunt event is designed as a two-phase educational activity:

1. **Workshop Phase** (2-3 hours): Participants build the receivers and if time remains also an example transmitter
2. **Hunt Phase** (1-2 hours): Participants use their receivers to find hidden transmitters

## Learning Objectives

By completing this workshop, participants will learn:

- Fundamentals of wireless communication protocols
- ESP-NOW technology and advantages
- Signal strength measurement techniques
- RSSI-based distance estimation
- ESP32 programming and capabilities
- Web interface development for IoT devices
- Wireless signal propagation principles

## Required Materials

### Per Participant/Team:
- ESP32 DevKit module
- lion cell 
- Boost converter like MT3608  (to convert 3.7v from cell to the required 5v for the esp32)
- Optional: Small enclosure
- Computer with Arduino IDE installed (one per team)

### For Instructors:
- Pre-programmed ESP32 transmitters (6 recommended)
- Spare ESP32 modules
- USB cables (ensure it has data lines to program the board)
- Fully charged lion cells
- Sample completed receiver for demonstration

## Workshop Schedule

### Introduction (30 minutes)
- Welcome and IEEE APS introduction
- Theory of wireless communication and ESP-NOW protocol
- Overview of the project and competition rules

### Hardware Setup (30 minutes)
- Distribute ESP32 modules to participants
- Connect to power sources
- Basic ESP32 orientation

### Software Setup (60 minutes)
- ESP32 board setup in Arduino IDE
- Required libraries installation
- ESP-NOW protocol explanation
- Code walkthrough and explanation
- Uploading code to ESP32

### Testing & Calibration (30 minutes)
- WiFi access point verification
- Web interface testing
- Signal detection validation
- Range testing with sample transmitter
- RSSI to distance calibration

### Competition Briefing (15 minutes)
- Explanation of scoring system
- Transmitter point values
- Competition rules and time limit
- Safety considerations

### Signal Hunt Competition (60-120 minutes)
- Deploy hidden transmitters around venue
- Release participants to find transmitters
- prvide hints if necessary
- Track scores via web interface
- request for the json files with all the details at the end from each team
- Time limit enforcement

### Award Ceremony (15 minutes)
- Announce winners
- Distribute prizes
- Explain solutions and optimal routes
- Collect feedback for future events

## Workshop Presentation Topics

1. **ESP-NOW Protocol**
   - Connectionless communication advantage
   - Comparison with WiFi and Bluetooth
   - Security and reliability features
   - Power efficiency

2. **Signal Propagation**
   - Electromagnetic wave behavior
   - Obstacles and interference
   - Multipath effects
   - Environmental factors

3. **RSSI Measurements**
   - Received Signal Strength Indicator
   - Converting RSSI to distance
   - Statistical reliability improvements
   - Smoothing and filtering techniques

4. **ESP32 Capabilities**
   - Dual-core architecture
   - WiFi and Bluetooth functionalities
   - Power management features
   - Programming considerations

## Competition Rules

1. Each team uses their built receiver to locate hidden transmitters
2. Points awarded based on transmitter discovery (verified by web interface)
3. Time limit: 60-120 minutes (adjust based on venue size)
4. Leaderboard updates automatically via web interface
5. Winning team is one with highest point total at the end

## Instructor Tips

- Deploy a mix of easy and challenging transmitters
- Place higher-value transmitters in more difficult locations
- Have a sample receiver to demonstrate expected behavior
- Provide hints for teams struggling after a set time
- Consider environmental factors affecting signal propagation
- Use tape or marking to indicate boundaries of the hunt area
- Have technical support available during the hunt

## Safety Considerations

- Establish clear boundaries for the hunt area
- Provide emergency contact information
- Use buddy system if in larger or more complex venues
- Ensure all areas are accessible and safe
- Have first aid kit available
- Communicate time limit and return procedure clearly

## ESP-NOW Technical Advantages for Discussion

1. **Power Efficiency**
   - Sleep mode between transmissions
   - Fast wake-up and transmission times
   - No connection overhead

2. **Simplicity**
   - No WiFi router required
   - Direct device-to-device communication
   - Simple API implementation

3. **Reliability**
   - Acknowledgment mechanism
   - Multiple retry attempts
   - Consistent performance in crowded RF environments

4. **Range**
   - Up to 200m line-of-sight
   - 30-50m indoor typical range
   - Better penetration than Bluetooth

## Post-Event Activities

- Collect participant feedback
- Document lessons learned
- Recover all transmitters
- Analyze most popular/difficult transmitter locations
- Share photos and results on IEEE social media
- Plan improvements for future events

## Advanced Options

### Extended Projects
- Add digital displays to receivers
- Implement team-based scoring system
- Create mobile app companion for hunt
- Design custom PCBs for more compact receivers
- Add encryption to ESP-NOW communications

### Variations for Different Skill Levels
**Beginners:**
- Pre-assembled receivers with focus on hunt experience
- More frequent signal transmission intervals
- Limited hunt area with clearly visible landmarks

**Advanced:**
- Add triangulation algorithms for precise location
- Implement custom web interface designs
- Create mesh network of transmitters
- Add environmental sensors to transmitters

### Integration with Other IEEE Topics
- Combine with power management workshop (battery efficiency)
- Add encryption components for cybersecurity education
- Include signal processing elements for DSP education
- Connect to AWS IoT for cloud integration learning

---

Last Updated: 2025-06-01 
Prepared by: Deratheone
