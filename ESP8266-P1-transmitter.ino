/*
My smart meter:

DSMR 4.2
115200 8N1:
Baudrate = 115200
Data bits = 8
Parity = None
Stop bits = 1

inverted 5V uart
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>

//put settings in config.h in the same dir as the sketch or inline:
#include "config.h"
/*
const char* ssid = "Your SSID";
const char* password = "Your pass";
IPAddress mqttserver(127,0,0,1); // fill in your MQTT server
IPAddress influxdb(127,0,0,1); //influxdb server ip
unsigned int influxport = 8888;
*/

WiFiClient wclient;
PubSubClient client(wclient, mqttserver);
int incomingByte = 0;
String inputString;

void setup() {
	Serial.begin (115200);
	WiFi.begin (ssid,password);
	Serial.println("");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay (500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
}

void loop () {
   if (WiFi.status() == WL_CONNECTED) {
      if (!client.connected()) {
         if (client.connect("ESP8266-slimmemeter")) {
            Serial.println("mqtt connected");
            client.publish("/sensor/p1/startup", "Hello from the ESP8266 connected to the smart meter");
         }
    }
    if (client.connected()) {
      CheckSerial();
      client.loop();
    }
  }
}

void SendInflux(String message)
{
   WiFiUDP UDP;
   UDP.beginPacket(influxdb, influxport);
   UDP.print(message);
   UDP.endPacket();
}

void CheckSerial(){
   while (Serial.available() > 0) {
    incomingByte = Serial.read();
    char inChar = (char)incomingByte;
    inputString += inChar; 
   }
   //each telegrams ends with an exclammation mark
   if (inputString.endsWith("!")) {
      //Publish entire unparsed string to MQTT for external parsing or debugging
      client.publish("/sensor/p1/raw",inputString);

      // Extract substrings/values
      int T1_pos = inputString.indexOf("1-0:1.8.1", 0);
      String T1 = inputString.substring(T1_pos + 10, T1_pos + 20);
      client.publish("/sensor/p1/T1",T1);
      SendInflux("T1 value=" + T1);

      int T2_pos = inputString.indexOf("1-0:1.8.2", T1_pos + 1);
      String T2 = inputString.substring(T2_pos + 10, T2_pos + 20);
      client.publish("/sensor/p1/T2",T2);
      SendInflux("T2 value=" + T2);
      
      int T7_pos = inputString.indexOf("1-0:2.8.1", T1_pos + 1);
      String T7 = inputString.substring(T7_pos + 10, T7_pos + 20);
      client.publish("/sensor/p1/T7",T7);
      SendInflux("T7 value=" + T7);

      int T8_pos = inputString.indexOf("1-0:2.8.2", T7_pos + 1);
      String T8 = inputString.substring(T8_pos + 10, T8_pos + 20);
      client.publish("/sensor/p1/T8",T8);
      SendInflux("T8 value=" + T8);

      int P1_pos = inputString.indexOf("1-0:1.7.0", T8_pos + 1);
      String P1 = inputString.substring(P1_pos + 10, P1_pos + 16);
      client.publish("/sensor/p1/P1",P1);
      SendInflux("P1 value=" + P1);

      int P2_pos = inputString.indexOf("1-0:2.7.0", P1_pos + 1);
      String P2 = inputString.substring(P2_pos + 10, P2_pos + 16);
      client.publish("/sensor/p1/P2",P2);
      SendInflux("P2 value=" + P2);

      int G1_pos = inputString.indexOf("0-1:24.2.1", P2_pos + 1);
      String G1 = inputString.substring(G1_pos + 26, G1_pos + 35);
      client.publish("/sensor/p1/G1",G1); 
      SendInflux("G1 value=" + G1);

      inputString = "";
   }
}