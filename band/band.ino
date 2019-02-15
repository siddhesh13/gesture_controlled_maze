#include "Wire.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

extern "C" {
  #include "user_interface.h"
}

const char* ssid = "Marble Maze";
const char* password = "letsplay";
IPAddress ipBroadCast(192, 168, 4, 1);
unsigned int udpRemotePort=4210;
unsigned int udplocalPort=4222;
const int UDP_PACKET_SIZE = 48;
char udpBuffer[ UDP_PACKET_SIZE];
WiFiUDP Udp;


const int MPU_addr=0x68; // I2C address of the MPU-6050
 
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
long gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ;
float gyroX_cal, gyroY_cal, gyroZ_cal;
float angle_pitch, angle_roll;
float angle_roll_acc, angle_pitch_acc;
float angle_pitch_output, angle_roll_output;
long loop_timer;
int count = 0; 
void setup() {
Wire.begin(0,2);
Serial.begin(115200);
 
check_I2c(MPU_addr); // Check that there is an MPU
 
Wire.beginTransmission(MPU_addr);
Wire.write(0x6B); // PWR_MGMT_1 register
Wire.write(0); // set to zero (wakes up the MPU-6050)
Wire.endTransmission(true);
//Wire.begin();
  setupMPU();
  delay(1000);
  Serial.println("caliberating MPU6050");
  for(int i=0; i<2000; i++){
    if(i %125 == 0) Serial.print(".");
    recordAccelReg();
    gyroX_cal += GyX;
    gyroY_cal += GyY;
    gyroZ_cal += GyZ;
    delay(3);
  }
  gyroX_cal /= 2000;
  gyroY_cal /= 2000;
  gyroZ_cal /= 2000;
  Serial.print("gyroX_cal: ");
  Serial.print(gyroX_cal);
  Serial.print("  gyroY_cal: ");
  Serial.print(gyroY_cal);
  Serial.print("  gyroZ_cal: ");
  Serial.print(gyroZ_cal);
  delay(1000);
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  Udp.begin(udplocalPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), udplocalPort);

  loop_timer = micros();
}
 
void loop() {
 
recordAccelReg();
  
  GyX -= gyroX_cal;
  GyY -= gyroY_cal;
  GyZ -= gyroZ_cal;
  //printData();
  angle_pitch += GyX * 0.0000611;
  angle_roll += GyY * 0.0000611;

   //0.000001066 = 0.0000611 * (3.142(PI) / 180degr) The Arduino sin function is in radians
  angle_pitch += angle_roll * sin(GyZ * 0.000001066);               //If the IMU has yawed transfer the roll angle to the pitch angel
  angle_roll -= angle_pitch * sin(GyZ * 0.000001066);               //If the IMU has yawed transfer the pitch angle to the roll angel
  int xAngle = angle_roll;
  int yAngle = angle_pitch;
  // Serial.print("X: ");
 // Serial.println(angle_pitch);
  //Serial.print("  ");
 // Serial.println(angle_roll);
 /* if(millis() - previousMillis  > 1000){
    Serial.print("count: ");
    Serial.println(count);
    count =0;
    previousMillis = millis(); 
   }*/
   //delay(2);

   while(micros() - loop_timer < 4000);{
    // send back a reply, to the IP address and port we got the packet from
    //Serial.println(angle_pitch);
   // Serial.println(angle_roll);
    if(count == 1){
     // if(xAngle >= -45 && xAngle <= 45){
      String value = "x" + String(xAngle);
      value.toCharArray(udpBuffer,UDP_PACKET_SIZE);
      //Serial.println(udpBuffer); 
      Udp.beginPacket(ipBroadCast, udpRemotePort);
      Udp.write(udpBuffer, sizeof(udpBuffer));
      Udp.endPacket();
    // }
    }
     
    if(count == 2){
     //if(yAngle >= -45 && yAngle <= 45){
      String value = "y" + String(yAngle);
      value.toCharArray(udpBuffer,UDP_PACKET_SIZE);
      //Serial.println(udpBuffer); 
      Udp.beginPacket(ipBroadCast, udpRemotePort);
      Udp.write(udpBuffer, sizeof(udpBuffer));
      Udp.endPacket();
      count = 0;
     }
   // }
   }   
   loop_timer = micros();
   count++;
}
 
byte check_I2c(byte addr){
// We are using the return value of
// the Write.endTransmisstion to see if
// a device did acknowledge to the address.
byte error;
Wire.beginTransmission(addr);
error = Wire.endTransmission();
 
if (error == 0)
{
Serial.print(" Device Found at 0x");
Serial.println(addr,HEX);
}
else
{
Serial.print(" No Device Found at 0x");
Serial.println(addr,HEX);
}
return error;
}

void recordAccelReg(){
  Wire.beginTransmission(MPU_addr);
Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
Wire.endTransmission(false);
Wire.requestFrom(MPU_addr,14,true); // request a total of 14 registers
AcX=Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
AcY=Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
AcZ=Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
Tmp=Wire.read()<<8|Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
GyX=Wire.read()<<8|Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
GyY=Wire.read()<<8|Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
GyZ=Wire.read()<<8|Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

}

void setupMPU(){
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0b00000000);
  Wire.endTransmission();
  Wire.beginTransmission(0b1101000);
  Wire.write(0x1B);
  Wire.write(0x08);
  Wire.endTransmission();
  Wire.beginTransmission(0b1101000);
  Wire.write(0x1C);
  Wire.write(0x10);
  Wire.endTransmission();
}
