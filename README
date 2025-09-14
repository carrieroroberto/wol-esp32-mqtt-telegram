# Remote Wake-on-LAN System with ESP32, MQTT & Telegram

![ESP32 Wake-on-LAN](https://img.shields.io/badge/ESP32-WoL-blue) ![Python](https://img.shields.io/badge/Python-FastAPI-green) ![MQTT](https://img.shields.io/badge/MQTT-Secure-orange)

A **remote Wake-on-LAN (WoL) system** that allows you to power on your PC from anywhere in the world using an **ESP32 microcontroller**, **MQTT**, and either **Telegram bot** or **HTTP API**. Fully deployable, secure, and designed following industry best practices for IoT projects.

---

## Table of Contents

- [Features](#features)  
- [Architecture](#architecture)  
- [Hardware Requirements](#hardware-requirements)  
- [Software Requirements](#software-requirements)  
- [ESP32 Setup](#esp32-setup)  
- [Python Services](#python-services)  
- [Usage](#usage)  
- [Security Considerations](#security-considerations)  
- [Portfolio / CV Benefits](#portfolio--cv-benefits)

---

## Features

- 🔌 **Remote Wake-on-LAN:** Power on your PC from anywhere.  
- 📊 **Real-time PC status:** Check if the PC is online and receive uptime info.  
- 💬 **Telegram Bot Integration:** Interact easily with `/wol`, `/ping`, `/status` commands.  
- 🌐 **HTTP API Endpoint:** Trigger WoL via FastAPI REST calls.  
- 🔒 **Secure MQTT Communication:** TLS-enabled broker for global access.  
- 🛠 **Modular & Deploy-Ready:** ESP32 firmware, Python services, and environment-based config.

---

## Architecture
  
  ┌────────────┐
  │  Telegram  │
  │   Bot /    │
  │  HTTP API  │
  └─────┬──────┘
        │ MQTT Commands (/wol, /ping, /status)
        ▼
   ┌───────────┐        ┌─────────────┐
   │  MQTT     │ <----> │ ESP32       │
   │ Broker    │        │ (WiFi + WoL)│
   └─────┬─────┘        └─────┬───────┘
         │                     │
         │                     ▼
         │            Magic Packet over LAN
         │                     │
         ▼                     ▼
    Python Services       Target PC
(Telegram & HTTP)          Powers On

  
---

## Hardware Requirements

- **ESP32 Development Board** (WiFi-enabled)  
- **Target PC** supporting Wake-on-LAN (WoL) in BIOS/UEFI  
- WiFi network accessible by ESP32  
- Optional: Cloud server or VPS to run Python Telegram Bot / FastAPI HTTP service

---

## Software Requirements

### ESP32 Firmware

- Arduino IDE or PlatformIO  
- Libraries: `WiFi`, `WiFiUdp`, `PubSubClient`, `WakeOnLan`, `ESPping`

### Python Services

- Python 3.10+  
- Packages:

```bash
fastapi
paho-mqtt
python-dotenv
python-telegram-bot

.env file with credentials:
BOT_TOKEN=
ALLOWED_ID=
MQTT_HOST=
MQTT_PORT=8883
MQTT_TOPIC_COMMANDS=
MQTT_TOPIC_RESPONSE=
MQTT_USER=
MQTT_PASS=

ESP32 Setup

Clone the repository and open wol_esp32_telegram_v4.ino in Arduino IDE.

Modify config.h with your WiFi, MQTT, and WoL PC info:
#define WIFI_SSID "your_wifi_name"
#define WIFI_PASS "your_wifi_password"
#define MAC_ADDR "AA:BB:CC:DD:EE:FF"
const IPAddress PC_IP(192, 168, 1, 100);
#define MQTT_HOST "your_mqtt_host"
#define MQTT_PORT 8883
#define MQTT_USER "mqtt_user"
#define MQTT_PASS "mqtt_password"
#define MQTT_TOPIC_COMMANDS "wol/commands"
#define MQTT_TOPIC_RESPONSE "wol/response"

Upload to ESP32 and open Serial Monitor to verify connectivity.

Python Services
FastAPI HTTP Trigger

fastapi_wol.py exposes endpoints /wol and /ping.

Example usage:
curl http://localhost:8000/wol
curl http://localhost:8000/ping

Publishes MQTT commands securely via TLS.

Telegram Bot

telegram_wol_bot.py listens to Telegram commands (/wol, /ping, /status) and forwards them to ESP32 via MQTT.

Responses are displayed in Telegram with status and uptime info.

Usage

Turn on PC remotely:

Via Telegram: /wol

Via HTTP API: GET /wol

Check PC status:

Telegram: /ping or /status

HTTP: GET /ping

View uptime, IP, and network info in Telegram via /status.

Security Considerations

MQTT connection uses TLS encryption.

Telegram bot restricts access to ALLOWED_ID.

Never commit real credentials; use .env or config.h safely.
