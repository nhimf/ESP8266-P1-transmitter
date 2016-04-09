# ESP8266-P1-transmitter
This sketch written in the Arduino language makes an ESP8266 send incoming P1 smartmeter data to MQTT.

I have used this MQTT library for pubsubclient:
https://github.com/Imroy/pubsubclient

Also added in this script is an insert into an influxdb using it's UDP interface.
