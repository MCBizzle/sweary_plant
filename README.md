# sweary_plant

This repository contains the source code for a sentient plant sensor node based on the Node MCU arduino compatible wifi board. 

Attached to the board are a capacative soil moisture sensor, a DHT22 Temperature and Humidity sensor as well as a pair of 0.96" oled displays that can be configured as eyes to convey the emotions of the plant. Wifi connectivity allows sensor data to be published to an MQTT server for access on a mobile device.

# Board Details

Board is the NodeMCU esp8266 wifi board.

add the following to "additional board managers URL" in arduino IDE preferences panel... http://arduino.esp8266.com/stable/package_esp8266com_index.json

This allows you to install the "esp8266" library (version 2.4.2) from "ESP8266 Community" using the board managers dialog in the arduino IDE.

Flash settings: 

Board: NodeMCU 1.0 (ESP-12E Module)
port: COM port with the arduino attached
Others:default

NOTE: If the arduino is not appearing as a COM port,
1) check the USB cable is both data and power (some just do power).
2) most arduino clones from china use the cheaper CH340 USB-serial chipset. The drivers for this chipset are often not included in your operating system and can be downloaded from here...
https://sparks.gogo.co.nz/ch340.html

