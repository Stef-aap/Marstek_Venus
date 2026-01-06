# ESP32  Marstek Venus E v3 Monitor

Simpel monitor for the Marstek Venus with the followig features:
- communicates with Marstek over local Wifi through UDP: Marstek_UDP_vxx.ino
- communicates with Marstek over BLE: Marstek_BLE_vxx.ino
- reads actual (and some aggregating) data from the Marstek
- presents the values on a webpage
- sends the values to an MQTT broker
- supports Home Assistant discovery and sends data to Home Assitant
- doesn't block the Marstek for other devices
- tested on ESP32 and ESP32C3

### You should make the following changes:
- adapt Wifi credentials
- set the correct Marstek IP address.
- If you need MQTT you should also set the correct IP-address of the MQTT-broker.
- If you need Home Assistant support you should unquote //#define HomeAssistant
- If you need more debug information, unquote //#define Debug

<img width="600" alt="screen_20260106_115032" src="https://github.com/user-attachments/assets/e2c0a146-f02d-4aca-98e4-ea5ea1f83ae0" />
