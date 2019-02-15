#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h> 

const char* ssid = "Marble Maze";
const char* password = "letsplay";
String device_Name = "Marble Maze";

WiFiUDP Udp;
unsigned int localUdpPort = 4210;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets
char  replyPacekt[] = "Hi there! Got the message :-)";  // a reply string to send back
String message;

Servo myservoX;  // create servo object to control a servo 
Servo myservoY;  // create servo object to control a servo 
void setup()
    { 
      myservoX.attach(0);//3
      myservoY.attach(5);//1 
      Serial.begin(115200);
      Serial.println();
      
      WiFi.mode(WIFI_AP);
      // Serial.printf("Connecting to %s ", ssid);
      WiFi.softAP(ssid, password); 
    
      Udp.begin(localUdpPort);
      Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.softAPIP().toString().c_str(), localUdpPort);
      //Serial.println("IP address: ");
      //Serial.println(WiFi.softAPIP());

      myservoX.write(90);
      myservoX.write(90);
    }


void loop()
    {    
      int packetSize = Udp.parsePacket();
      if (packetSize)
         {
           //Serial.print("#");
           // receive incoming UDP packets
           //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
           int len = Udp.read(incomingPacket, 255);
           if (len > 0)
              {
                incomingPacket[len] = 0;
              }
           message = incomingPacket;
          // Serial.printf("%s\n", incomingPacket);
      }

      String msg = String(incomingPacket);
      if (msg.startsWith("x")) {
       String xAngle = msg.substring(1);
       int x = xAngle.toInt();
       if(x >= -45 && x <= 45){
       x = map(x,-45,45,135,45);
       //Serial.println(x);
       myservoX.write(x);
       }
      }
      if (msg.startsWith("y")) {
       String yAngle = msg.substring(1);
       int y = yAngle.toInt();
       if(y >= -45 && y <= 45){
       y = map(y,-45,45,135,45);
       //Serial.println(y);
       myservoY.write(y);
       }
      }
    }

