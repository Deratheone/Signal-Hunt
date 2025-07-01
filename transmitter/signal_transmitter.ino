/**
 * Signal Hunt Transmitter
 * IEEE APS CUSAT Educational Event
 * 
 * This code runs on an ESP32 acting as a beacon for the RF Signal Hunt game.
 * It uses ESP-NOW to broadcast periodic signals that receivers can track.
 * 
 * Hardware:
 * - ESP32 development board
 * - Battery for portable operation
 */

#include <esp_now.h>
#include <WiFi.h>

// ============= EDIT THESE VALUES FOR EACH TRANSMITTER =============
// Valid IDs: 1001=Alpha, 1002=Beta, 1003=Gamma, 1004=Delta, 1005=Epsilon, 1006=Omega
#define TRANSMITTER_ID 1001  // CHANGE THIS for each transmitter!
#define TRANSMITTER_NAME "Alpha"  // CHANGE THIS to match ID!

// Points awarded for finding this transmitter
#define TRANSMITTER_POINTS 10  // CHANGE THIS based on difficulty
// ==================================================================

// Broadcast address (sends to all ESP-NOW receivers)
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// How often to transmit (milliseconds)
#define TRANSMIT_INTERVAL 3000

// LED to indicate transmission (built-in LED on most ESP32 boards)
#define LED_PIN 2

// Data structure for transmissions
typedef struct {
  unsigned long id;
  char name[10];
  int points;
  unsigned long timestamp;
} SignalData;

// Create a structured object
SignalData myData;

// Last send time
unsigned long lastSendTime = 0;

// Status variables
bool ledState = false;

// ESP-NOW send callback - Updated for newer ESP-NOW API
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    // Successful transmission - toggle LED
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  } else {
    // Failed transmission - fast blink
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(50);
      digitalWrite(LED_PIN, LOW);
      delay(50);
    }
  }
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set LED pin as output
  pinMode(LED_PIN, OUTPUT);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register for Send Callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer (broadcast)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  // Prepare data structure
  myData.id = TRANSMITTER_ID;
  strncpy(myData.name, TRANSMITTER_NAME, 9);
  myData.name[9] = '\0'; // Ensure null termination
  myData.points = TRANSMITTER_POINTS;
  
  Serial.println("ESP-NOW Signal Hunt Transmitter");
  Serial.print("ID: ");
  Serial.print(TRANSMITTER_ID);
  Serial.print(" (");
  Serial.print(TRANSMITTER_NAME);
  Serial.println(")");
  
  // Startup indicator pattern
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  
  Serial.println("Transmitter initialized and broadcasting");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Send data at regular intervals
  if (currentTime - lastSendTime > TRANSMIT_INTERVAL) {
    // Update timestamp
    myData.timestamp = currentTime;
    
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    
    if (result == ESP_OK) {
      Serial.print("Transmission sent: ");
      Serial.print(myData.id);
      Serial.print(" (");
      Serial.print(myData.name);
      Serial.println(")");
    } else {
      Serial.println("Error sending transmission");
    }
    
    lastSendTime = currentTime;
  }
}
