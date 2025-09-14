#include <WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include "WakeOnLan.h"
#include <ESPping.h>
#include "config.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================
WiFiClient wifiClient;               // WiFi client for MQTT
PubSubClient mqttClient(wifiClient); // MQTT client using WiFi
WiFiUDP udp;                         // UDP client for WoL
WakeOnLan wol(udp);                  // WoL object to send magic packets

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
unsigned long startTime;                    // Tracks start time of ESP32 for uptime
unsigned long lastMQTTReconnectAttempt = 0; // Last attempt to reconnect to MQTT

// ============================================================================
// FUNCTION: formatUptime
// Converts seconds into human-readable "h m s" format
// ============================================================================
String formatUptime(unsigned long seconds) {
  unsigned long h = seconds / 3600;
  unsigned long m = (seconds % 3600) / 60;
  unsigned long s = seconds % 60;
  return String(h) + "h " + String(m) + "m " + String(s) + "s";
}

// ============================================================================
// FUNCTION: publishResponse
// Publishes a string message to the MQTT response topic
// ============================================================================
void publishResponse(const char* msg) {
  mqttClient.publish("wol/response", msg);
}

// ============================================================================
// FUNCTION: isPCOnline
// Checks if the target PC responds to ping requests
// Returns true if PC is online, false otherwise
// ============================================================================
bool isPCOnline() {
  return Ping.ping(PC_IP, 3);       // 3 attempts for reliability
}

// ============================================================================
// FUNCTION: executeWakeOnLAN
// Sends a Wake-on-LAN magic packet to the target PC
// Publishes status messages to MQTT topic
// ============================================================================
void executeWakeOnLAN() {
  // If PC is already online, inform via MQTT
  if (isPCOnline()) {
    publishResponse("/al_on");
    return;
  }

  // Send WoL packet
  wol.sendMagicPacket(MAC_ADDR);
  publishResponse("/wol_sent");

  // Wait 20 seconds for PC to boot
  delay(20000);

  // Verify if PC is online after WoL
  if (isPCOnline()) publishResponse("/wol_ok");
  else publishResponse("/wol_fail");
}

// ============================================================================
// FUNCTION: onMqttMessage
// MQTT callback executed when a message arrives on subscribed topics
// ============================================================================
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  // Convert payload bytes to string
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  // Handle commands on the subscribed topic
  if (String(topic) == "wol/commands") {

    // Wake-on-LAN command
    if (msg == "/wol") executeWakeOnLAN();

    // Ping command to check PC status
    else if (msg == "/ping") {
      if (isPCOnline()) publishResponse("/ping_ok");
      else publishResponse("/ping_fail");
    }

    // Status command: publish PC info with /stat_info prefix
    else if (msg == "/status") {
        // Calculate uptime
        String uptime = formatUptime((millis() - startTime) / 1000);

        // Create info string
        String infoMsg = "/stat_info {";
        infoMsg += "\"Status\":\"Online\",";
        infoMsg += "\"Local IP\":\"" + WiFi.localIP().toString() + "\",";
        infoMsg += "\"SSID\":\"" + WiFi.SSID() + "\",";
        infoMsg += "\"Uptime\":\"" + uptime + "\"";
        infoMsg += "}";

        // Publish on response topic
        mqttClient.publish("wol/response", infoMsg.c_str());
    }
  }
}

// ============================================================================
// FUNCTION: reconnectMQTT
// Attempts to reconnect to the MQTT broker if disconnected
// Returns true if successful, false otherwise
// ============================================================================
boolean reconnectMQTT() {
  if (mqttClient.connect("ESP32_WoL", MQTT_USER, MQTT_PASS)) {
    mqttClient.subscribe("wol/commands");       // Subscribe to command topic
    return true;
  }
  return false;
}

// ============================================================================
// SETUP
// Initializes WiFi, MQTT, and Wake-on-LAN
// ============================================================================
void setup() {
  Serial.begin(115200);
  startTime = millis();     // Save start time for uptime calculation

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }

  // Prepare WoL broadcast address based on local network
  wol.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());

  // Setup MQTT server and callback
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(onMqttMessage);
}

// ============================================================================
// LOOP
// Handles MQTT reconnects and processes incoming messages
// ============================================================================
void loop() {
  // Attempt MQTT reconnect if disconnected
  if (!mqttClient.connected()) {
    unsigned long now = millis();
    if (now - lastMQTTReconnectAttempt > 5000) {            // Retry every 5s
      if (reconnectMQTT()) lastMQTTReconnectAttempt = 0;
      else lastMQTTReconnectAttempt = now;
    }
  } 
  // If connected, process MQTT messages
  else {
    mqttClient.loop();
  }
}