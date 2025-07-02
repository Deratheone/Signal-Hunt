/**
 * ESP-NOW Signal Hunt Receiver
 * IEEE APS CUSAT Educational Event
 * 
 * This code implements a signal hunt receiver using ESP32 and ESP-NOW protocol.
 * The ESP32 creates a WiFi access point and hosts a web server with a radar-like
 * interface to help participants track hidden transmitters.
 * 
 * Hardware Requirements:
 * - ESP32 DevKit board
 * - Power via USB cable or power bank
 * 
 * Author: IEEE APS CUSAT Student Branch
 * Date: 2025-07-02
 * Version: 3.2 - Improved accuracy, fixed reset functionality, better radar
 */

#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

//============== WIFI CONFIGURATION ==============//
const char* ssid = "SIGNAL-HUNT";    // Changed to match repository name
const char* password = "ieee2024";

//============== GAME CONFIGURATION ==============//
#define DISCOVERY_RANGE 0.5    // Distance in meters to discover a transmitter (reduced from 5m)
#define MIN_SIGNAL_COUNT 5
#define DISCOVERY_RSSI_THRESHOLD -70
#define SIGNAL_STABILITY_PERIOD 5000
#define SIGNAL_TIMEOUT 5000    // Time in ms before signal is considered lost (reduced from 10s)
#define RESET_COOLDOWN 5000    // Cooldown after reset before new discoveries allowed

// Web Server on port 80
WebServer server(80);

// Structure for incoming ESP-NOW data
typedef struct {
  unsigned long id;
  char name[10];
  int points;
  unsigned long timestamp;
} SignalData;

// Game State Structure
struct TransmitterData {
  unsigned long id;          // Unique transmitter ID
  String name;               // Transmitter name (Alpha, Beta, etc.)
  int points;                // Points awarded for finding
  float distance;            // Estimated distance in meters
  int rssi;                  // Signal strength (dBm)
  unsigned long lastSeen;    // Timestamp when last detected
  bool isActive;             // Currently being detected
  float smoothedRSSI;        // Filtered RSSI for stable readings
  int signalCount;           // Number of recent signal detections
  float angle;               // Angle for radar display (0-359 degrees)
};

// Transmitter Database
const int MAX_TRANSMITTERS = 10;
TransmitterData transmitters[MAX_TRANSMITTERS];
int transmitterCount = 0;

// Game State
struct GameState {
  int totalScore;                        // Accumulated points
  bool foundTransmitters[MAX_TRANSMITTERS]; // Discovered transmitters
  unsigned long sessionStart;            // Game start time
  int foundCount;                        // Number of found transmitters
  float maxDistance;                     // Maximum detection range (auto-calibrated)
  unsigned long lastResetTime;           // Time of last reset
  bool resetMode;                        // Is system in reset cooldown mode?
} gameState;

// Signal processing variables
float rssiAlpha = 0.3;      // Exponential smoothing factor (0-1)
float distanceScaleFactor = 1.2; // Scaling factor for distance (increase to show longer distances)

// EEPROM addresses
#define EEPROM_SIZE 512
#define SCORE_ADDR 0
#define FOUND_ADDR 4
#define SETTINGS_ADDR 100

/**
 * ESP-NOW Data Received Callback
 * Called when ESP-NOW data is received from a transmitter
 */
void OnDataReceived(const esp_now_recv_info_t *info, const uint8_t *data, int data_len) {
  // Ignore signals during reset cooldown
  if (gameState.resetMode) {
    return;
  }
  
  // Get the sender's MAC address
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           info->src_addr[0], info->src_addr[1], info->src_addr[2], 
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);
  
  // Get RSSI from WiFi
  int rssi = WiFi.RSSI();
  
  // Print MAC and RSSI for debugging
  Serial.print("📡 Signal from: ");
  Serial.print(macStr);
  Serial.print(" [RSSI: ");
  Serial.print(rssi);
  Serial.print("dBm] ");

  // Process received data
  if (data_len == sizeof(SignalData)) {
    SignalData* receivedData = (SignalData*)data;
    
    Serial.print("ID: ");
    Serial.print(receivedData->id);
    Serial.print(" (");
    Serial.print(receivedData->name);
    Serial.println(")");
    
    // Find matching transmitter in our database
    int index = findTransmitterIndex(receivedData->id);
    
    if (index >= 0) {
      // Update transmitter with received data and RSSI
      updateTransmitterData(index, rssi, receivedData);
      checkForNewDiscovery(index);
    } else {
      Serial.println("Unknown transmitter ID: " + String(receivedData->id));
    }
  } else {
    Serial.println("Received data with unexpected size");
  }
}

/**
 * SETUP FUNCTION
 * Initializes all hardware and software components
 */
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n======= ESP-NOW SIGNAL HUNT RECEIVER STARTING =======");
  Serial.println("IEEE Antennas and Propagation Society - CUSAT");
  Serial.println("Version 3.2 - 2025-07-02");

  // Initialize EEPROM for persistent storage
  EEPROM.begin(EEPROM_SIZE);
  Serial.println("1. EEPROM initialized");
  
  // Load game state from EEPROM
  loadGameState();
  Serial.println("2. Game state loaded from EEPROM");

  // Initialize transmitter database
  initializeTransmitterDB();
  Serial.println("3. Transmitter database initialized");
  
  // Set up ESP-NOW first in STA mode
  WiFi.mode(WIFI_AP_STA);
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register callback for received data
  esp_now_register_recv_cb(OnDataReceived);
  
  Serial.println("4. ESP-NOW initialized and ready to receive");
  
  // Setup WiFi Access Point
  WiFi.softAP(ssid, password);
  Serial.println("5. WiFi Access Point started");
  Serial.println("   SSID: " + String(ssid));
  IPAddress ip = WiFi.softAPIP();
  Serial.println("   IP: " + ip.toString());
  
  // Setup web server routes
  setupWebServer();
  Serial.println("6. Web server initialized");
  
  // Initialize game state
  gameState.sessionStart = millis();
  gameState.maxDistance = 30.0;  // Initial max range estimate (will auto-calibrate)
  gameState.lastResetTime = 0;
  gameState.resetMode = false;
  
  Serial.println("\n======= SYSTEM READY =======");
  Serial.println("Connect to WiFi network: " + String(ssid));
  Serial.println("Then navigate to http://" + ip.toString());
}

/**
 * MAIN LOOP FUNCTION
 * Handles main program flow
 */
void loop() {
  // Handle web requests
  server.handleClient();
  
  // Update transmitter status (mark as inactive if not seen recently)
  updateTransmitterStatus();
  
  // Update reset mode status
  if (gameState.resetMode && (millis() - gameState.lastResetTime > RESET_COOLDOWN)) {
    gameState.resetMode = false;
    Serial.println("Reset cooldown period ended, normal operation resumed");
  }
  
  // Small delay to prevent CPU hogging
  delay(10);
}

/**
 * Initialize the transmitter database with predefined transmitters
 * for the treasure hunt game
 */
