#include <SoftwareSerial.h>
#include <math.h>
#include <QueueList.h>

  SoftwareSerial BTSerial(4, 5);
  byte inBuffer[128];
  int inBufferPosition;
  
  byte outBuffer[128];
  int outBufferPosition;
  
  boolean temp = false;
  boolean scanning = false;

  QueueList<int> queue;

  int save[5];
  int loc;
  
void setup(){
  BTSerial.begin(19200); 
  Serial.begin(19200); 
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  Serial.println("Hello!");
  
  inBufferPosition = 0;
  outBufferPosition = 0;

  for(int i = 0; i < 5; i++){
    save[i] = 0;
  }
  loc = 0;
}

void loop(){
  if (BTSerial.available()){ // receive from ble    
    char c = BTSerial.read();
    
    if(c == '\r'){
        inBuffer[inBufferPosition] = 0;
        
        String message = inBuffer;
        message.trim();
        message.toLowerCase();
        if(message.equals("")){
          return;
        }
        else{
          if(scanning == true){
            if(message.equals("ready")){
              Serial.println(message);
              scanning = false;
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
                  Serial.print("NEAR!! @ ");
                }
                Serial.println(distance);
              }
            }
          }
          else{
            Serial.println(message);
            if(message.equals("scanning")){
              scanning = true;
            }
          }
        }
        inBufferPosition = 0;
        inBuffer[0] = 0;
    }
    else{
      inBuffer[inBufferPosition] = c;
      inBufferPosition ++;
    }
  }
  if(Serial.available()){ // command to ble
    char c = Serial.read();
    if(c == '\r'){
        outBuffer[outBufferPosition] = 0;
        String message = outBuffer;
        message.toLowerCase();
        
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
/*
String* split(String buffer, char seperator, int num){
  String returnStr[num];
  int loc = 0;
  int oldLoc = 0;
  for(int i = 0; i < num; i++){
    if((loc = buffer.indexOf(seperator, oldLoc)) == -1){
      loc = buffer.length();
    }
    returnStr[i] = buffer.substring(oldLoc, loc);
    Serial.println(returnStr[i]);
    oldLoc = loc + 1;
  }
  return returnStr;
}
*/
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
/*
float averageDistance(QueueList<int> q){
  float avg;
  int cnt = q.count();
  for(int i = 0; i < cnt; i++){
    int pop = q.pop();
    avg += pop;
    q.push(pop);
  }
  avg /= cnt;
  return avg;
}
*/
float averageDistanceArray(int s[]){
  float avg;
  
  for(int i = 0; i < 5; i++){
    avg += s[i];
  }
  avg /= 5;
  return avg;
}
