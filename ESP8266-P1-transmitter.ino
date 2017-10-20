/*
Disclaimer: This is a work in progress and might or might not work.

My smart meter:

DSMR 5.0
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
//#include <PubSubClient.h>
#define MAX_SRV_CLIENTS 1


//put settings in config.h in the same dir as the sketch or inline:
#include "config.h"
/*
const char* ssid = "Your SSID";
const char* password = "Your pass";
IPAddress mqttserver(127,0,0,1); // fill in your MQTT server
IPAddress influxdb(127,0,0,1); //influxdb server ip
unsigned int influxport = 8888;
String deviceName = "SmartMeter";
String testTelegram = "";
*/

WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];

//WiFiClient wclient;
// PubSubClient client(wclient, mqttserver);
int incomingByte = 0;
String telegram;

void setup() {
	Serial.begin (115200);
	WiFi.begin (ssid,password);

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay (500);
	}

}

void loop () {
      CheckSerial();
      delay(1000);
}

void SendInflux(String message)
{
   WiFiUDP UDP;
   UDP.beginPacket(influxdb, influxport);
   UDP.print(message);
   UDP.endPacket();
}

void SendDebug(String message)
{
   WiFiUDP UDP;
   UDP.beginPacket(influxdb, influxport+1);
   UDP.print(message);
   UDP.endPacket();
}
void CheckSerial(){
      bool beginOfTelegramFound = false;
      bool endOfTelegramFound = false;
      bool fullTelegramFetched = false;
      char sbuf[2048];
      int i = 0;
      char crc[4];
      String rawData = "";

  // Read a full telegram
  while ( !fullTelegramFetched )
  {
    if( Serial.available() )
    {
      char readChar = (char)Serial.read();
      if ( readChar == '/' )
      {
        beginOfTelegramFound = true;
        endOfTelegramFound = false;
        telegram = "!";
        continue;
      }
      if ( readChar == '!' )
      {
        endOfTelegramFound = true;
        i = 0;
        continue;
      }
      if ( beginOfTelegramFound && !endOfTelegramFound )
      {
        telegram += readChar;
        continue;
      }
      if (endOfTelegramFound && i < 4)
      {
        crc[i] = readChar;
        i++;
        continue;
      }
      if (endOfTelegramFound && i >= 4)
      {
         fullTelegramFetched = true;
      }
    }
  }

   SendDebug(telegram);
   

   // Okay, no more data, but we haven't fetched a full frame. As we are missing data for sure, break.
   if ( !fullTelegramFetched )
   {
    return;
   }

      String udpString = "Power,device=" + deviceName + " debug=1";

      // Extract substrings/values
      // Meter Reading electricity delivered to client (Tariff 1) in 0,001 kWh
      int T1_pos = telegram.indexOf("1-0:1.8.1", 0);
      if (T1_pos >= 0)
      {
        String T1 = telegram.substring(T1_pos + 10, T1_pos + 20);
        udpString += ",T1=" + T1;
      }

      // Meter Reading electricity delivered to client (Tariff 2) in 0,001 kWh
      int T2_pos = telegram.indexOf("1-0:1.8.2", T1_pos + 1);
      if (T2_pos >= 0)
      {
        String T2 = telegram.substring(T2_pos + 10, T2_pos + 20);
        udpString += ",T2=" + T2;
      }

      // Meter Reading electricity delivered by client (Tariff 1) in 0,001 kWh
      int T7_pos = telegram.indexOf("1-0:2.8.1", T1_pos + 1);
      if (T7_pos >= 0)
      {
        String T7 = telegram.substring(T7_pos + 10, T7_pos + 20);
        udpString += ",T7=" + T7;
      }

      // Meter Reading electricity delivered by client (Tariff 2) in 0,001 kWh
      int T8_pos = telegram.indexOf("1-0:2.8.2", T7_pos + 1);
      if (T8_pos >= 0)
      {
        String T8 = telegram.substring(T8_pos + 10, T8_pos + 20);
        udpString += ",T8=" + T8;
      }

      // Actual electricity power delivered (+P) in 1 Watt reso- lution
      int P1_pos = telegram.indexOf("1-0:1.7.0", T8_pos + 1);
      if (P1_pos >= 0)
      {
        String P1 = telegram.substring(P1_pos + 10, P1_pos + 16);
        udpString += ",P1=" + P1;
      }

      // Actual electricity power received (-P) in 1 Watt resolution
      int P2_pos = telegram.indexOf("1-0:2.7.0", P1_pos + 1);
      if (P2_pos >= 0)
      {
        String P2 = telegram.substring(P2_pos + 10, P2_pos + 16);
        udpString += ",P2=" + P2;
      }

      // Last 5-minute value (temperature con- verted), gas deliv- ered to client in m3, including decimal values and capture time
      int G1_pos = telegram.indexOf("0-1:24.2.1", P2_pos + 1);
      if (G1_pos >= 0)
      {
        String G1 = telegram.substring(G1_pos + 26, G1_pos + 35);
        udpString += ",G1=" + G1;
      }
      //SendInflux("Power,device=" + deviceName + " T1=" + T1 + ",T2=" + T2 + ",T7=" + T7 + ",T8=" + T8 + ",P1=" + P1 + ",P2=" + P2 + ",G1=" + G1);
      SendInflux (udpString);
}
