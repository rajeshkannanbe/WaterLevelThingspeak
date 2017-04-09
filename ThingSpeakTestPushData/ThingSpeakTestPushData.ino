//nodeMCU v1.0 (black) with Arduino IDE
//stream temperature data DS18B20 with 1wire on ESP8266 ESP12-E (nodeMCU v1.0)
//shin-ajaran.blogspot.com
//nodemcu pinout https://github.com/esp8266/Arduino/issues/584
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

//Def
#define myPeriodic 15 //in sec | Thingspeak pub is 15sec
#define ONE_WIRE_BUS 2  // DS18B20 on arduino pin2 corresponds to D4 on physical board

float prevTemp = 0;
const char* server = "api.thingspeak.com";
const char * myWriteAPIKey = "NJY2D2P1IJ348UD2";
unsigned long myChannelNumber = 255487;
const char* MY_SSID = "moto"; 
const char* MY_PWD = "12345678";

#define WATER_LEVEL_HIGH 14
#define WATER_LEVEL_MEDIUM  12
#define WATER_LEVEL_LOW 13

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
  pinMode(3,OUTPUT);
  pinMode(15,OUTPUT);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, LOW); 
  waterlevel();
  ThingSpeak.setField(1,thingspeakWaterLevel.motorStatus);
  ThingSpeak.setField(2,thingspeakWaterLevel.waterLevel);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  
}

void loop() {
  static float temp = 34.40;
  static int loopCount = 0;
   
  int storemotor = ThingSpeak.readFloatField(myChannelNumber,1);
  Serial.println("Motor status1:: ");
  Serial.println(storemotor);
  if(storemotor != thingspeakWaterLevel.motorStatus)
    motoron(storemotor);
  waterlevel();
  Serial.println("WaterLevel:: ");
  Serial.println(thingspeakWaterLevel.waterLevel);
  Serial.println("Motor status2:: ");
  Serial.println(thingspeakWaterLevel.motorStatus);
  ThingSpeak.setField(1,thingspeakWaterLevel.motorStatus);
  ThingSpeak.setField(2,thingspeakWaterLevel.waterLevel);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  
  int count = myPeriodic;
  while(count--){
  delay(1000);
  storemotor = ThingSpeak.readFloatField(myChannelNumber,1);
  Serial.println("Motor 9 :: ");
  Serial.println(storemotor);
  if(storemotor != thingspeakWaterLevel.motorStatus)
    break;
  }
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
     case 7:
       thingspeakWaterLevel.waterLevel = 3;
       if(digitalRead(motor)){
        
       }
       else{
         }
       break;
     case 0:
        thingspeakWaterLevel.waterLevel = 0;
        if(digitalRead(motor)){
        
         }
       else{
         } 
      break;
     default:
      thingspeakWaterLevel.waterLevel = -1;
      if(digitalRead(motor)){
        //digitalWrite(motor, LOW);
        thingspeakWaterLevel.motorStatus = 0;
       }
       break;
  }
  return 1;
}

int motoron(int on)
{
  
  level = 0;
  if(!on)
  {
    digitalWrite(motor, LOW);
    return 1;
  }
  
  for(int i = 0; i < sensors; i++) {
     if(digitalRead(sensorPin[i]) == HIGH) {
          level |= (1<< i);
        
     } else {
  
     }    
  }
  if(level == 7)
  {
    Serial.print("motor on  \r\n");
    digitalWrite(motor, HIGH);
    digitalWrite(LED_HIGH_PIN, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(1, HIGH);
    
    return 0;
  }
  else{
      digitalWrite(motor, HIGH);
  }

  return 1;
}

