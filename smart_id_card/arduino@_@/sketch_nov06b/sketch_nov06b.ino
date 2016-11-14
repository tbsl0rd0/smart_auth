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

  int initLevel = 0;

  int save[5];
  int saveLoc;

  Timer t;
  int timerID;

void setup() {
  BTSerial.begin(19200); 
  Serial.begin(19200);  

  inBufferPosition = 0;
  outBufferPosition = 0;

  resetSaves();

  timerID = t.every(256, initAT);
}

void loop() {
  t.update();
  if (BTSerial.available()){  // receive FROM BLE    
    char c = BTSerial.read();

    if(c == '\r'){
      inBuffer[inBufferPosition] = 0;
      String message = (char*)inBuffer;
      message.trim();
      message.toLowerCase();

      if(!message.equals("")){
        // init
        if(initLevel < 3){
          //Serial.println(message);
          switch(initLevel){
          case 0: // AT
            if(message.equals("ok")){
              initLevel++;
            }
            break;
          case 1: // AT+REQSCAN0
            if(message.equals("ready") || message.equals("error")){
              initLevel++;
            }
            break;
          case 2: // AT+REQSCAN1
            if(message.equals("scanning")){
              initLevel++;
              scanning = true;
              t.stop(timerID);
            }
            break;
          }
        }
        else{
          if(scanning == true){
            if(message.equals("ready")){
              scanning = false;
              scanned = false;
              if(timerID >= 0){
                t.stop(timerID);
                timerID = -1;
              }
            }
            else{
              String UUID = getUUID(message, ',');
              String rssi = getRSSI(message, ',');
              UUID.trim();

              char cRssi[7];
              rssi.toCharArray(cRssi, 7);
              int nRssi = -((int)strtol(cRssi, NULL, 16) & 0x7F);
              
              if(UUID.equals("001901b0018c")){
                //Serial.print("find");
                save[saveLoc++] = nRssi;
                if(saveLoc >=5 ){
                  saveLoc = 0;
                }
                float avgRssi = averageDistanceArray(save);
                float distance = calcDistance(avgRssi, -59);
                if(distance <= 2.5){
                   //Serial.print("near : ");
                   //Serial.println(distance);                   
                   scanned = true;
                   if(timerID >= 0){
                      t.stop(timerID);
                   }
                   timerID = t.every(3000, notScanned); 
                }                
              }
            }            
          }
        }      
      }
      inBufferPosition = 0;
      inBuffer[0] = 0;
    }   
    else{ // not \r
      inBuffer[inBufferPosition] = c;
      inBufferPosition ++;
    }
  }
  if(Serial.available()){ // $$$ command TO BLE
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
    // BTSerial.write(c);
  }
}
void resetSaves(){
  for(int i = 0; i < 5; i++){
    save[i] = 0;
    saveLoc = 0;
  }
}
void initAT(){
  switch(initLevel){
  case 0:
    BTSerial.write("at\r");
    break;
  case 1:
    BTSerial.write("at+reqscan0\r");
    break;
  case 2:
    BTSerial.write("at+reqscan1\r");
    break;
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
  for(int i = 0; i < 5; i++){
    save[i] = 0;
  }
}