void initializeTransmitterDB() {
  // Define your transmitters here - customize based on your deployment
  const struct {
    unsigned long id;      // ID transmitted
    const char* name;      // Display name
    int points;            // Points value
  } transmitterList[] = {
    {1001, "Alpha", 10},     // Easiest to find
    {1002, "Beta", 15},
    {1003, "Gamma", 20},
    {1004, "Delta", 25},
    {1005, "Epsilon", 30},
    {1006, "Omega", 50}      // Most challenging/hidden
  };

  transmitterCount = sizeof(transmitterList) / sizeof(transmitterList[0]);
  
  for (int i = 0; i < transmitterCount; i++) {
    transmitters[i].id = transmitterList[i].id;
    transmitters[i].name = String(transmitterList[i].name);
    transmitters[i].points = transmitterList[i].points;
    transmitters[i].distance = 999.0;
    transmitters[i].rssi = -100;
    transmitters[i].lastSeen = 0;
    transmitters[i].isActive = false;
    transmitters[i].smoothedRSSI = -100;
    transmitters[i].signalCount = 0;
    transmitters[i].angle = i * (360.0 / transmitterCount); // Evenly distribute initial angles
  }
}

/**
 * Find the index of a transmitter in our database by its ID
 */
int findTransmitterIndex(unsigned long id) {
  for (int i = 0; i < transmitterCount; i++) {
    if (transmitters[i].id == id) {
      return i;
    }
  }
  return -1; // Not found
}
/**
 * Update transmitter data with new signal information
 * Uses exponential smoothing for stable readings
 */
void updateTransmitterData(int index, int rssi, SignalData* data) {
  // Store last seen time
  transmitters[index].lastSeen = millis();
  transmitters[index].isActive = true;
  
  // Increase signal count (used for reliability) - cap at 20 for stability
  transmitters[index].signalCount = min(20, transmitters[index].signalCount + 1);
  
  // Ensure name and points are updated from data
  transmitters[index].name = String(data->name);
  transmitters[index].points = data->points;
  
  // Set raw RSSI
  transmitters[index].rssi = rssi;
  
  // Apply exponential smoothing to RSSI for stability
  // Reduce alpha to 0.2 for more stability (was 0.3)
  const float rssiAlpha = 0.2;
  
  if (transmitters[index].smoothedRSSI == -100) {
    // First reading
    transmitters[index].smoothedRSSI = rssi;
  } else {
    transmitters[index].smoothedRSSI = 
      rssiAlpha * rssi + (1-rssiAlpha) * transmitters[index].smoothedRSSI;
  }
  
  // Convert RSSI to approximate distance using log-distance path loss model
  // Modified parameters for more realistic distance calculations
  float txPower = -65.0;  // RSSI at 1 meter (was -55.0, increasing makes distances larger)
  float pathLossExponent = 3.5;  // Path loss exponent (was 2.8, increasing makes distances grow faster)
  
  // Log-distance path loss model: d = 10^((Txpower - RSSI)/(10 * n))
  // Where n is path loss exponent
  float smoothedRssi = transmitters[index].smoothedRSSI;
  float rawDistance = pow(10, (txPower - smoothedRssi) / (10 * pathLossExponent));
  
  // Apply scaling factor - increase to 1.8 (was 1.2) to make distances appear larger
  transmitters[index].distance = rawDistance * 1.8;
  
  // Remove randomness for more stable readings
  // transmitters[index].distance += random(-10, 11) / 100.0;
  
  // Limit distance to reasonable range
  transmitters[index].distance = constrain(transmitters[index].distance, 0.5f, 50.0f);
  
  // Update angle with some smooth movement (drift)
  // Keep same angle calculation logic...
  float baseAngle = (index * 60) % 360; 
  float drift = sin(millis() / 10000.0) * 10; 
  float targetAngle = baseAngle + drift;
  float angleDiff = targetAngle - transmitters[index].angle;
  
  // Normalize angle difference to -180 to +180 degrees
  if (angleDiff > 180) angleDiff -= 360;
  if (angleDiff < -180) angleDiff += 360;
  
  // Move 5% toward target angle (smooth movement)
  transmitters[index].angle += angleDiff * 0.05;
  
  // Keep angle in 0-359 range
  if (transmitters[index].angle >= 360) transmitters[index].angle -= 360;
  if (transmitters[index].angle < 0) transmitters[index].angle += 360;
  
  // Auto-calibrate the maximum detection range
  if (transmitters[index].distance > gameState.maxDistance * 0.8 && 
      transmitters[index].distance < gameState.maxDistance) {
    // Gradually adjust max distance based on actual readings
    gameState.maxDistance = (gameState.maxDistance * 0.95) + (transmitters[index].distance * 1.2 * 0.05);
  }
  
  Serial.print("Transmitter ");
  Serial.print(transmitters[index].name);
  Serial.print(": Distance estimate: ");
  Serial.print(transmitters[index].distance);
  Serial.print("m (RSSI: ");
  Serial.print(transmitters[index].rssi);
  Serial.print("dBm, Smoothed: ");
  Serial.print(transmitters[index].smoothedRSSI);
  Serial.print("dBm, Angle: ");
  Serial.print(transmitters[index].angle);
  Serial.println("°)");
}
/**
 * Check if a transmitter is close enough to be discovered
 * Added time-based stability requirements
 */
void checkForNewDiscovery(int index) {
  // Skip if in reset cooldown mode
  if (gameState.resetMode || millis() - gameState.lastResetTime < RESET_COOLDOWN) {
    return;
  }
  
  static unsigned long discoveryTimers[MAX_TRANSMITTERS] = {0};
  static bool discoveryInProgress[MAX_TRANSMITTERS] = {false};
  
  // Only consider transmitters that have been consistently detected
  // Increased from 3 to MIN_SIGNAL_COUNT (10)
  if (transmitters[index].signalCount < MIN_SIGNAL_COUNT) {
    // Not enough consistent readings, reset discovery timer
    discoveryInProgress[index] = false;
    discoveryTimers[index] = 0;
    return;
  }
  
  // Additional RSSI validation - require stronger signal (-70 instead of -80)
  if (transmitters[index].smoothedRSSI < DISCOVERY_RSSI_THRESHOLD) {
    // Signal too weak, reset discovery timer
    discoveryInProgress[index] = false;
    discoveryTimers[index] = 0;
    return;
  }
  
  // Check if within discovery range and not already found
  if (transmitters[index].distance <= DISCOVERY_RANGE && !gameState.foundTransmitters[index]) {
    
    // If discovery just starting, record the time
    if (!discoveryInProgress[index]) {
      discoveryInProgress[index] = true;
      discoveryTimers[index] = millis();
      Serial.println("Discovery process started for " + transmitters[index].name);
      return;
    }
    
    // Check if discovery conditions have been met for at least SIGNAL_STABILITY_PERIOD (5 seconds)
    if (millis() - discoveryTimers[index] >= SIGNAL_STABILITY_PERIOD) {
      // Requirements met! Award points
      gameState.foundTransmitters[index] = true;
      gameState.totalScore += transmitters[index].points;
      gameState.foundCount++;
      
      Serial.println("===============================");
      Serial.println("🎉 DISCOVERY! Found " + transmitters[index].name + 
                     " - Awarded " + String(transmitters[index].points) + " points!");
      Serial.println("Total score: " + String(gameState.totalScore));
      Serial.println("===============================");
      
      saveGameState();
      
      // Reset discovery process
      discoveryInProgress[index] = false;
    }
    
  } else {
    // No longer in discovery range or conditions not met
    discoveryInProgress[index] = false;
    discoveryTimers[index] = 0;
  }
}
/**
 * Update status of all transmitters
 * Marks transmitters as inactive if not seen recently
 */
