#include <Arduino.h>
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
// FUNCTION PROTOTYPES
// ============================================================================
String formatUptime(unsigned long seconds);
void publishResponse(const char* msg);
bool isPCOnline();
void executeWakeOnLAN();
void onMqttMessage(char* topic, byte* payload, unsigned int length);
bool reconnectMQTT();

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
    mqttClient.publish(MQTT_TOPIC_RESPONSE, msg);
}

// ============================================================================
// FUNCTION: isPCOnline
// Checks if the target PC responds to ping requests
// ============================================================================
bool isPCOnline() {
    return Ping.ping(PC_IP, 3);  // 3 attempts for reliability
}

// ============================================================================
// FUNCTION: executeWakeOnLAN
// Sends a Wake-on-LAN magic packet and publishes status via MQTT
// ============================================================================
void executeWakeOnLAN() {
    if (isPCOnline()) {
        publishResponse("/al_on");
        return;
    }

    wol.sendMagicPacket(MAC_ADDR);
    publishResponse("/wol_sent");

    delay(20000); // Wait 20s for PC to boot

    if (isPCOnline()) publishResponse("/wol_ok");
    else publishResponse("/wol_fail");
}

// ============================================================================
// FUNCTION: onMqttMessage
// MQTT callback executed when a message arrives
// ============================================================================
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

    if (String(topic) == MQTT_TOPIC_COMMANDS) {

        if (msg == "/wol") executeWakeOnLAN();

        else if (msg == "/ping") {
            if (isPCOnline()) publishResponse("/ping_ok");
            else publishResponse("/ping_fail");
        }

        else if (msg == "/status") {
            String uptime = formatUptime((millis() - startTime) / 1000);

            String infoMsg = "/stat_info {";
            infoMsg += "\"Status\":\"Online\",";
            infoMsg += "\"Local IP\":\"" + WiFi.localIP().toString() + "\",";
            infoMsg += "\"SSID\":\"" + WiFi.SSID() + "\",";
            infoMsg += "\"Uptime\":\"" + uptime + "\"";
            infoMsg += "}";

            mqttClient.publish(MQTT_TOPIC_RESPONSE, infoMsg.c_str());
        }
    }
}

// ============================================================================
// FUNCTION: reconnectMQTT
// Attempts to reconnect to the MQTT broker if disconnected
// ============================================================================
bool reconnectMQTT() {
    if (mqttClient.connect("ESP32_WoL", MQTT_USER, MQTT_PASS)) {
        mqttClient.subscribe(MQTT_TOPIC_COMMANDS);
        return true;
    }
    return false;
}

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    Serial.begin(115200);
    startTime = millis();

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) delay(250);

    // Prepare WoL broadcast address
    wol.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());

    // Setup MQTT server and callback
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCallback(onMqttMessage);
}

// ============================================================================
// LOOP
// ============================================================================
void loop() {
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastMQTTReconnectAttempt > 5000) {
            if (reconnectMQTT()) lastMQTTReconnectAttempt = 0;
            else lastMQTTReconnectAttempt = now;
        }
    } else {
        mqttClient.loop();
    }
}