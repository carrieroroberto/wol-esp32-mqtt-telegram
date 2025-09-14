# -------------------------------
# Import required libraries
# -------------------------------
from fastapi import FastAPI, HTTPException
import paho.mqtt.client as mqtt
import ssl
import threading
import os
from dotenv import load_dotenv

# -------------------------------
# Load environment variables
# -------------------------------
load_dotenv()

MQTT_HOST = os.getenv("MQTT_HOST")
MQTT_PORT = int(os.getenv("MQTT_PORT", "8883"))
MQTT_USER = os.getenv("MQTT_USER")
MQTT_PASS = os.getenv("MQTT_PASS")
MQTT_TOPIC = os.getenv("MQTT_TOPIC")

# -------------------------------
# Initialize FastAPI app
# -------------------------------
app = FastAPI()

# -------------------------------
# Function to publish a message to MQTT
# -------------------------------
def publish_message(payload: str):
    """
    Publishes a payload to the MQTT broker securely over TLS.
    Raises HTTPException if any MQTT or general error occurs.
    """
    try:
        # Create a new MQTT client
        client = mqtt.Client()

        # Callback executed on MQTT connection
        def on_connect(client, userdata, flags, rc):
            if rc == 0:
                print("MQTT connected successfully!")
            else:
                print(f"MQTT connection failed, error code {rc}")

        # Assign the on_connect callback
        client.on_connect = on_connect

        # Set MQTT authentication credentials
        client.username_pw_set(MQTT_USER, MQTT_PASS)

        # Enable TLS/SSL for secure communication
        client.tls_set(tls_version=ssl.PROTOCOL_TLS)

        # Connect to MQTT broker
        client.connect(MQTT_HOST, MQTT_PORT)

        # Start the network loop in background to handle callbacks
        client.loop_start()

        # Publish the payload to the topic
        result = client.publish(MQTT_TOPIC, payload)
        result.wait_for_publish()  # Wait until the message is actually sent

        # Stop the network loop and disconnect
        client.loop_stop()
        client.disconnect()

        # Log success
        print(f"Message published to {MQTT_TOPIC}: {payload}")

    # Handle MQTT-specific exceptions
    except mqtt.MQTTException as e:
        print(f"MQTT Error: {e}")
        raise HTTPException(status_code=500, detail=f"MQTT Error: {e}")

    # Handle any other general exceptions
    except Exception as e:
        print(f"General error while publishing: {e}")
        raise HTTPException(status_code=500, detail=f"General error: {e}")

# -------------------------------
# Endpoint: /wol
# Sends the MQTT "/wol" command
# Returns HTTP 200 if successful, raises HTTP 500 if an error occurs
# -------------------------------
@app.get("/wol")
async def wol():
    """
    Sends the /wol command via MQTT.
    Runs publishing in a background thread to avoid blocking FastAPI.
    """
    threading.Thread(target=publish_message, args=("/wol",)).start()
    return {"status": "success", "topic": MQTT_TOPIC, "message": "/wol"}

# -------------------------------
# Endpoint: /ping
# Checks if server and MQTT broker are awake
# Returns HTTP 200 if successful, raises HTTP 500 if connection fails
# -------------------------------
@app.get("/ping")
async def ping():
    """
    Tests MQTT broker connectivity without publishing any message.
    Raises HTTPException if connection fails.
    """
    try:
        # Create MQTT client for testing connection
        client = mqtt.Client()
        client.username_pw_set(MQTT_USER, MQTT_PASS)
        client.tls_set(tls_version=ssl.PROTOCOL_TLS)
        client.connect(MQTT_HOST, MQTT_PORT)
        client.disconnect()  # Disconnect immediately; no message published

        return {"status": "success", "message": "server and MQTT broker awake"}

    except Exception as e:
        # Raise HTTP 500 error if broker is not reachable
        raise HTTPException(status_code=500, detail=f"MQTT connection failed: {e}")