void updateTransmitterStatus() {
  unsigned long currentTime = millis();
  
  for (int i = 0; i < transmitterCount; i++) {
    // Mark as inactive if not seen for the timeout period
    if (transmitters[i].isActive && currentTime - transmitters[i].lastSeen > SIGNAL_TIMEOUT) {
      transmitters[i].isActive = false;
      transmitters[i].distance = 999.0;
      transmitters[i].signalCount = 0;
      Serial.println("Lost signal from " + transmitters[i].name);
    }
  }
}

/**
 * Load game state from EEPROM
 */
void loadGameState() {
  EEPROM.get(SCORE_ADDR, gameState.totalScore);
  for (int i = 0; i < MAX_TRANSMITTERS; i++) {
    EEPROM.get(FOUND_ADDR + i, gameState.foundTransmitters[i]);
    if (gameState.foundTransmitters[i]) {
      gameState.foundCount++;
    }
  }
  
  // Initialize if first run or invalid data detected
  if (gameState.totalScore < 0 || gameState.totalScore > 10000) {
    Serial.println("Initializing new game state (invalid data found)");
    gameState.totalScore = 0;
    gameState.foundCount = 0;
    for (int i = 0; i < MAX_TRANSMITTERS; i++) {
      gameState.foundTransmitters[i] = false;
    }
    saveGameState();
  }
}

/**
 * Save game state to EEPROM
 */
void saveGameState() {
  EEPROM.put(SCORE_ADDR, gameState.totalScore);
  for (int i = 0; i < MAX_TRANSMITTERS; i++) {
    EEPROM.put(FOUND_ADDR + i, gameState.foundTransmitters[i]);
  }
  
  if (EEPROM.commit()) {
    Serial.println("Game state saved to EEPROM successfully");
  } else {
    Serial.println("ERROR! EEPROM commit failed");
  }
}

/**
 * Configure web server routes
 */
void setupWebServer() {
  // Serve the main page
  server.on("/", handleRoot);
  server.on("/index.html", handleRoot);
  
  // API endpoints
  server.on("/api/transmitters", handleTransmittersAPI);
  server.on("/api/score", handleScoreAPI);
  server.on("/api/status", handleStatusAPI);
  server.on("/api/reset", HTTP_POST, handleResetAPI);
  server.on("/api/reset", HTTP_GET, handleResetAPI); // Also allow GET for easy browser access
  
  // Download endpoint
  server.on("/api/download", handleDownloadAPI);
  
  // Handle 404
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found. Use IP address to access the Signal Hunt game.");
  });

  server.begin();
  Serial.println("Web server started");
}

/**
 * API endpoint to download results as JSON file
 */
void handleDownloadAPI() {
  StaticJsonDocument<1024> doc;
  
  // Team info and basic stats
  doc["teamName"] = "IEEE APS Team";  // Could be made configurable
  doc["totalScore"] = gameState.totalScore;
  doc["foundCount"] = gameState.foundCount;
  doc["gameTime"] = millis() - gameState.sessionStart;
  
  // Found transmitters with details
  JsonArray found = doc.createNestedArray("foundTransmitters");
  for (int i = 0; i < transmitterCount; i++) {
    if (gameState.foundTransmitters[i]) {
      JsonObject tx = found.createNestedObject();
      tx["id"] = transmitters[i].id;
      tx["name"] = transmitters[i].name;
      tx["points"] = transmitters[i].points;
    }
  }
  
  // Active transmitters at time of download
  JsonArray active = doc.createNestedArray("activeTransmitters");
  for (int i = 0; i < transmitterCount; i++) {
    if (transmitters[i].isActive) {
      JsonObject tx = active.createNestedObject();
      tx["id"] = transmitters[i].id;
      tx["name"] = transmitters[i].name;
      tx["distance"] = transmitters[i].distance;
      tx["rssi"] = transmitters[i].rssi;
    }
  }
  
  // Device information for verification
  doc["deviceID"] = String((uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF), HEX);  // Use ESP32's unique MAC as ID
  doc["timestamp"] = millis();
  doc["version"] = "3.2";
  
  String response;
  serializeJsonPretty(doc, response);  // Pretty print for readability
  
  // Send with download headers
  server.sendHeader("Content-Disposition", "attachment; filename=signal_hunt_results.json");
  server.send(200, "application/json", response);
  
  Serial.println("Results downloaded");
}

/**
 * API endpoint to get transmitter information
 * Returns JSON with active transmitters and status
 */
void handleTransmittersAPI() {
  StaticJsonDocument<1024> doc;
  
  JsonArray transmitterArray = doc.createNestedArray("transmitters");
  int activeCount = 0;
  
  // Don't return any transmitters during reset cooldown
  if (!gameState.resetMode) {
    // Add active transmitters to JSON
    for (int i = 0; i < transmitterCount; i++) {
      if (transmitters[i].isActive) {
        JsonObject transmitter = transmitterArray.createNestedObject();
        transmitter["id"] = transmitters[i].id;
        transmitter["name"] = transmitters[i].name;
        transmitter["distance"] = transmitters[i].distance;
        transmitter["rssi"] = transmitters[i].rssi;
        transmitter["points"] = transmitters[i].points;
        transmitter["lastSeen"] = transmitters[i].lastSeen;
        transmitter["isActive"] = transmitters[i].isActive;
        transmitter["angle"] = transmitters[i].angle; // Send angle for radar positioning
        activeCount++;
      }
    }
  }
  
  doc["timestamp"] = millis();
  doc["activeCount"] = activeCount;
  doc["maxRange"] = gameState.maxDistance;
  doc["resetMode"] = gameState.resetMode;
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
}

/**
 * API endpoint to get score information
 * Returns JSON with game state
 */
void handleScoreAPI() {
  StaticJsonDocument<512> doc;
  
  doc["totalScore"] = gameState.totalScore;
  doc["foundCount"] = gameState.foundCount;
  doc["sessionTime"] = millis() - gameState.sessionStart;
  
  JsonArray foundArray = doc.createNestedArray("foundTransmitters");
  for (int i = 0; i < transmitterCount; i++) {
    if (gameState.foundTransmitters[i]) {
      foundArray.add(transmitters[i].id);
    }
  }
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
}

