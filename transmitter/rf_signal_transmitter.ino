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

// Battery monitoring disabled
// #define BATTERY_SAVING_MODE false  // true = longer gaps between transmissions
// #define BATTERY_PIN A0             // analog pin connected to battery via voltage divider

// Create RCSwitch instance
RCSwitch mySwitch = RCSwitch();

// Transmission functions
unsigned long lastTransmitTime = 0;
// Battery monitoring disabled, but we need this for serial printing
const int batteryLevel = 100;  // Fixed value since monitoring is disabled

void setup() {
  // Initialize serial monitor for debugging
  Serial.begin(9600);
  Serial.println("RF Signal Hunt Transmitter");
  Serial.println("ID: " + String(TRANSMITTER_ID) + " (" + TRANSMITTER_NAME + ")");
  
  // Set protocol to match receiver
  mySwitch.setProtocol(1);  // Must match RF_PROTOCOL in receiver
  
  // Initialize RF transmitter
  mySwitch.enableTransmit(RF_DATA_PIN);
  mySwitch.setPulseLength(RF_PULSE_LENGTH);
  
  // Visual indicator for debugging
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Unique startup pattern to identify this transmitter
  for (int i = 0; i < 3; i++) {
    transmitSignal();
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
  
  Serial.println("Transmitter initialized and ready");
}

void loop() {
  // Get current time
  unsigned long currentMillis = millis();
  
  // Check if it's time to transmit
  if (currentMillis - lastTransmitTime >= TRANSMIT_INTERVAL) {
    // Visual feedback
    digitalWrite(LED_BUILTIN, HIGH);
    
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
    
    // Turn off LED
    digitalWrite(LED_BUILTIN, LOW);
    
    // Update last transmit time
    lastTransmitTime = currentMillis;
    
    // Output status to serial
    Serial.println("TX: " + String(TRANSMITTER_ID) + " (" + TRANSMITTER_NAME + ")");
  }
  
  // Battery saving mode removed
}

// Transmit the signal multiple times for reliability
void transmitBurst() {
  for (int i = 0; i < TRANSMIT_REPEAT; i++) {
    transmitSignal();
    delay(50);
  }
}

// Transmit the signal with just the ID (no battery info)
void transmitSignal() {
  // Send only the ID (no battery level encoding)
  mySwitch.send(TRANSMITTER_ID, 24);  // Use 24 bits for the ID
}

// Battery-related functions removed
