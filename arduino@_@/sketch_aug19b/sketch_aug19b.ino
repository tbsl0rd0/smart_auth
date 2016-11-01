#include <SoftwareSerial.h>
#include <math.h>
#include "Timer.h"

  SoftwareSerial BTSerial(4, 5);
  byte inBuffer[128];
  int inBufferPosition;
  
  byte outBuffer[128];
  int outBufferPosition;
  
  boolean scanned = false;
  boolean scanning = false;
  boolean init1 = false;

  int save[5];
  int loc;

  Timer t;
  int timerID;
  
void setup(){
  BTSerial.begin(19200); 
  Serial.begin(19200);   
  //Serial.println("Hello!");
  
  inBufferPosition = 0;
  outBufferPosition = 0;

  for(int i = 0; i < 5; i++){
    save[i] = 0;
  }
  loc = 0;

  timerID = t.every(500, initAT);
}

void loop(){
  t.update();
  if (BTSerial.available()){ // receive from ble    
    char c = BTSerial.read();
    
    if(c == '\r'){
      
      if(init1 == false){
        inBuffer[inBufferPosition] = 0;
        
        String message = (char*)inBuffer;
        message.trim();
        message.toLowerCase();
        
        if(message.equals("")){
          return;
        }
        else{
          if(message.equals("ok")){
              t.stop(timerID);
              BTSerial.write("at+reqscan0\r");
              //
              //Serial.println("++ at+reqscan0 ++");
              //
              timerID = t.after(100, startScanning);
          }
        }
      }
      else{
        inBuffer[inBufferPosition] = 0;
        
        String message = (char*)inBuffer;
        message.trim();
        message.toLowerCase();
        if(message.equals("")){
          return;
        }
        else{
          if(scanning == true){
            if(message.equals("ready")){
              //
              //Serial.println(message);
              //
              scanning = false;
              scanned = false;
              if(timerID >= 0){
                t.stop(timerID);
                timerID = -1;
              }
            }
            else{
              String UUID = getUUID(message, ',');
              UUID.trim();
              String rssi = getRSSI(message, ',');
    
              char cRssi[7];
              rssi.toCharArray(cRssi, 7);
              int nRssi = -((int)strtol(cRssi, NULL, 16) & 0x7F);
    
              if(UUID.equals("001901b0018c")){
                save[loc++] = nRssi;
                if(loc >=5 ){
                  loc = 0;
                }
                float avgRssi = averageDistanceArray(save);
                
                float distance = calcDistance(avgRssi, -59);
                if(distance <= 4.0){
                  //
                  //Serial.println("NEAR!! @ ");
                  //
                  scanned = true;
                  if(timerID >= 0){
                    t.stop(timerID);
                  }
                  timerID = t.every(3000, notScanned); 
                  //Serial.print("b");
                }
                //Serial.println(distance);
              }
            }
          }
          else{ // if scanning is false
            //Serial.println(message);
            if(message.equals("scanning")){
              scanning = true;
            }
          }
        }
        inBufferPosition = 0;
        inBuffer[0] = 0;
      }
    }
    else{ // not \r
      inBuffer[inBufferPosition] = c;
      inBufferPosition ++;
    }
  }
  if(Serial.available()){ // command to ble
    char c = Serial.read();
    if(c == '\r'){
        outBuffer[outBufferPosition] = 0;
        String message = (char*)outBuffer;
        message.toLowerCase();

        if(message.equals("a")){
          if(scanned == false){
            Serial.write("c");
          }
          else if(scanned == true){
            Serial.write("b");
          }
        }
        
        outBufferPosition = 0;
        outBuffer[0] = 0;
    }
    else{
        outBuffer[outBufferPosition] = c;
        outBufferPosition ++;
    }
    BTSerial.write(c);
  }
}
String getUUID(String buffer, char seperator){
  String UUID;
  int loc = buffer.indexOf(seperator);
  UUID = buffer.substring(0, loc);

  return UUID;
}
String getRSSI(String buffer, char seperator){
  String rssi;
  int loc1 = buffer.indexOf(seperator);
  int loc2 = buffer.indexOf(seperator, loc1 + 1);
  rssi = buffer.substring(loc1 + 1, loc2);

  return rssi;
}
float calcDistance(float txPower, double rssi){
  if(rssi == 0){
    return -1.0;
  }
  float ratio = rssi * 1.0 / txPower;
  if(ratio < 1.0){
    return pow(ratio, 10);
  }
  else{
    float dis = (0.89976) * pow(ratio, 7.7095) + 0.111;
    
    return dis;
  }
}
float averageDistanceArray(int s[]){
  float avg;
  
  for(int i = 0; i < 5; i++){
    avg += s[i];
  }
  avg /= 5;
  return avg;
}
void notScanned(){
  //Serial.println("false!");
  scanned = false;
  t.stop(timerID);
}
void initAT(){
  //
    //Serial.println("initAT");
    //
    BTSerial.write("at\r");
}
void startScanning(){
  //
    //Serial.println("startScanning");
    //
    BTSerial.write("at+reqscan1\r"); 
    init1 = true;
    //
    //Serial.println("++ at+reqscan1 ++"); 
    //
    //init1 = true;
}