/**
 * API endpoint for system status
 * Returns system info
 */
void handleStatusAPI() {
  StaticJsonDocument<256> doc;
  
  doc["uptime"] = millis() / 1000;
  doc["rssi"] = WiFi.RSSI();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["activeSignals"] = 0;
  doc["resetMode"] = gameState.resetMode;
  
  // Count active signals
  for (int i = 0; i < transmitterCount; i++) {
    if (transmitters[i].isActive) {
      doc["activeSignals"] = doc["activeSignals"].as<int>() + 1;
    }
  }
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
}
/**
 * API endpoint to get discovery progress information 
 */
void handleDiscoveryProgressAPI() {
  StaticJsonDocument<512> doc;
  JsonArray progressArray = doc.createNestedArray("discoveryProgress");
  
  for (int i = 0; i < transmitterCount; i++) {
    if (transmitters[i].isActive && !gameState.foundTransmitters[i] && 
        transmitters[i].distance <= DISCOVERY_RANGE && 
        transmitters[i].smoothedRSSI >= DISCOVERY_RSSI_THRESHOLD &&
        transmitters[i].signalCount >= MIN_SIGNAL_COUNT) {
      
      JsonObject progress = progressArray.createNestedObject();
      progress["id"] = transmitters[i].id;
      progress["name"] = transmitters[i].name;
      progress["distance"] = transmitters[i].distance;
      progress["rssi"] = transmitters[i].smoothedRSSI;
      progress["signalCount"] = transmitters[i].signalCount;
    }
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}
/**
 * API endpoint to reset the game
 * POST or GET to reset game state
 */
void handleResetAPI() {
  // Reset game state
  gameState.totalScore = 0;
  gameState.foundCount = 0;
  gameState.sessionStart = millis();
  gameState.lastResetTime = millis();
  gameState.resetMode = true;
  
  // Clear all transmitter data completely
  for (int i = 0; i < transmitterCount; i++) {
    // Clear found status
    gameState.foundTransmitters[i] = false;
    
    // Reset all transmitter data completely
    transmitters[i].isActive = false;
    transmitters[i].distance = 999.0;
    transmitters[i].rssi = -100;
    transmitters[i].smoothedRSSI = -100;
    transmitters[i].lastSeen = 0;
    transmitters[i].signalCount = 0;
  }
  
  // Save reset state and force immediate commit
  saveGameState();
  
  StaticJsonDocument<128> doc;
  doc["status"] = "success";
  doc["message"] = "Game reset successfully";
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
  
  Serial.println("Game state reset - all data cleared");
  Serial.println("Reset cooldown period active for " + String(RESET_COOLDOWN/1000) + " seconds");
}

/**
 * Serve the main HTML page
 */
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Signal Hunt - IEEE APS</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        :root {
            --primary: #0ea5e9;
            --secondary: #10b981;
            --background: #0f172a;
            --card-bg: #1e293b;
            --highlight: #3b82f6;
            --danger: #ef4444;
            --warning: #f59e0b;
            --text: #f8fafc;
            --text-secondary: #94a3b8;
            --border: rgba(59, 130, 246, 0.2);
        }
        
        body {
            font-family: 'Inter', system-ui, -apple-system, sans-serif;
            background: linear-gradient(135deg, #0f172a 0%, #1e293b 100%);
            min-height: 100vh;
            color: var(--text);
            overflow-x: hidden;
        }
        
        .container {
            max-width: 480px;
            margin: 0 auto;
            min-height: 100vh;
            background: rgba(15, 23, 42, 0.95);
            border: 1px solid var(--border);
            backdrop-filter: blur(24px);
        }
        
        .header {
            background: linear-gradient(135deg, #1e3a8a 0%, #1e40af 100%);
            padding: 20px;
            color: white;
            position: relative;
            box-shadow: 0 4px 20px rgba(30, 58, 138, 0.4);
        }
        
        .header-content {
            display: flex;
            align-items: center;
            justify-content: space-between;
        }
        
        .logo {
            font-weight: 700;
            font-size: 18px;
        }
        
        .status-indicator {
            display: flex;
            align-items: center;
            gap: 6px;
            font-size: 12px;
        }
        
        .status-dot {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background: var(--secondary);
            box-shadow: 0 0 10px var(--secondary);
            animation: pulse 2s infinite;
        }
        
        .status-dot.offline {
            background: var(--danger);
            box-shadow: 0 0 10px var(--danger);
        }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; transform: scale(1); }
            50% { opacity: 0.7; transform: scale(1.1); }
        }
        
        .title {
            text-align: center;
            margin-top: 12px;
            font-size: 24px;
            font-weight: 800;
        }
        
        .subtitle {
            text-align: center;
            font-size: 13px;
            color: var(--text-secondary);
            margin-top: 4px;
        }
        
        .radar-container {
            position: relative;
            width: 280px;
            height: 280px;
            margin: 24px auto;
            background: radial-gradient(circle at center, 
                rgba(14, 165, 233, 0.15) 0%, 
                rgba(14, 165, 233, 0.05) 40%, 
                transparent 70%);
            border-radius: 50%;
            border: 2px solid rgba(14, 165, 233, 0.3);
            box-shadow: 0 0 30px rgba(14, 165, 233, 0.15);
        }
        
        .radar-center {
            position: absolute;
            width: 10px;
            height: 10px;
            background: var(--primary);
            border-radius: 50%;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            box-shadow: 0 0 15px var(--primary);
        }
        
        .radar-circles {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
        }
        
        .radar-circle {
            position: absolute;
            border: 1px dashed rgba(14, 165, 233, 0.3);
            border-radius: 50%;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
        }
        
        .radar-circle:nth-child(1) { width: 70px; height: 70px; }
        .radar-circle:nth-child(2) { width: 140px; height: 140px; }
        .radar-circle:nth-child(3) { width: 210px; height: 210px; }
        .radar-circle:nth-child(4) { width: 280px; height: 280px; }
        
        .radar-sweep {
            position: absolute;
            top: 50%;
            left: 50%;
            width: 50%;
            height: 2px;
            background: linear-gradient(90deg, rgba(14, 165, 233, 0) 0%, rgba(14, 165, 233, 0.8) 80%);
            transform-origin: left center;
            animation: sweep 4s linear infinite;
            z-index: 10;
        }
        
        @keyframes sweep {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        
        .radar-axis {
            position: absolute;
            top: 0;
            left: 50%;
            width: 1px;
            height: 100%;
            background: rgba(14, 165, 233, 0.2);
            transform: translateX(-50%);
        }
        
        .radar-axis:nth-child(2) {
            transform: translateX(-50%) rotate(45deg);
        }
        
        .radar-axis:nth-child(3) {
            transform: translateX(-50%) rotate(90deg);
        }
        
        .radar-axis:nth-child(4) {
            transform: translateX(-50%) rotate(135deg);
        }
        
        .blip {
            position: absolute;
            width: 12px;
            height: 12px;
            margin: -6px 0 0 -6px;
            background: var(--highlight);
            border-radius: 50%;
            box-shadow: 0 0 10px var(--highlight);
            animation: blink 2s infinite;
        }
        
        @keyframes blink {
            0%, 100% { transform: scale(1); opacity: 1; }
            50% { transform: scale(1.5); opacity: 0.7; }
        }
        
        .distance-labels {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            pointer-events: none;
        }
        
        .distance-label {
            position: absolute;
            font-size: 10px;
            color: var(--text-secondary);
            font-family: monospace;
        }
        
        .distance-label:nth-child(1) { top: 50%; left: calc(50% + 40px); transform: translate(-50%, -50%); }
        .distance-label:nth-child(2) { top: 50%; left: calc(50% + 75px); transform: translate(-50%, -50%); }
        .distance-label:nth-child(3) { top: 50%; left: calc(50% + 110px); transform: translate(-50%, -50%); }
        
        .current-target {
            background: var(--card-bg);
            border-radius: 16px;
            padding: 16px;
            margin: 0 20px 20px;
            border: 1px solid var(--border);
            display: flex;
        }
        
        .target-icon {
            width: 48px;
            height: 48px;
            background: linear-gradient(135deg, var(--primary), var(--highlight));
            border-radius: 12px;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: 700;
            flex-shrink: 0;
            margin-right: 16px;
        }
        
        .target-info {
            flex: 1;
        }
        
        .target-name {
            font-size: 18px;
            font-weight: 600;
            color: var(--text);
            margin-bottom: 4px;
        }
        
        .target-id {
            font-family: monospace;
            font-size: 12px;
            color: var(--text-secondary);
            margin-bottom: 6px;
        }
        
        .target-distance {
            font-family: monospace;
            font-size: 16px;
            font-weight: 600;
            color: var(--secondary);
            margin-top: 2px;
        }
        
        .signal-bars {
            display: flex;
            gap: 3px;
            margin-top: 8px;
        }
        
        .signal-bar {
            width: 4px;
            background: rgba(100, 116, 139, 0.4);
            border-radius: 2px;
        }
        
        .signal-bar:nth-child(1) { height: 6px; }
        .signal-bar:nth-child(2) { height: 10px; }
        .signal-bar:nth-child(3) { height: 14px; }
        .signal-bar:nth-child(4) { height: 18px; }
        .signal-bar:nth-child(5) { height: 22px; }
        
        .signal-bar.active {
            background: linear-gradient(0deg, var(--secondary), var(--primary));
        }
        
        .score-section {
            background: rgba(15, 23, 42, 0.7);
            border-radius: 16px;
            padding: 20px;
            text-align: center;
            margin: 0 20px 20px;
            border: 1px solid var(--border);
        }
        
        .score-value {
            font-size: 36px;
            font-weight: 800;
            background: linear-gradient(90deg, var(--primary), var(--secondary));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            margin-bottom: 4px;
        }
        
        .score-label {
            font-size: 14px;
            color: var(--text-secondary);
            font-weight: 500;
        }
        
        .progress-container {
            margin-top: 12px;
        }
        
        .progress-bar {
            height: 8px;
            background: rgba(30, 41, 59, 0.5);
            border-radius: 4px;
            overflow: hidden;
        }
        
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, var(--primary), var(--secondary));
            border-radius: 4px;
            width: 0%;
            transition: width 1s ease;
        }
        
        .progress-text {
            font-size: 11px;
            color: var(--text-secondary);
            margin-top: 4px;
            text-align: right;
        }
        
        .button-container {
            margin-top: 16px;
        }

        .button {
            background: linear-gradient(135deg, var(--primary), var(--highlight));
            border: none;
            color: white;
            padding: 10px 20px;
            border-radius: 8px;
            font-weight: 600;
            font-size: 14px;
            cursor: pointer;
            transition: all 0.3s ease;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            box-shadow: 0 4px 12px rgba(59, 130, 246, 0.3);
            width: 100%;
        }

        .button:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 16px rgba(59, 130, 246, 0.4);
        }

        .button:active {
            transform: translateY(0px);
        }

        .button-icon {
            margin-right: 8px;
            font-size: 16px;
        }
        
        .danger-button {
            background: linear-gradient(135deg, #f43f5e, #ef4444);
            box-shadow: 0 4px 12px rgba(239, 68, 68, 0.3);
            margin-top: 10px;
        }
        
        .danger-button:hover {
            box-shadow: 0 6px 16px rgba(239, 68, 68, 0.4);
        }
        
        .transmitter-grid {
            padding: 20px;
        }
        
        .grid-title {
            font-size: 18px;
            font-weight: 600;
            margin-bottom: 16px;
        }
        
        .grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 12px;
        }
        
        .grid-item {
            background: var(--card-bg);
            border-radius: 12px;
            padding: 16px;
            border: 1px solid var(--border);
            position: relative;
        }
        
        .grid-item.found {
            background: rgba(16, 185, 129, 0.15);
            border-color: rgba(16, 185, 129, 0.3);
        }
        
        .grid-item.active {
            background: rgba(14, 165, 233, 0.15);
            border-color: rgba(14, 165, 233, 0.3);
        }
        
        .transmitter-name {
            font-size: 16px;
            font-weight: 600;
            margin-bottom: 4px;
        }
        
        .transmitter-details {
            font-size: 12px;
            color: var(--text-secondary);
            font-family: monospace;
        }
        
        .points-badge {
            position: absolute;
            top: 12px;
            right: 12px;
            background: var(--card-bg);
            color: var(--primary);
            padding: 3px 8px;
            border-radius: 12px;
            font-size: 11px;
            font-weight: 600;
            border: 1px solid var(--border);
        }
        
        .found .points-badge {
            background: var(--secondary);
            color: white;
            border-color: rgba(16, 185, 129, 0.3);
        }
        
        .status-badge {
            display: inline-block;
            width: 8px;
            height: 8px;
            border-radius: 50%;
            margin-right: 5px;
        }
        
        .status-badge.lost {
            background: var(--text-secondary);
        }
        
        .status-badge.active {
            background: var(--primary);
            box-shadow: 0 0 5px var(--primary);
            animation: pulse 2s infinite;
        }
        
        .status-badge.found {
            background: var(--secondary);
            box-shadow: 0 0 5px var(--secondary);
        }
        
        .status-text {
            font-size: 11px;
            margin-top: 8px;
            display: flex;
            align-items: center;
        }
        
        .scanning-status {
            text-align: center;
            color: var(--secondary);
            font-family: monospace;
            font-size: 14px;
            font-weight: 600;
            margin: -10px 0 16px;
            letter-spacing: 1px;
        }
        
        .scanning-dots::after {
            content: '...';
            animation: dots 1.5s infinite;
            display: inline-block;
            width: 24px;
            text-align: left;
        }
        
        @keyframes dots {
            0%, 20% { content: '.'; }
            40%, 60% { content: '..'; }
            80%, 100% { content: '...'; }
        }
        
        .notification {
            position: fixed;
            top: 20px;
            left: 50%;
            transform: translateX(-50%);
            background: var(--card-bg);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 16px;
            box-shadow: 0 5px 20px rgba(0, 0, 0, 0.3);
            z-index: 100;
            max-width: 280px;
            opacity: 0;
            transition: opacity 0.3s ease;
            pointer-events: none;
            text-align: center;
        }
        
        .notification.active {
            opacity: 1;
        }
        
        .notification-title {
            font-weight: 600;
            margin-bottom: 4px;
        }
        
        .notification-body {
            font-size: 14px;
            color: var(--text-secondary);
        }
        
        .notification.success {
            border-color: var(--secondary);
        }
        
        .notification.success .notification-title {
            color: var(--secondary);
        }

        .notification.error {
            border-color: var(--danger);
        }
        
        .notification.error .notification-title {
            color: var(--danger);
        }
        
        .footer {
            padding: 16px;
            text-align: center;
            font-size: 11px;
            color: var(--text-secondary);
            border-top: 1px solid var(--border);
            margin-top: 20px;
        }

        /* Modal styles */
        .modal {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.7);
            z-index: 1000;
            justify-content: center;
            align-items: center;
        }
        
        .modal.active {
            display: flex;
        }
        
        .modal-content {
            background: var(--card-bg);
            border-radius: 16px;
            padding: 24px;
            width: 90%;
            max-width: 320px;
            border: 1px solid var(--border);
            text-align: center;
        }
        
        .modal-title {
            font-size: 18px;
            font-weight: 600;
            margin-bottom: 16px;
            color: var(--danger);
        }
        
        .modal-text {
            font-size: 14px;
            color: var(--text-secondary);
            margin-bottom: 20px;
        }
        
        .modal-buttons {
            display: flex;
            justify-content: space-between;
            gap: 12px;
        }
        
        .modal-button {
            flex: 1;
            padding: 10px;
            border-radius: 8px;
            font-weight: 600;
            font-size: 14px;
            cursor: pointer;
            transition: all 0.2s ease;
            border: none;
        }
        
        .cancel-button {
            background: var(--card-bg);
            color: var(--text);
            border: 1px solid var(--border);
        }
        
        .confirm-button {
            background: var(--danger);
            color: white;
        }
        
        @media (max-width: 380px) {
            .radar-container {
                width: 240px;
                height: 240px;
            }
            
            .radar-circle:nth-child(1) { width: 60px; height: 60px; }
            .radar-circle:nth-child(2) { width: 120px; height: 120px; }
            .radar-circle:nth-child(3) { width: 180px; height: 180px; }
            .radar-circle:nth-child(4) { width: 240px; height: 240px; }
            
            .title {
                font-size: 22px;
            }
            
            .score-value {
                font-size: 32px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <div class="header-content">
                <div class="logo">IEEE APS</div>
                <div class="status-indicator">
                    <div class="status-dot" id="connectionStatus"></div>
                    <span id="connectionText">CONNECTED</span>
                </div>
            </div>
            
            <div class="title">SIGNAL HUNT</div>
            <div class="subtitle">LOCATE HIDDEN TRANSMITTERS</div>
        </div>
        
        <div class="radar-container">
            <div class="radar-circles">
                <div class="radar-circle"></div>
                <div class="radar-circle"></div>
                <div class="radar-circle"></div>
                <div class="radar-circle"></div>
            </div>
            <div class="radar-axis"></div>
            <div class="radar-axis"></div>
            <div class="radar-axis"></div>
            <div class="radar-axis"></div>
            <div class="radar-center"></div>
            <div class="radar-sweep"></div>
            <div id="blipContainer"></div>
            
            <div class="distance-labels">
                <span class="distance-label" id="distLabel1">5m</span>
                <span class="distance-label" id="distLabel2">10m</span>
                <span class="distance-label" id="distLabel3">15m</span>
            </div>
        </div>
        
        <div class="scanning-status">
            <span id="scanningText">SCANNING</span>
            <span class="scanning-dots"></span>
        </div>
        
        <div class="current-target" id="currentTarget" style="display:none;">
            <div class="target-icon" id="targetIcon">TX</div>
            <div class="target-info">
                <div class="target-name" id="targetName">--</div>
                <div class="target-id" id="targetId">#----</div>
                <div class="target-distance" id="targetDistance">--</div>
                <div class="signal-bars" id="signalBars">
                    <div class="signal-bar"></div>
                    <div class="signal-bar"></div>
                    <div class="signal-bar"></div>
                    <div class="signal-bar"></div>
                    <div class="signal-bar"></div>
                </div>
            </div>
        </div>
        
        <div class="score-section">
            <div class="score-value" id="scoreValue">0</div>
            <div class="score-label">POINTS</div>
            
            <div class="progress-container">
                <div class="progress-bar">
                    <div class="progress-fill" id="progressFill"></div>
                </div>
                <div class="progress-text">
                    <span id="foundCount">0</span>/<span id="totalCount">6</span> transmitters
                </div>
            </div>
            
            <div class="button-container">
                <button class="button" onclick="downloadResults()">
                    <span class="button-icon">⬇️</span> Download Results
                </button>
                <button class="button danger-button" onclick="showResetModal()">
                    <span class="button-icon">🔄</span> Reset Game
                </button>
            </div>
        </div>
        
        <div class="transmitter-grid">
            <div class="grid-title">TRANSMITTERS</div>
            <div class="grid" id="transmitterGrid">
                <!-- Transmitter grid items will be populated by JavaScript -->
            </div>
        </div>
        
        <div class="notification" id="notification">
            <div class="notification-title" id="notificationTitle"></div>
            <div class="notification-body" id="notificationBody"></div>
        </div>
        
        <div class="footer">
            IEEE APS CUSAT Student Branch • ESP Signal Propagation Workshop 2025
        </div>
    </div>

    <!-- Reset confirmation modal -->
    <div class="modal" id="resetModal">
        <div class="modal-content">
            <div class="modal-title">Reset Game?</div>
            <div class="modal-text">
                This will clear all progress, found transmitters, and reset your score to zero. This action cannot be undone.
            </div>
            <div class="modal-buttons">
                <button class="modal-button cancel-button" onclick="hideResetModal()">Cancel</button>
                <button class="modal-button confirm-button" onclick="resetGame()">Reset</button>
            </div>
        </div>
    </div>

    <script>
        // Game state management
        const gameState = {
            connected: true,
            score: 0,
            foundTransmitters: new Set(),
            activeTransmitters: [],
            transmitterCount: 6,
            maxDistance: 30,
            resetMode: false
        };

        // Transmitter database (reference data)
        const transmitterDB = {
            1001: { name: "Alpha", points: 10, icon: "A" },
            1002: { name: "Beta", points: 15, icon: "B" },
            1003: { name: "Gamma", points: 20, icon: "Γ" },
            1004: { name: "Delta", points: 25, icon: "Δ" },
            1005: { name: "Epsilon", points: 30, icon: "E" },
            1006: { name: "Omega", points: 50, icon: "Ω" }
        };

        // DOM Elements
        const elements = {
            connectionStatus: document.getElementById('connectionStatus'),
            connectionText: document.getElementById('connectionText'),
            blipContainer: document.getElementById('blipContainer'),
            currentTarget: document.getElementById('currentTarget'),
            targetIcon: document.getElementById('targetIcon'),
            targetName: document.getElementById('targetName'),
            targetId: document.getElementById('targetId'),
            targetDistance: document.getElementById('targetDistance'),
            signalBars: document.getElementById('signalBars').querySelectorAll('.signal-bar'),
            scoreValue: document.getElementById('scoreValue'),
            progressFill: document.getElementById('progressFill'),
            foundCount: document.getElementById('foundCount'),
            totalCount: document.getElementById('totalCount'),
            transmitterGrid: document.getElementById('transmitterGrid'),
            scanningText: document.getElementById('scanningText'),
            notification: document.getElementById('notification'),
            notificationTitle: document.getElementById('notificationTitle'),
            notificationBody: document.getElementById('notificationBody'),
            resetModal: document.getElementById('resetModal'),
            distLabel1: document.getElementById('distLabel1'),
            distLabel2: document.getElementById('distLabel2'),
            distLabel3: document.getElementById('distLabel3')
        };

        // Initialize the transmitter grid
        function initTransmitterGrid() {
            elements.transmitterGrid.innerHTML = '';
            
            Object.entries(transmitterDB).forEach(([id, tx]) => {
                const item = document.createElement('div');
                item.className = 'grid-item';
                item.dataset.id = id;
                
                item.innerHTML = `
                    <div class="points-badge">${tx.points} pts</div>
                    <div class="transmitter-name">${tx.name}</div>
                    <div class="transmitter-details">#${id}</div>
                    <div class="status-text">
                        <span class="status-badge lost"></span>
                        <span class="status-text-value">Not detected</span>
                    </div>
                `;
                
                elements.transmitterGrid.appendChild(item);
            });
            
            elements.totalCount.textContent = Object.keys(transmitterDB).length;
        }

        // Update connection status UI
        function updateConnectionStatus(connected) {
            gameState.connected = connected;
            elements.connectionStatus.className = `status-dot ${connected ? '' : 'offline'}`;
            elements.connectionText.textContent = connected ? 'CONNECTED' : 'DISCONNECTED';
            
            if (!connected) {
                elements.scanningText.textContent = 'CONNECTION LOST';
                showNotification('Connection Lost', 'Check your WiFi connection', 'error');
            } else if (gameState.resetMode) {
                elements.scanningText.textContent = 'RESET COOLDOWN';
            } else {
                elements.scanningText.textContent = 'SCANNING';
            }
        }
        
        // Update the radar blips - improved to position based on angle and distance
        function updateRadarBlips(transmitters) {
            elements.blipContainer.innerHTML = '';
            
            // Update radar distance labels based on max distance
            const maxDist = gameState.maxDistance || 30;
            elements.distLabel1.textContent = Math.round(maxDist/6) + 'm';
            elements.distLabel2.textContent = Math.round(maxDist/3) + 'm';
            elements.distLabel3.textContent = Math.round(maxDist/2) + 'm';
            
            transmitters.forEach(tx => {
                if (!tx.isActive) return;
                
                const blip = document.createElement('div');
                blip.className = 'blip';
                
                // Calculate position based on distance and angle
                // Convert real distance to radar display distance (0-130px from center)
                const radarRadius = 130; // pixels from center to edge
                const centerX = 140;  // center X coordinate
                const centerY = 140;  // center Y coordinate
                
                // Scale distance - closer to center = closer in real life
                // Limit to 95% of radius for visibility
                const distanceRatio = Math.min(0.95, tx.distance / maxDist);
                const blipDistance = distanceRatio * radarRadius;
                
                // Use the transmitter's angle (0-359 degrees) for positioning
                // This creates consistent positioning that moves smoothly
                const angle = tx.angle || 0; 
                const radians = angle * Math.PI / 180;
                
                // Calculate X,Y position using polar coordinates
                const x = centerX + Math.cos(radians) * blipDistance;
                const y = centerY + Math.sin(radians) * blipDistance;
                
                // Apply the position
                blip.style.left = `${x}px`;
                blip.style.top = `${y}px`;
                
                // Set blip color based on transmitter ID (visually distinguishable)
                const txId = parseInt(tx.id);
                const hue = ((txId - 1000) * 60) % 360;
                blip.style.backgroundColor = `hsl(${hue}, 80%, 60%)`;
                blip.style.boxShadow = `0 0 10px hsl(${hue}, 80%, 60%)`;
                
                // Add tooltip showing transmitter name and distance
                blip.title = `${tx.name}: ${tx.distance.toFixed(1)}m`;
                
                elements.blipContainer.appendChild(blip);
            });
            
            updateCurrentTarget(transmitters);
        }

        // Update the current target display
        function updateCurrentTarget(transmitters) {
            // Find closest active transmitter
            const activeTransmitters = transmitters.filter(tx => tx.isActive);
            
            if (activeTransmitters.length === 0) {
                elements.currentTarget.style.display = 'none';
                return;
            }
            
            // Sort by distance and pick closest
            const closest = activeTransmitters.sort((a, b) => a.distance - b.distance)[0];
            
            // Update UI
            elements.currentTarget.style.display = 'flex';
            elements.targetIcon.textContent = transmitterDB[closest.id]?.icon || 'TX';
            elements.targetName.textContent = closest.name;
            elements.targetId.textContent = `#${closest.id}`;
            elements.targetDistance.textContent = `${closest.distance.toFixed(1)}m`;
            
            // Update signal strength bars (recalibrated to be more accurate)
            // Show fewer bars for longer distances
            const minDist = 1;   // 5 bars at 1m or less
            const maxDist = 15;  // 0 bars at 15m or more
            const normalizedStrength = Math.max(0, 1 - (closest.distance - minDist) / (maxDist - minDist));
            const barCount = Math.round(normalizedStrength * 5);
            
            elements.signalBars.forEach((bar, i) => {
                bar.className = `signal-bar ${i < barCount ? 'active' : ''}`;
            });
        }

        // Update transmitter grid status
        function updateTransmitterGrid(transmitters) {
            // Map of active transmitters for quick lookup
            const activeMap = {};
            transmitters.forEach(tx => {
                if (tx.isActive) {
                    activeMap[tx.id] = tx;
                }
            });
            
            // Update each grid item
            document.querySelectorAll('#transmitterGrid .grid-item').forEach(item => {
                const id = parseInt(item.dataset.id);
                const statusBadge = item.querySelector('.status-badge');
                const statusText = item.querySelector('.status-text-value');
                
                if (gameState.foundTransmitters.has(id)) {
                    // Transmitter is found
                    item.className = 'grid-item found';
                    statusBadge.className = 'status-badge found';
                    statusText.textContent = 'Found';
                } else if (activeMap[id]) {
                    // Transmitter is active
                    const tx = activeMap[id];
                    item.className = 'grid-item active';
                    statusBadge.className = 'status-badge active';
                    statusText.textContent = `${tx.distance.toFixed(1)}m`;
                } else {
                    // Transmitter is not detected
                    item.className = 'grid-item';
                    statusBadge.className = 'status-badge lost';
                    statusText.textContent = 'Not detected';
                }
            });
        }

        // Update score and progress
        function updateScore() {
            elements.scoreValue.textContent = gameState.score;
            elements.foundCount.textContent = gameState.foundTransmitters.size;
            
            const progress = (gameState.foundTransmitters.size / gameState.transmitterCount) * 100;
            elements.progressFill.style.width = `${progress}%`;
        }

        // Check for newly discovered transmitters
        function checkDiscoveries(transmitters) {
            // Skip discovery checks during reset mode
            if (gameState.resetMode) return;
            
            transmitters.forEach(tx => {
                if (tx.isActive && 
                    tx.distance <= 2.0 && // Discovery range now 2.0m
                    !gameState.foundTransmitters.has(tx.id)) {
                    
                    // New discovery!
                    gameState.foundTransmitters.add(tx.id);
                    gameState.score += transmitterDB[tx.id]?.points || 0;
                    
                    // Update UI
                    updateScore();
                    updateTransmitterGrid(transmitters);
                    
                    // Show notification
                    const name = transmitterDB[tx.id]?.name || 'Transmitter';
                    const points = transmitterDB[tx.id]?.points || 0;
                    
                    showNotification(
                        `${name} Found!`, 
                        `+${points} points added to your score`, 
                        'success'
                    );
                    
                    // Update scanning text
                                        // Update scanning text
                    elements.scanningText.textContent = `FOUND ${name}!`;
                    setTimeout(() => {
                        if (gameState.connected) {
                            if (gameState.resetMode) {
                                elements.scanningText.textContent = 'RESET COOLDOWN';
                            } else {
                                elements.scanningText.textContent = 'SCANNING';
                            }
                        }
                    }, 3000);
                }
            });
        }

        // Show notification
        function showNotification(title, body, type = 'info') {
            elements.notification.className = `notification ${type} active`;
            elements.notificationTitle.textContent = title;
            elements.notificationBody.textContent = body;
            
            setTimeout(() => {
                elements.notification.className = 'notification';
            }, 4000);
        }

        // Show reset confirmation modal
        function showResetModal() {
            elements.resetModal.classList.add('active');
        }
        
        // Hide reset confirmation modal
        function hideResetModal() {
            elements.resetModal.classList.remove('active');
        }
        
        // Reset the game
        function resetGame() {
            // First clear local UI immediately
            gameState.resetMode = true;
            gameState.activeTransmitters = [];
            gameState.score = 0;
            gameState.foundTransmitters = new Set();
            
            // Update UI immediately
            elements.blipContainer.innerHTML = '';
            elements.currentTarget.style.display = 'none';
            elements.scanningText.textContent = 'RESETTING...';
            updateScore();
            updateTransmitterGrid([]);
            
            // Call server to reset game state
            fetch('/api/reset')
                .then(response => response.json())
                .then(data => {
                    if (data.status === 'success') {
                        // Show success notification
                        showNotification(
                            'Game Reset', 
                            'Your progress has been reset. Please wait 5 seconds before new discoveries.', 
                            'success'
                        );
                        
                        // Update scanning text to show cooldown
                        elements.scanningText.textContent = 'RESET COOLDOWN';
                        
                        // Hide modal
                        hideResetModal();
                    } else {
                        // Show error notification
                        showNotification('Reset Failed', 'Could not reset the game', 'error');
                        gameState.resetMode = false;
                    }
                })
                .catch(err => {
                    console.error('Error resetting game:', err);
                    showNotification('Reset Failed', 'Could not reset the game', 'error');
                    gameState.resetMode = false;
                    elements.scanningText.textContent = 'ERROR';
                });
        }

        // Download results as JSON file
        function downloadResults() {
            showNotification('Preparing Download', 'Creating results file...', 'info');
            
            // Trigger download from the server
            window.location.href = '/api/download';
            
            // Show success notification after a delay
            setTimeout(() => {
                showNotification('Download Complete', 'Results saved to your device', 'success');
            }, 1000);
        }

        // API CALLS

        // Fetch transmitter data
        function fetchTransmitterData() {
            fetch('/api/transmitters')
                .then(res => res.json())
                .then(data => {
                    updateConnectionStatus(true);
                    gameState.activeTransmitters = data.transmitters || [];
                    gameState.resetMode = data.resetMode || false;
                    
                    if (data.maxRange) {
                        gameState.maxDistance = data.maxRange;
                    }
                    
                    updateRadarBlips(gameState.activeTransmitters);
                    updateTransmitterGrid(gameState.activeTransmitters);
                    checkDiscoveries(gameState.activeTransmitters);
                    
                    // Update scanning text based on reset mode
                    if (gameState.resetMode && elements.scanningText.textContent !== 'CONNECTION LOST') {
                        elements.scanningText.textContent = 'RESET COOLDOWN';
                    }
                })
                .catch(err => {
                    console.error('Failed to fetch transmitter data', err);
                    updateConnectionStatus(false);
                });
        }

        // Fetch score data
        function fetchScoreData() {
            fetch('/api/score')
                .then(res => res.json())
                .then(data => {
                    gameState.score = data.totalScore;
                    
                    // Update found transmitters
                    if (Array.isArray(data.foundTransmitters)) {
                        gameState.foundTransmitters = new Set(data.foundTransmitters);
                    }
                    
                    updateScore();
                    updateTransmitterGrid(gameState.activeTransmitters);
                })
                .catch(err => {
                    console.error('Failed to fetch score data', err);
                });
        }

        // Fetch system status
        function fetchSystemStatus() {
            fetch('/api/status')
                .then(res => res.json())
                .then(data => {
                    if (data.resetMode !== undefined) {
                        gameState.resetMode = data.resetMode;
                    }
                })
                .catch(err => {
                    console.error('Failed to fetch status data', err);
                });
        }

        // Initialize the application
        function initialize() {
            initTransmitterGrid();
            updateConnectionStatus(true);
            updateScore();
            
            // Fetch initial data
            fetchTransmitterData();
            fetchScoreData();
            fetchSystemStatus();
            
            // Set up periodic data fetching
            setInterval(fetchTransmitterData, 1000);  // Update transmitter data every 1 second
            setInterval(fetchScoreData, 5000);        // Update score data every 5 seconds
            setInterval(fetchSystemStatus, 10000);    // Update system status every 10 seconds
        }

        // Start the app when document is ready
        document.addEventListener('DOMContentLoaded', initialize);
    </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}
