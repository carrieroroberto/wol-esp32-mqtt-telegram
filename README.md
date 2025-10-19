# Remote Wake-on-LAN System with ESP32, MQTT & Telegram Bot

![ESP32 Wake-on-LAN](https://img.shields.io/badge/ESP32-WoL-blue) ![Python](https://img.shields.io/badge/Python-FastAPI-green) ![MQTT](https://img.shields.io/badge/MQTT-Secure-orange)

A **remote Wake-on-LAN (WoL) system** that allows powering on home PCs from anywhere in the world using an **ESP32 microcontroller**, **MQTT**, and either **Telegram bot** or **HTTP API**.

<p align="center">
  <img src="https://github.com/user-attachments/assets/c5233cba-2a80-494a-99ce-af0caf9d86dd" alt="Immagine WhatsApp 2025-10-19 ore" width="350">
</p>

---

## Table of Contents

- [Architecture](#architecture)  
- [Features](#features)  
- [Hardware Requirements](#hardware-requirements)  
- [Software Requirements](#software-requirements)  
- [ESP32 Setup](#esp32-setup)  
- [Telegram Bot Setup](#telegram-bot)  
- [Python Services Setup](#python-services-setup)  
- [Usage](#usage)  
- [Security Considerations](#security-considerations)  
- [Portfolio / CV Benefits](#portfolio--cv-benefits)

---

## Architecture

      ┌───────────────────────────┐
      │   Telegram Bot / HTTP API │
      │  (User triggers commands) │
      └─────────────┬─────────────┘
                    │
                    ▼
           ┌────────────────┐
           │   MQTT Broker  │
           │  (TLS enabled) │
           └────────┬───────┘
                    │
                    ▼
             ┌─────────────┐
             │    ESP32    │
             │  (WiFi +    │
             │  WoL client)│
             └─────┬───────┘
                   │
                   ▼
           ┌────────────────┐
           │   Target PC    │
           │  (WoL enabled) │
           └────────────────┘

---

## Features

- 🔌 **Remote Wake-on-LAN:** Power on your PC from anywhere.  
- 📊 **Real-time PC status:** Check if the PC is online and receive uptime info.  
- 💬 **Telegram Bot Integration:** Interact easily with `/wol`, `/ping`, `/status` commands.  
- 🌐 **HTTP API Endpoint:** Trigger WoL via REST calls.  
- 🔒 **Secure MQTT Communication:** TLS-enabled broker for global access.  
- 🛠 **Modular & Deploy-Ready:** Firmware, Python services, and environment-based configuration.

---

## Hardware Requirements

- **ESP32 Development Board** (WiFi-enabled)  
- **Target PC** supporting Wake-on-LAN (WoL) in BIOS/UEFI  
- WiFi network accessible by ESP32  
- Optional: Cloud server or VPS to run Python Telegram Bot / HTTP service

---

## Software Requirements

- **ESP32 Firmware Tools:** Arduino IDE or PlatformIO  
- **Python 3** for services  
- Environment-based configuration using `.env` files and `config.h`

---

## ESP32 Setup

1. Open `config.h` in your firmware directory.  
2. Modify the following lines with your personal information:

```bash
#define WIFI_SSID "your_wifi_name"
#define WIFI_PASS "your_wifi_password"
#define MAC_ADDR "AA:BB:CC:DD:EE:FF"
const IPAddress PC_IP(0, 0, 0, 0);
#define MQTT_HOST "your_mqtt_host"
#define MQTT_PORT 8883
#define MQTT_USER "mqtt_user"
#define MQTT_PASS "mqtt_password"
#define MQTT_TOPIC_COMMANDS "your_commands_topic"
#define MQTT_TOPIC_RESPONSE "your_response_topic"
```

3. Upload the firmware to your ESP32 and verify connectivity via Serial Monitor.

---

## Telegram Bot Setup

1. **Create a new bot** via [BotFather](https://t.me/BotFather) on Telegram.  
2. **Save the `BOT_TOKEN`** provided by BotFather. This will be needed later in your `.env` file.  
3. **Obtain your Telegram user ID** to restrict access to authorized users and set it as `ALLOWED_ID` in your `.env` file.

---

## Python Services Setup

1. Modify `.env` files in both services folders.
2. Add your credentials:

### HTTP Wake-On-Lan Trigger Server:
```bash
MQTT_HOST=your_mqtt_host
MQTT_PORT=8883
MQTT_TOPIC_COMMANDS=your_commands_topic
MQTT_USER=mqtt_user
MQTT_PASS=mqtt_password
```

### Telegram Bot:
```bash
BOT_TOKEN=your_telegram_bot_token
ALLOWED_ID=your_telegram_user_id
MQTT_HOST=your_mqtt_host
MQTT_PORT=8883
MQTT_TOPIC_COMMANDS=your_commands_topic
MQTT_USER=mqtt_user
MQTT_PASS=mqtt_password
```

3. Run the FastAPI service to trigger WoL via HTTP.  
4. Run the Telegram bot to interact with ESP32 via MQTT.  

---

## Usage

**Turn on PC remotely:**

- Telegram: `/wol`  
- HTTP API: GET `/wol`  

**Check PC status:**

- Telegram: `/ping`

**Check services status:**

- Telegram: `/status`  
- HTTP API: `/ping`  

---

## Security Considerations

- MQTT connection uses TLS encryption.  
- Telegram bot access is restricted to authorized users (ALLOWED_ID).  
- Always use `.env` and `config.h` for credentials; **never commit them publicly**.
