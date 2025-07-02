/**
 * ESP-NOW Signal Hunt Transmitter
 * IEEE APS CUSAT Educational Event
 * 
 * This code implements a wireless transmitter using ESP32 and ESP-NOW protocol.
 * The transmitter broadcasts its identity at regular intervals, allowing receivers
 * to detect and locate it. Each transmitter has a unique ID, name, and point value.
 * 
 * Hardware Requirements:
 * - ESP32 DevKit board
 * - Power via battery or USB
 * 
 * Author: IEEE APS CUSAT Student Branch
 * Date: 2025-07-02
 * Version: 1.0
 */

#include <esp_now.h>  // Include ESP-NOW library for direct device-to-device communication
#include <WiFi.h>     // WiFi library required for ESP-NOW operation

//============== TRANSMITTER CONFIGURATION ==============//
// !!! IMPORTANT: Change these values for each transmitter you deploy !!!
#define TRANSMITTER_ID 1003        // Unique identifier (1001-1006)
#define TRANSMITTER_NAME "Gamma"   // Human-readable name
#define TRANSMITTER_POINTS 20      // Points awarded when found

// Broadcast address (FF:FF:FF:FF:FF:FF sends to all ESP-NOW receivers)
// This is similar to a radio broadcast that any receiver can pick up
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Transmission timing
#define TRANSMIT_INTERVAL 3000     // Time between transmissions in milliseconds (3 seconds)

// LED configuration for visual feedback
#define LED_PIN 2                  // Built-in LED on most ESP32 boards

//============== DATA STRUCTURE ==============//
// This structure defines the data packet that will be sent via ESP-NOW
// Must match exactly with the receiver's expected format
typedef struct {
  unsigned long id;                // Unique transmitter identifier
  char name[10];                   // Human-readable name (9 chars + null terminator)
  int points;                      // Point value when discovered
  unsigned long timestamp;         // Current time when packet was sent
} SignalData;

// Create an instance of the data structure
SignalData myData;

// Track the last transmission time
unsigned long lastSendTime = 0;

/**
 * ESP-NOW Send Callback Function
 * 
 * This function is automatically called after each ESP-NOW transmission attempt
 * It provides visual feedback via the LED about transmission success/failure
 * 
 * @param mac_addr  MAC address of the recipient
 * @param status    Status of the transmission (success or failure)
 */
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    // Transmission successful - single short LED blink
    Serial.println("✓ Transmission sent successfully");
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
  } else {
    // Transmission failed - triple blink error pattern
    Serial.println("✗ Error sending transmission");
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(50);
      digitalWrite(LED_PIN, LOW);
      delay(50);
    }
  }
}

/**
 * Setup Function - Runs once at startup
 * 
 * Initializes all hardware and software components:
 * - Serial communication for debugging
 * - LED for visual status indication
 * - WiFi in station mode for ESP-NOW
 * - ESP-NOW protocol and peer registration
 * - Data structure initialization
 */
void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(115200);
  delay(1000); // Give serial time to start
  
  // Print startup banner and configuration
  Serial.println("\n\n=============================");
  Serial.println("ESP-NOW TRANSMITTER STARTING");
  Serial.println("=============================");
  Serial.print("Transmitter: ");
  Serial.print(TRANSMITTER_NAME);
  Serial.print(" (ID: ");
  Serial.print(TRANSMITTER_ID);
  Serial.print(", Points: ");
  Serial.print(TRANSMITTER_POINTS);
  Serial.println(")");
  
  // Configure LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Turn on LED during setup to indicate activity
  
  // Initialize WiFi in Station mode (required for ESP-NOW)
  // ESP-NOW works in WiFi station mode without connecting to an access point
  WiFi.mode(WIFI_STA);
  
  // Print MAC address for identification/debugging
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Initialize ESP-NOW protocol with error checking
  Serial.println("Initializing ESP-NOW...");
  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: ESP-NOW initialization failed!");
    // If ESP-NOW fails to initialize, enter error loop with rapid LED blinking
    while (1) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  }
  Serial.println("ESP-NOW initialized successfully");
  
  // Register callback function for when data is sent
  esp_now_register_send_cb(OnDataSent);
  
  // Register the peer device (in this case, broadcast to all receivers)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  // Use WiFi channel 0 (automatic)
  peerInfo.encrypt = false; // No encryption for broadcast
  
  // Add the peer with error checking
  Serial.println("Registering broadcast peer...");
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("ERROR: Failed to add broadcast peer");
    // If peer registration fails, enter error loop with slow LED blinking
    while (1) {
      digitalWrite(LED_PIN, HIGH);
      delay(300);
      digitalWrite(LED_PIN, LOW);
      delay(300);
    }
  }
  Serial.println("Broadcast peer added successfully");
  
  // Prepare data structure with transmitter information
  // This information will be sent with every transmission
  myData.id = TRANSMITTER_ID;
  strncpy(myData.name, TRANSMITTER_NAME, 9); // Copy name safely
  myData.name[9] = '\0'; // Ensure null termination
  myData.points = TRANSMITTER_POINTS;
  
  // Setup complete - turn off LED
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("Transmitter initialized and ready to broadcast");
  Serial.println("------------------------------");
}

/**
 * Main Loop Function - Runs continuously
 * 
 * This function:
 * - Checks if it's time to send a new transmission
 * - Updates the timestamp for each new transmission
 * - Sends the data packet via ESP-NOW broadcast
 */
void loop() {
  // Get current time in milliseconds
  unsigned long currentTime = millis();
  
  // Check if it's time for a new transmission
  // This creates a regular interval between transmissions
  if (currentTime - lastSendTime > TRANSMIT_INTERVAL) {
    // Update timestamp in the data packet to current time
    myData.timestamp = currentTime;
    
    Serial.print("Sending transmission... ");
    
    // Send the data packet via ESP-NOW
    // This broadcasts to all ESP-NOW receivers in range
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    
    // Check if send command was successful
    // Note: This doesn't guarantee delivery, just that the command was accepted
    if (result != ESP_OK) {
      Serial.println("ERROR: Send failed with code " + String(result));
    }
    
    // Update the last send time
    lastSendTime = currentTime;
  }
  
  // No delay needed here - the timing is handled by checking lastSendTime
  // This allows the CPU to handle other tasks between transmissions
}
