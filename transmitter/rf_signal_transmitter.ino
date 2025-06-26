/**
 * RF Signal Hunt Transmitter - IEEE APS CUSAT Educational Event
 * 
 * This code implements a 433MHz RF transmitter using Arduino Uno,
 * designed to serve as a "beacon" for students to find during the
 * Signal Hunt competition.
 * 
 * Hardware Requirements:
 * - Arduino Uno board
 * - 433MHz RF transmitter module
 * - Battery for portable operation
 * 
 * Pin Connections:
 * - 433MHz Data Pin → Digital Pin 10
 * - Optional: LED → Digital Pin 13 (built-in)
 * 
 * Author: IEEE APS CUSAT Student Branch
 * Date: 2025-06-26
 */

#include <RCSwitch.h>

//============== TRANSMITTER SETTINGS ==============//
// Each transmitter should have a unique ID from this list:
// Alpha: 1001, Beta: 1002, Gamma: 1003, Delta: 1004, Epsilon: 1005, Omega: 1006

// *** CHANGE THIS TO THE DESIRED TRANSMITTER ID ***
#define TRANSMITTER_ID 1001   // Set to your transmitter's ID (1001-1006)

// *** CHANGE THIS TO MATCH YOUR TRANSMITTER NAME ***
#define TRANSMITTER_NAME "Alpha"   // Set to your transmitter's name

//============== RF CONFIGURATION ==============//
#define RF_TRANSMIT_PIN 10       // Pin connected to 433MHz transmitter data pin
#define STATUS_LED_PIN 13        // LED to show transmission status (built-in LED)

// Advanced RF settings
#define RF_PULSE_LENGTH 350      // Pulse length (microseconds) - affects range
#define RF_PROTOCOL 1            // RC protocol (1-6, match with receiver)
#define RF_TRANSMIT_REPEATS 10   // Repeat transmissions for better reliability
#define TRANSMIT_INTERVAL 500    // Interval between transmissions (milliseconds)

// Battery saving options
#define BATTERY_SAVING_MODE false // Set to true to enable battery saving (reduces range)
#define SLEEP_INTERVAL 5000      // Sleep interval in battery saving mode (milliseconds)

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600);
  Serial.println("\n\n======= RF SIGNAL HUNT TRANSMITTER =======");
  Serial.println("IEEE APS CUSAT - 2025");
  Serial.print("Transmitter ID: ");
  Serial.print(TRANSMITTER_ID);
  Serial.print(" (");
  Serial.print(TRANSMITTER_NAME);
  Serial.println(")");
  
  // Configure RF transmitter with optimized settings
  mySwitch.enableTransmit(RF_TRANSMIT_PIN);
  mySwitch.setProtocol(RF_PROTOCOL);
  mySwitch.setPulseLength(RF_PULSE_LENGTH);
  mySwitch.setRepeatTransmit(RF_TRANSMIT_REPEATS);
  
  // Initialize status LED
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  // Indicate startup with LED flash pattern
  for (int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(100);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(100);
  }
  
  // Battery saving mode notification
  if (BATTERY_SAVING_MODE) {
    Serial.println("Battery saving mode ENABLED - Range will be reduced");
  }
  
  Serial.println("Transmitter activated!");
}

void loop() {
  // Turn on LED during transmission
  digitalWrite(STATUS_LED_PIN, HIGH);
  
  // Transmit the identifier code
  mySwitch.send(TRANSMITTER_ID, 24);  // 24-bit transmission
  
  Serial.println("Signal transmitted: " + String(TRANSMITTER_ID));
  
  // Brief delay with LED on
  delay(50);
  
  // Turn off LED
  digitalWrite(STATUS_LED_PIN, LOW);
  
  // Wait between transmissions
  delay(TRANSMIT_INTERVAL);
  
  // Additional sleep for battery saving if enabled
  if (BATTERY_SAVING_MODE) {
    delay
