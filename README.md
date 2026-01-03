# ESP32  Marstek Venus E v3 Monitor

Simpel monitor for the Marstek Venus with the followig features:
- communicates with Marstek over local Wifi through UDP
- reads actual (and some aggregating) data from the Marstek
- presents the values on a webpage
- sends the values to an MQTT broker
- supports Home Assistant discovery and sends data to Home Assitant
- doesn't block the Marstek for other devices
- tested on ESP32 and ESP32C3

You should adapt Wifi credentials and the Marstek IP address.
If you need MQTT you should also set the correct IP-address of the MQTT-broker.
If you need Home Assistant support you should unquote //#define HomeAssistant
If you need more debug information, unquote //#define Debug

<img width="1018" height="692" alt="screen_20260103_230149" src="https://github.com/user-attachments/assets/689e4e11-153a-434f-822f-635b923cd195" />
