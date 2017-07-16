//nodeMCU v1.0 (black) with Arduino IDE
//stream temperature data DS18B20 with 1wire on ESP8266 ESP12-E (nodeMCU v1.0)
//shin-ajaran.blogspot.com
//nodemcu pinout https://github.com/esp8266/Arduino/issues/584
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

//Def
#define myPeriodic 5//in sec | Thingspeak pub is 15sec
#define ONE_WIRE_BUS 2  // DS18B20 on arduino pin2 corresponds to D4 on physical board

int prev_motorstatus = -1;
const char* server = "api.thingspeak.com";
const char * myWriteAPIKey = "NJY2D2P1IJ348UD2";
unsigned long myChannelNumber = 255487;
const char* MY_SSID = "priya"; 
const char* MY_PWD = "priya@9945";

#define WATER_LEVEL_HIGH 13
#define WATER_LEVEL_MEDIUM  12
#define WATER_LEVEL_LOW 14

#define LED_HIGH_PIN 15
#define LED_MEDIUM_PIN 3
#define LED_LOW_PIN 1

byte sensorPin[] = {WATER_LEVEL_LOW, WATER_LEVEL_MEDIUM, WATER_LEVEL_HIGH};
byte ledPin[] = {LED_LOW_PIN, LED_MEDIUM_PIN, LED_HIGH_PIN}; // number of leds = numbers of sensors

const byte sensors = 3;
int level = 0;
int count = 0;
int motor = 5;

typedef struct waterLevelCntl{
   int waterLevel;
   int motorStatus;
};


waterLevelCntl thingspeakWaterLevel;
WiFiClient  client;
int motoron(int on);
int waterlevel(void);
void connectWifi(void);
int sent = 0;
void setup() {
  Serial.begin(115200);
  connectWifi();

  for(int i = 0; i < sensors; i++) {
    pinMode(sensorPin[i], INPUT); 
    digitalWrite(sensorPin[i],LOW);
     //pinMode(ledPin[i], OUTPUT);
 }
  pinMode(12,OUTPUT);
  digitalWrite(12, LOW);
  pinMode(3,OUTPUT);
  pinMode(15,OUTPUT);
  //pinMode(1,OUTPUT);
  digitalWrite(3, LOW);
  //digitalWrite(1, LOW);
  digitalWrite(15, LOW);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, LOW); 
  waterlevel();
  ThingSpeak.setField(1,thingspeakWaterLevel.motorStatus);
  ThingSpeak.setField(2,thingspeakWaterLevel.waterLevel);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  
}

void loop() {
  static int loopCount = 0;
  static int storemotor;// = ThingSpeak.readFloatField(myChannelNumber,1);
  static int i = 0;
  Serial.println("Motor status1:: ");
  Serial.println(storemotor);
  
  if(storemotor != thingspeakWaterLevel.motorStatus)
    motoron(storemotor);
  
  waterlevel();
  //storemotor = digitalRead(motor);
  Serial.println("WaterLevel:: ");
  Serial.println(thingspeakWaterLevel.waterLevel);
  Serial.println("Motor status2:: ");
  Serial.println(thingspeakWaterLevel.motorStatus);
  if(!(i % 4)||(prev_motorstatus != thingspeakWaterLevel.motorStatus)) 
  {
    prev_motorstatus = thingspeakWaterLevel.motorStatus;
    ThingSpeak.setField(1,thingspeakWaterLevel.motorStatus);
    i = 0;
    Serial.println("Motor status update\r\n");
  }
  ThingSpeak.setField(2,thingspeakWaterLevel.waterLevel);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  
  int count = myPeriodic;
  i++;
  while(count--){
  //delay(1000);
  storemotor = ThingSpeak.readFloatField(myChannelNumber,1);
  Serial.println("Motor 9 :: ");
  Serial.println(storemotor);
  if(storemotor != thingspeakWaterLevel.motorStatus)
    break;
  }
  delay(1000);
}

void connectWifi()
{
  Serial.print("Connecting to "+*MY_SSID);
  WiFi.begin(MY_SSID, MY_PWD);
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("Connected");
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  ThingSpeak.begin(client);  
}//end connect


int waterlevel()
{

  level = 0;
  for(int i = 0; i < sensors; i++) {
     if(digitalRead(sensorPin[i]) == HIGH) {
        //digitalWrite(ledPin[i], HIGH);
        //Serial.println(sensorPin[i]);
        level |= (1<< i);
        
     } else {
       //digitalWrite(ledPin[i], LOW);       
     }    
  }
  
  //Serial.println(level);
  if(digitalRead(motor))
    thingspeakWaterLevel.motorStatus = 1;
  else
    thingspeakWaterLevel.motorStatus = 0;
  switch(level) {
     case 1:
       //LOW
       thingspeakWaterLevel.waterLevel = 1; 
      break;
     case 3:
       thingspeakWaterLevel.waterLevel = 2;
      break;
     case 5:
     case 7:
       thingspeakWaterLevel.waterLevel = 3;
       if(digitalRead(motor)){
        Serial.println("motor off tank full\r\n");
        digitalWrite(motor, LOW);
        digitalWrite(15, LOW);
        thingspeakWaterLevel.motorStatus = 0;
        //digitalWrite(3, LOW);
       }
       else{
         }
       break;
     case 0:
        thingspeakWaterLevel.waterLevel = 0;
        if(digitalRead(motor)){
        
         }
       else{
          digitalWrite(motor, HIGH);
          digitalWrite(15, HIGH);
          //digitalWrite(3, HIGH);
          thingspeakWaterLevel.motorStatus = 1;
         } 
      break;
     default:
      Serial.println("motor off defalut\r\n");
      thingspeakWaterLevel.waterLevel = -1;
      if(digitalRead(motor)){
        digitalWrite(motor, LOW);
        digitalWrite(15, LOW);
        //digitalWrite(3, LOW);
        thingspeakWaterLevel.motorStatus = 0;
       }
       break;
  }
  return 1;
}

int motoron(int on)
{
  
  level = 0;
  Serial.println("motoon function enter\r\n");
  if(!on)
  {
    digitalWrite(motor, LOW);
    digitalWrite(15, LOW);
    //digitalWrite(3, LOW);
    return 0;
  }
  
  for(int i = 0; i < sensors; i++) {
     if(digitalRead(sensorPin[i]) == HIGH) {
          level |= (1<< i);
        
     } else {
  
     }    
  }
  if(level == 7 || level == 5)
  {
    Serial.print("motor off  \r\n");
    if(digitalRead(motor)){
      digitalWrite(motor, LOW);
      digitalWrite(15, LOW);
      //digitalWrite(LED_HIGH_PIN, HIGH);
      //digitalWrite(3, LOW);
      //digitalWrite(1, HIGH);
    }
    return 0;
  }
  else{
      digitalWrite(motor, HIGH);
      digitalWrite(15, HIGH);
      //digitalWrite(3, HIGH);
  }

  return 1;
}

