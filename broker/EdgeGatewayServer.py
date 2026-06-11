import asyncio
import paho.mqtt.client as mqtt
import json
import time
import logging
from amqtt.broker import Broker

# Configuration logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger('EdgeGateway')


# CONFIGURATION LOCAL BROKER & LOGIC
LOCAL_BROKER = "172.20.10.4"
#LOCAL_BROKER = "10.0.217.167"
# 10.0.217.167
LOCAL_PORT = 1883
TOPIC_SENSOR_IN = "home/sensors"
TOPIC_ACTUATOR_OUT = "home/actuators/led1"

# Configuration for AMQTT
broker_config = {
    'listeners': {
        'default': {
            'type': 'tcp',
            'bind': '0.0.0.0:1883',
        }
    },
    'sys_interval': 10,
    'auth': {
        'allow-anonymous': True,
    }
}

# CONFIGURATION CLOUD GATEWAY (app.coreiot.io)
CLOUD_HOST = 'app.coreiot.io'
CLOUD_PORT = 1883
GATEWAY_ACCESS_TOKEN = 'CooZmkhZjSLxzxm6spkH'  
TOPIC_CLOUD_TELEMETRY = 'v1/gateway/telemetry'

client_local = mqtt.Client(client_id="Python_Local_Logic")
client_cloud = mqtt.Client(client_id="Python_Cloud_Gateway")

# 3. Function to Handle MQTT Events
def on_cloud_connect(client, userdata, flags, rc):
    if rc == 0:
        logger.info("[Cloud] Connected to app.coreiot.io successfully!")
    else:
        logger.error(f"[Cloud] Failed to connect to Cloud (Error Code {rc})")

def on_local_connect(client, userdata, flags, rc):
    if rc == 0:
        logger.info(f"[Local] Rule Engine connected to Local Broker {LOCAL_BROKER}!")
        client.subscribe(TOPIC_SENSOR_IN)
        logger.info(f"[Local] Listening for data on Topic: {TOPIC_SENSOR_IN}")
    else:
        logger.error(f"[Local] Error connecting to Local Broker ({rc})")

def on_local_message(client, userdata, msg):
    """
    This function will:
    1. Parse incoming JSON data from Mạch A.
    2. Apply simple rule logic: If humidity > 60%, send "ON" command to Mạch B; else send "OFF".
    3. Forward the telemetry data to Cloud Gateway in the format expected by ThingsBoard/CoreIOT (JSON with timestamp and values).
    """
    try:
        payload = msg.payload.decode('utf-8')
        data = json.loads(payload)
        logger.info(f"[Local] Received data from Sensor Node: {data}")
        
        # Lấy thông số
        device_id = data.get("id", "Sensor_Node_1") 
        temp = data.get("temp", data.get("temperature"))
        humi = data.get("humi", data.get("humidity"))

        if temp is not None:
            # RULE LOGIC for Node B 
            if float(humi) > 70:
                logger.warning(f"Warning: Humidity > 60 ({humi}%). Sending ON command to node B!")
                cmd_msg = json.dumps({"cmd": "ON"})
            else:
                logger.info(f"Normal ({humi}%). Sending OFF command to node B!")
                cmd_msg = json.dumps({"cmd": "OFF"})
                   
            # Publish command to Local Broker for Node B
            client_local.publish(TOPIC_ACTUATOR_OUT, cmd_msg)
            
            # Encapsulate and FORWARD to THINGSBOARD/COREIOT GATEWAY
            telemetry = {
                device_id: [
                    {
                        "ts": int(time.time() * 1000), 
                        "values": {
                            "temperature": temp, 
                            "humidity": humi
                        }
                    }
                ]
            }
            
            cloud_payload = json.dumps(telemetry)
            if client_cloud.is_connected():
                client_cloud.publish(TOPIC_CLOUD_TELEMETRY, cloud_payload)
                logger.info(f"[Cloud] Pushed data to Dashboard: {cloud_payload}")
            else:
                logger.error("[Cloud] Cannot push data due to lost cloud connection!")

    except Exception as e:
        logger.error(f"Error occurred while processing local message: {e}")

# Broker 
async def start_local_broker():
    """Initialize AMQTT Broker in background"""
    broker = Broker(broker_config)
    await broker.start()
    logger.info("[Broker] MQTT Broker (AMQTT) running at 0.0.0.0:1883")
    
    while True:
        await asyncio.sleep(1)

def start_edge_gateway():
    """Initialize Cloud Client and Local Logic Client concurrently"""
    # Connect to Cloud Broker (app.coreiot.io)
    client_cloud.username_pw_set(GATEWAY_ACCESS_TOKEN)
    client_cloud.on_connect = on_cloud_connect
    
    logger.info("[Cloud] Connecting to Cloud...")
    try:
        client_cloud.connect(CLOUD_HOST, CLOUD_PORT, 60)
        client_cloud.loop_start()
    except Exception as e:
        logger.error(f"[Cloud] Failed to connect to Cloud Broker: {e}")

    # Connect to Local Broker (AMQTT)
    client_local.on_connect = on_local_connect
    client_local.on_message = on_local_message
    
    logger.info("[Local] Connecting to Local Broker...")
    try:
        # Trong trường hợp script và broker chạy trên cùng 1 máy thì sẽ connect localhost nhanh chóng
        # Cố định localhost cho Rule Engine vì script chạy cùng máy với Broker
        client_local.connect("127.0.0.1", LOCAL_PORT, 60)
        client_local.loop_start()
    except Exception as e:
        logger.error(f"[Local] Failed to connect to Local Broker: {e}")

import threading

if __name__ == '__main__':
    try:
        print("\n" + "="*50)
        print("Initializing IOT EDGE GATEWAY")
        print("="*50 + "\n")
        
        # Initialize Local Broker 
        def run_broker_background():
            asyncio.run(start_local_broker())
            
        broker_thread = threading.Thread(target=run_broker_background)
        broker_thread.daemon = True 
        broker_thread.start()
        
        logger.info("[Broker] Waiting for Broker initialization...")
        time.sleep(2)
        
        start_edge_gateway()
        
        while True:
            time.sleep(1)
        
    except KeyboardInterrupt:
        logger.info("[Main] System Gateway has been stopped by the user.")
        client_local.loop_stop()
        client_cloud.loop_stop()
        client_local.disconnect()
        client_cloud.disconnect()
