/**
 * ESP-NOW Signal Hunt Transmitter - Fixed Version
 * IEEE APS CUSAT Educational Event
 */

#include <esp_now.h>
#include <WiFi.h>

// Transmitter configuration
#define TRANSMITTER_ID 1003
#define TRANSMITTER_NAME "Gamma"
#define TRANSMITTER_POINTS 20

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

// ESP-NOW send callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("✓ Transmission sent successfully");
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
  } else {
    Serial.println("✗ Error sending transmission");
    // Blink rapidly to indicate error
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(50);
      digitalWrite(LED_PIN, LOW);
      delay(50);
    }
  }
}

void setup() {
  // Initialize Serial Monitor with higher baud rate
  Serial.begin(115200);
  delay(1000); // Give serial time to start
  
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
  
  // Set LED pin as output
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Turn on LED during setup
  
  // Initialize WiFi in Station mode
  WiFi.mode(WIFI_STA);
  
  // Print MAC address for debugging
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Initialize ESP-NOW with error checking
  Serial.println("Initializing ESP-NOW...");
  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: ESP-NOW initialization failed!");
    while (1) {
      // Blink LED rapidly to indicate fatal error
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  }
  Serial.println("ESP-NOW initialized successfully");
  
  // Register for Send Callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer (broadcast)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  // Use WiFi channel 0 (auto)
  peerInfo.encrypt = false;
  
  // Add peer with error checking
  Serial.println("Registering broadcast peer...");
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("ERROR: Failed to add broadcast peer");
    while (1) {
      // Blink LED in different pattern for this error
      digitalWrite(LED_PIN, HIGH);
      delay(300);
      digitalWrite(LED_PIN, LOW);
      delay(300);
    }
  }
  Serial.println("Broadcast peer added successfully");
  
  // Prepare data structure
  myData.id = TRANSMITTER_ID;
  strncpy(myData.name, TRANSMITTER_NAME, 9);
  myData.name[9] = '\0'; // Ensure null termination
  myData.points = TRANSMITTER_POINTS;
  
  // Turn off setup LED
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("Transmitter initialized and ready to broadcast");
  Serial.println("------------------------------");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Send data at regular intervals
  if (currentTime - lastSendTime > TRANSMIT_INTERVAL) {
    // Update timestamp
    myData.timestamp = currentTime;
    
    Serial.print("Sending transmission... ");
    
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    
    if (result != ESP_OK) {
      Serial.println("ERROR: Send failed with code " + String(result));
    }
    
    lastSendTime = currentTime;
  }
}
