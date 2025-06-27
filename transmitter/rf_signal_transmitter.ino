/**
 * RF Signal Hunt Transmitter
 * IEEE APS Educational Event - 2025
 * 
 * This code runs on Arduino Uno connected to a 433MHz RF transmitter module.
 * Each transmitter broadcasts its unique ID periodically, which can be detected
 * by receivers during the hunt.
 * 
 * Connection:
 * - 433MHz Transmitter VCC to Arduino 5V
 * - 433MHz Transmitter GND to Arduino GND
 * - 433MHz Transmitter DATA to Arduino pin 10
 * - 17.3cm straight copper wire to transmitter ANT pin
 */

#include <RCSwitch.h>

// ============= EDIT THESE VALUES FOR EACH TRANSMITTER =============
// Valid IDs: 1001=Alpha, 1002=Beta, 1003=Gamma, 1004=Delta, 1005=Epsilon, 1006=Omega
#define TRANSMITTER_ID 1001  // CHANGE THIS for each transmitter!
#define TRANSMITTER_NAME "Alpha"  // CHANGE THIS to match ID!

// Transmission pattern (0=normal, 1=beacon, 2=SOS pattern)
#define TRANSMISSION_PATTERN 0
// ==================================================================

// Timing and power settings
#define TRANSMIT_INTERVAL 5000  // milliseconds between transmissions
#define TRANSMIT_REPEAT 5       // how many times to repeat each transmission
#define RF_DATA_PIN 10          // pin connected to transmitter DATA pin
#define RF_PULSE_LENGTH 350     // microseconds (higher = better range, more power)

// Optional battery saving features
#define BATTERY_SAVING_MODE false  // true = longer gaps between transmissions
#define BATTERY_PIN A0             // analog pin connected to battery via voltage divider

// Create RCSwitch instance
RCSwitch mySwitch = RCSwitch();

// Transmission functions
unsigned long lastTransmitTime = 0;
int batteryLevel = 100;

void setup() {
  // Initialize serial monitor for debugging
  Serial.begin(9600);
  Serial.println("RF Signal Hunt Transmitter");
  Serial.println("ID: " + String(TRANSMITTER_ID) + " (" + TRANSMITTER_NAME + ")");
  mySwitch.setProtocol(1);  // Must match RF_PROTOCOL in receiver
  // Initialize RF transmitter
  mySwitch.enableTransmit(RF_DATA_PIN);
  mySwitch.setPulseLength(RF_PULSE_LENGTH);
  
  // Setup ADC for battery monitoring
  analogReference(DEFAULT);
  
  // Initial battery reading
  batteryLevel = readBattery();
  Serial.println("Battery: " + String(batteryLevel) + "%");
  
  // Unique startup pattern to identify this transmitter
  for (int i = 0; i < 3; i++) {
    transmitSignal();
    delay(200);
  }
}

void loop() {
  // Get current time
  unsigned long currentMillis = millis();
  
  // Check if it's time to transmit
  if (currentMillis - lastTransmitTime >= getTransmitInterval()) {
    // Update battery level
    batteryLevel = readBattery();
    
    // Transmit according to selected pattern
    switch(TRANSMISSION_PATTERN) {
      case 0:
        // Normal pattern - simple periodic transmission
        transmitBurst();
        break;
        
      case 1:
        // Beacon pattern - rapid series of pulses
        for (int i = 0; i < 3; i++) {
          transmitBurst();
          delay(200);
        }
        break;
        
      case 2:
        // SOS pattern - ... --- ...
        // 3 short
        for (int i = 0; i < 3; i++) {
          transmitSignal();
          delay(200);
        }
        delay(500);
        // 3 long
        for (int i = 0; i < 3; i++) {
          for (int j = 0; j < 3; j++) {
            transmitSignal();
            delay(50);
          }
          delay(200);
        }
        delay(500);
        // 3 short again
        for (int i = 0; i < 3; i++) {
          transmitSignal();
          delay(200);
        }
        break;
    }
    
    // Update last transmit time
    lastTransmitTime = currentMillis;
    
    // Output status to serial
    Serial.println("TX: " + String(TRANSMITTER_ID) + ", Batt: " + String(batteryLevel) + "%");
  }
  
  // Battery saving sleep mode if enabled
  if (BATTERY_SAVING_MODE) {
    delay(100); // Allow serial to finish
    // Simple delay for Arduino Uno (deep sleep would require additional hardware)
    delay(getTransmitInterval() - 100);
  }
}

// Transmit the signal multiple times for reliability
void transmitBurst() {
  for (int i = 0; i < TRANSMIT_REPEAT; i++) {
    transmitSignal();
    delay(50);
  }
}

// Transmit the signal with ID and battery info
void transmitSignal() {
  // Encode battery level in high bits
  unsigned long message = TRANSMITTER_ID;
  message |= ((unsigned long)batteryLevel << 24);
  
  // Send the RF signal
  mySwitch.send(message, 32);
}

// Read battery level
int readBattery() {
  // Simple battery monitoring through voltage divider
  // Formula assumes 5V USB or fresh 9V with voltage divider
  int rawValue = analogRead(BATTERY_PIN);
  
  // Map ADC to percentage (adjust these values based on your voltage divider)
  // For 9V battery: full=~9V(raw 614), empty=~7V(raw 477)
  int percentage = map(rawValue, 477, 614, 0, 100);
  percentage = constrain(percentage, 0, 100);
  
  return percentage;
}

// Get transmission interval, potentially adjusted for battery level
unsigned long getTransmitInterval() {
  // If battery is low, transmit less frequently
  if (batteryLevel < 20) {
    return TRANSMIT_INTERVAL * 2;
  }
  return TRANSMIT_INTERVAL;
}
