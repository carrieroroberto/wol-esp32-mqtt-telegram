# -------------------------------
# Import required libraries
# -------------------------------
import os
import json
import threading
import asyncio
from dotenv import load_dotenv
import paho.mqtt.client as mqtt
from telegram import Update
from telegram.ext import ApplicationBuilder, CommandHandler, ContextTypes

# Load environment variables from .env
load_dotenv()

# -------------------------------
# Configuration
# -------------------------------
BOT_TOKEN = os.getenv("BOT_TOKEN")
ALLOWED_ID = int(os.getenv("ALLOWED_ID"))

MQTT_HOST = os.getenv("MQTT_HOST")
MQTT_PORT = int(os.getenv("MQTT_PORT"))
MQTT_TOPIC_COMMANDS = os.getenv("MQTT_TOPIC_COMMANDS")
MQTT_TOPIC_RESPONSE = os.getenv("MQTT_TOPIC_RESPONSE")
MQTT_USER = os.getenv("MQTT_USER")
MQTT_PASS = os.getenv("MQTT_PASS")

# -------------------------------
# Global state
# -------------------------------
app = None
mqtt_client = None
mqtt_connected = False

# -------------------------------
# MQTT callbacks
# -------------------------------
def on_connect(client, userdata, flags, rc):
    """Callback when MQTT connects to the broker"""
    global mqtt_connected
    mqtt_connected = rc == 0
    if mqtt_connected:
        print("MQTT connected successfully")
        client.subscribe(MQTT_TOPIC_RESPONSE)
    else:
        print(f"MQTT connection failed, rc={rc}")

def on_message(client, userdata, msg):
    """Callback when MQTT message is received"""
    payload = msg.payload.decode()
    loop = userdata["loop"]
    text = ""

    # Map MQTT payloads to human-readable messages
    if payload == "/al_on":
        text = "💡 The PC is already on"
    elif payload == "/wol_sent":
        text = "⚡ Magic Packet has been sent"
    elif payload == "/wol_ok":
        text = "✅ The PC turned on successfully"
    elif payload == "/wol_fail":
        text = "❌ The PC failed to turn on, please retry"
    elif payload == "/ping_ok":
        text = "💖 The PC is online"
    elif payload == "/ping_fail":
        text = "❌ The PC is offline or unreachable"
    elif payload.startswith("/stat_info"):
        try:
            # Rimuove il prefisso "/stat_info "
            json_data = json.loads(payload[len("/stat_info "):])
            text = "📊 Bot info:\n"
            for k, v in json_data.items():
                text += f"{k}: {v}\n"
        except Exception as e:
            text = f"⚠️ Error parsing status info: {e}"

    # Send message to Telegram in the asyncio loop
    asyncio.run_coroutine_threadsafe(
        app.bot.send_message(chat_id=ALLOWED_ID, text=text),
        loop
    )

# -------------------------------
# MQTT initialization
# -------------------------------
def start_mqtt(loop):
    """Start MQTT client in a separate thread"""
    global mqtt_client
    mqtt_client = mqtt.Client(userdata={"loop": loop})
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASS)
    mqtt_client.tls_set()  # Use default TLS settings
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    mqtt_client.connect(MQTT_HOST, MQTT_PORT)
    mqtt_client.loop_forever()

def publish_command(command: str):
    """Publish a command to MQTT if connected"""
    if mqtt_client and mqtt_connected:
        mqtt_client.publish(MQTT_TOPIC_COMMANDS, command)
    else:
        print(f"Cannot publish command '{command}', MQTT not connected")

# -------------------------------
# Telegram command handlers
# -------------------------------
async def start(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """Send welcome message"""
    if update.effective_user.id != ALLOWED_ID:
        return
    welcome = (
        "👋🏻 Hi, I'm *WoL*, your personal Wake-on-LAN bot!\n\n"
        "Available commands:\n"
        "🚀 /wol - Turn on your PC using Magic Packet\n"
        "💖 /ping - Check if the PC is on\n"
        "📊 /status - Show bot status\n"
        "✨ /start - Show this welcome message"
    )
    await update.message.reply_text(welcome, parse_mode="Markdown")

async def ping(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """Send /ping command via MQTT"""
    if update.effective_user.id != ALLOWED_ID:
        return
    publish_command("/ping")
    await update.message.reply_text("⏱ Checking PC status...")

async def status(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """Send /status command via MQTT"""
    if update.effective_user.id != ALLOWED_ID:
        return
    publish_command("/status")
    await update.message.reply_text("📊 Fetching PC status...")

async def wol(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """Send /wol command via MQTT"""
    if update.effective_user.id != ALLOWED_ID:
        return
    publish_command("/wol")
    await update.message.reply_text("⚡ Sending Magic Packet to turn on PC...")

# -------------------------------
# Main bot startup
# -------------------------------
def main():
    global app
    app = ApplicationBuilder().token(BOT_TOKEN).build()

    # Add Telegram command handlers
    app.add_handler(CommandHandler("start", start))
    app.add_handler(CommandHandler("ping", ping))
    app.add_handler(CommandHandler("status", status))
    app.add_handler(CommandHandler("wol", wol))

    # Start MQTT client in a background thread
    loop = asyncio.get_event_loop()
    mqtt_thread = threading.Thread(target=start_mqtt, args=(loop,), daemon=True)
    mqtt_thread.start()

    # Start polling Telegram for updates
    app.run_polling()

# -------------------------------
# Entry point
# -------------------------------
if __name__ == "__main__":
    main()