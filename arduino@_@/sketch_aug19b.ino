#include <SoftwareSerial.h>

  SoftwareSerial BTSerial(4, 5);
  byte inBuffer[128];
  int inBufferPosition;
  
  byte outBuffer[128];
  int outBufferPosition;
  
  boolean temp = false;
  boolean scanning = false;
  
void setup(){
  BTSerial.begin(19200); 
  Serial.begin(19200); 
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Serial.println("Hello!");
  inBufferPosition = 0;
  outBufferPosition = 0;
}

void loop(){
  if (BTSerial.available()){ // receive from ble
    //Serial.write(BTSerial.read()); 
    char c = BTSerial.read();
    if(c == '\r'){
        inBuffer[inBufferPosition] = 0;
        String message = inBuffer;
        message.toLowerCase();

        if(scanning == true){
          String UUID = getUUID(message, ',');
          UUID.trim();
          String rssi = getRSSI(message, ',');
          //Serial.print(rssi);
          if(UUID.equals("001901b0018c")){
            Serial.println("FIND");
          }
        }
        else{
          //Serial.write(inBuffer, inBufferPosition);
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

        if(scanning == true){
          if(message.equals("at+reqscan0")){
            scanning = false;
          }
        }
        else{
          if(message.equals("at+reqscan1")){
            scanning = true;
          }
          //Serial.write(outBuffer, outBufferPosition);
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
  if(temp == false){
    temp = true;
    BTSerial.write("AT\r");
  }
}
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
