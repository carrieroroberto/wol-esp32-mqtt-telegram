#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// NETWORK CONFIGURATION
// ============================================================================
#define WIFI_SSID ""
#define WIFI_PASS ""

// ============================================================================
// WAKE-ON-LAN CONFIGURATION
// ============================================================================
#define MAC_ADDR "XX:XX:XX:XX:XX:XX"
const IPAddress PC_IP(0, 0, 0, 0);

// ============================================================================
// MQTT BROKER CONFIGURATION
// ============================================================================
#define MQTT_HOST ""
#define MQTT_PORT 8883
#define MQTT_USER ""
#define MQTT_PASS ""
#define MQTT_TOPIC_COMMANDS ""
#define MQTT_TOPIC_RESPONSE ""

#endif // CONFIG_H