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
#include <PubSubClient.h>

//put settings in config.h in the same dir as the sketch

#include "config.h"
#define BUFFER_SIZE 100

WiFiClient wclient;
PubSubClient client(wclient, mqttserver);

char lastmessage_recvd[100];
String lastmessage;

char count = 0;
int incomingByte = 0;
int T1_pos;
int T2_pos;
int T7_pos;
int T8_pos;
int P1_pos;
int P2_pos;
int P3_pos;
int G1_pos;
String inputString;
String T1;
String T2;
String T7;
String T8;
String P1;
String P2;
String P3;
String G1;
String outputString;
String lastString;

const int led = 13;

void setup ( void ) {
	pinMode ( led, OUTPUT );
	digitalWrite ( led, 0 );
	Serial.begin ( 115200 );
	WiFi.begin ( ssid, password );
	Serial.println ( "" );

	// Wait for connection
	while ( WiFi.status() != WL_CONNECTED ) {
		delay ( 500 );
		Serial.print ( "." );
	}
	Serial.println ( "" );
	Serial.print ( "Connected to " );
	Serial.println ( ssid );
	Serial.print ( "IP address: " );
	Serial.println ( WiFi.localIP() );
}

void loop () {
   if (WiFi.status() == WL_CONNECTED) {
      if (!client.connected()) {
         if (client.connect("slimmemeter")) {
            Serial.println("mqtt connected");
            client.publish("/test/P1", "Hello from the ESP8266 connected to the smart meter");
         }
    }
    if (client.connected()) {
      CheckSerial();      
      client.loop();    
    }
  }
}

void CheckSerial(){
  while (Serial.available() > 0) {
    incomingByte = Serial.read();    
    char inChar = (char)incomingByte;
    inputString += inChar; 
   }
   //each telegrams ends with an exclammation mark, so start proccessing the telegram:
   if (inputString.endsWith("!")) {
      lastString = inputString;
      //Publish entire unparsed string to MQTT for external parsing or debugging
      client.publish("/test/P1_string",inputString);

      // Extract substings/values
      T1_pos = inputString.indexOf("1-0:1.8.1", 0);
      T1 = inputString.substring(T1_pos + 10, T1_pos + 20);
 
      T2_pos = inputString.indexOf("1-0:1.8.2", T1_pos + 1);
      T2 = inputString.substring(T2_pos + 10, T2_pos + 20);
      
      T7_pos = inputString.indexOf("1-0:2.8.1", T1_pos + 1);
      T7 = inputString.substring(T7_pos + 10, T7_pos + 20);
      
      T8_pos = inputString.indexOf("1-0:2.8.2", T7_pos + 1);
      T8 = inputString.substring(T8_pos + 10, T8_pos + 20);
      
      P1_pos = inputString.indexOf("1-0:1.7.0", T8_pos + 1);
      P1 = inputString.substring(P1_pos + 10, P1_pos + 16);
      
      P2_pos = inputString.indexOf("1-0:2.7.0", P1_pos + 1);
      P2 = inputString.substring(P2_pos + 10, P2_pos + 16);

      G1_pos = inputString.indexOf("0-1:24.2.1", P2_pos + 1);
      G1 = inputString.substring(G1_pos + 26, G1_pos + 35);

      // generate outputring
      outputString = "{T1:" + T1 + ",T2:" + T2 + ",T7:" + T7 + ",T8:" + T8 + ",P1:" + P1 + ",P2:" + P2  + ",G1:" + G1 + "}";
      client.publish("/test/P1",outputString);
      client.publish("/sensor/p1/T1",T1);
      client.publish("/sensor/p1/T2",T2);
      client.publish("/sensor/p1/T7",T7);
      client.publish("/sensor/p1/T8",T8);
      client.publish("/sensor/p1/P1",P1);
      client.publish("/sensor/p1/P2",P2);
      client.publish("/sensor/p1/G1",G1);
      inputString = "";
   } 
}