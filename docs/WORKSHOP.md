# Workshop Guide

This document outlines how to organize and run the RF Signal Hunt workshop and competition.

## Workshop Overview

The RF Signal Hunt event is designed as a two-phase educational activity:

1. **Workshop Phase** (2-3 hours): Participants build the RF receivers
2. **Hunt Phase** (1-2 hours): Participants use their receivers to find hidden transmitters

## Learning Objectives

By completing this workshop, participants will learn:

- Fundamentals of RF communication
- Signal propagation principles
- Hardware interfacing with ESP32
- Signal strength measurement techniques
- Practical antenna design
- Web interface development for IoT devices

## Required Materials

### Per Participant/Team:
- ESP32 DevKit module
- 433MHz RF receiver module
- Jumper wires
- Mini breadboard
- USB power bank
- Copper wire for antenna (17.3cm length)
- Computer with Arduino IDE installed

### For Instructors:
- Pre-programmed transmitters (6 recommended)
- Spare components
- USB cables
- Multimeter
- Wire cutters/strippers
- Sample completed receiver for demonstration

## Workshop Schedule

### Introduction (30 minutes)
- Welcome and IEEE APS introduction
- Theory of RF propagation and 433MHz communication
- Overview of the project and competition rules

### Hardware Assembly (60 minutes)
- Distribute components to participants
- Guide through hardware connections
- Antenna construction and optimization
- Power supply connections

### Software Setup (30 minutes)
- ESP32 board setup in Arduino IDE
- Required libraries installation
- Code walkthrough and explanation
- Uploading code to ESP32

### Testing & Calibration (30 minutes)
- WiFi access point verification
- Web interface testing
- Signal detection validation
- Range testing with sample transmitter

### Competition Briefing (15 minutes)
- Explanation of scoring system
- Transmitter point values
- Competition rules and time limit
- Safety considerations

### Signal Hunt Competition (60-120 minutes)
- Deploy hidden transmitters around venue
- Release participants to find transmitters
- Track scores via manual recording or automated system
- Time limit enforcement

### Award Ceremony (15 minutes)
- Announce winners
- Distribute prizes
- Explain solutions and optimal routes
- Collect feedback for future events

## Workshop Presentation Topics

1. **RF Fundamentals**
   - Electromagnetic spectrum
   - Wavelength and frequency relationship
   - Signal propagation models

2. **Antenna Theory**
   - Types of antennas
   - Antenna gain and directivity
   - Wavelength calculation
   - Polarization effects

3. **Signal Measurement**
   - RSSI (Received Signal Strength Indicator)
   - Path loss models
   - Distance estimation techniques
   - Environmental factors affecting signal

4. **ESP32 Capabilities**
   - WiFi access point mode
   - Web server implementation
   - Interrupt handling for RF
   - Battery management

## Competition Rules

1. Each team uses their built receiver to locate hidden transmitters
2. Points awarded based on transmitter discovery (verified by interface)
3. Time limit: 60-120 minutes (adjust based on venue size)
4. Leaderboard updates via manual recording or automated system
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

### Variations for Different Skill Levels
**Beginners:**
- Pre-assembled receivers with focus on hunt experience
- More frequent signal transmission intervals
- Limited hunt area with clearly visible landmarks

**Advanced:**
- Custom antenna design challenge
- Signal decoding requirements (beyond simple transmission ID)
- Multiple simultaneous hunts with different frequency bands

### Integration with Other IEEE Topics
- Combine with power management workshop (battery efficiency)
- Add encryption components for cybersecurity education
- Include signal processing elements for DSP education
- Connect to AWS IoT for cloud integration learning

---

Last Updated: 2025-06-26 19:37:29  
Prepared by: Deratheone
