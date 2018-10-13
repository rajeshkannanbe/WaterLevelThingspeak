//nodeMCU v1.0 (black) with Arduino IDE
//stream temperature data DS18B20 with 1wire on ESP8266 ESP12-E (nodeMCU v1.0)
//shin-ajaran.blogspot.com
//nodemcu pinout https://github.com/esp8266/Arduino/issues/584
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

#define MOTOR_AUTO_ON 0
#define MOTOR_CONTROL_SWITCH 0
//Def
#define myPeriodic 3//in sec | Thingspeak pub is 15sec
#define ONE_WIRE_BUS 2  // DS18B20 on arduino pin2 corresponds to D4 on physical board

int prev_motorstatus = -1;
int prev_waterstatus = -2;
int wifiStatus=0;
int internetStatus=0;
const char* server = "api.thingspeak.com";
const char * myWriteAPIKey = "NJY2D2P1IJ348UD2";
unsigned long myChannelNumber = 255487;
const char* MY_SSID = "inwavebala"; 
const char* MY_PWD = "bala@123";

#define WATER_LEVEL_HIGH 5
#define WATER_LEVEL_MEDIUM  3
#define WATER_LEVEL_LOW 4

#define LED_HIGH_PIN 15
#define LED_MEDIUM_PIN 3
#define LED_LOW_PIN 1

byte sensorPin[] = {WATER_LEVEL_LOW, WATER_LEVEL_MEDIUM, WATER_LEVEL_HIGH};
byte ledPin[] = {LED_LOW_PIN, LED_MEDIUM_PIN, LED_HIGH_PIN}; // number of leds = numbers of sensors

const byte sensors = 3;
int level = 0;
int count = 0;
int motor = 14;

typedef struct waterLevelCntl{
   int waterLevel;
   int motorStatus;
};


waterLevelCntl thingspeakWaterLevel;
WiFiClient  client;
int motoron(int on);
int waterlevel(void);
int connectWifi(void);
int sent = 0;
void setup() {
  delay(5000);
  Serial.begin(115200);
  wifiStatus = connectWifi();

  for(int i = 0; i < sensors; i++) {
    pinMode(sensorPin[i], INPUT); 
    digitalWrite(sensorPin[i],LOW);
     //pinMode(ledPin[i], OUTPUT);
 }
  pinMode(12,OUTPUT);
  digitalWrite(12, LOW);
  //pinMode(3,OUTPUT);
  //pinMode(15,OUTPUT);
  //pinMode(1,OUTPUT);
  //digitalWrite(3, LOW);
  //digitalWrite(1, LOW);
  //digitalWrite(15, LOW);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, LOW); 
  waterlevel();
  if(wifiStatus){
    ThingSpeak.setField(1,thingspeakWaterLevel.motorStatus);
    ThingSpeak.setField(2,thingspeakWaterLevel.waterLevel);
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  
  }
}

void loop() {
  static int loopCount = 0;
  static int storemotor;// = ThingSpeak.readFloatField(myChannelNumber,1);
  static int i = 0;
  static int wifiDisconnectCount=0;
  //Serial.println("Motor status1:: ");
  //Serial.println(storemotor);
  
  if(storemotor != thingspeakWaterLevel.motorStatus)
  {
#if(MOTOR_CONTROL_SWITCH == 0)
    motoron(storemotor);
#endif    
    i =100;
  }
  
  waterlevel();
  //storemotor = digitalRead(motor);
  Serial.println("WaterLevel:: ");
  Serial.println(thingspeakWaterLevel.waterLevel);
  Serial.println("Motor status2:: ");
  Serial.println(thingspeakWaterLevel.motorStatus);
  if(!(i % 100)||(prev_motorstatus != thingspeakWaterLevel.motorStatus)||(prev_waterstatus != thingspeakWaterLevel.waterLevel)) 
  {
    if(wifiStatus){
    prev_motorstatus = thingspeakWaterLevel.motorStatus;
    ThingSpeak.setField(1,thingspeakWaterLevel.motorStatus);
    i = 0;
    Serial.println("Send Thinkspeak cloud\r\n");
    prev_waterstatus = thingspeakWaterLevel.waterLevel;
    ThingSpeak.setField(2,thingspeakWaterLevel.waterLevel);
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); 
    }
  } 
  //storemotor = digitalRead(motor);

  int count = myPeriodic;
  i++;
  delay(1000);
  while(count--){
  //delay(1000);
  if(wifiStatus){
	  int resultCode,val;
	  val = ThingSpeak.readFloatField(myChannelNumber,1);
    resultCode = ThingSpeak.getLastReadStatus();
    if(resultCode == 200)
	    storemotor=val;
     else
      Serial.println("internet is not avilable\r\n");
  }

  //Serial.println(storemotor);
  if(storemotor != thingspeakWaterLevel.motorStatus)
    break;
  }
  Serial.println("count ");
  Serial.println(i);
  if(!wifiStatus){
    wifiDisconnectCount++;
    if(wifiDisconnectCount==60)
    {
      wifiStatus=connectWifi();
      wifiDisconnectCount = 0;
    }
   }
    delay(1000);
}

int connectWifi()
{
  int countDelay = 0;
  Serial.print("Connecting to "+*MY_SSID);
  WiFi.begin(MY_SSID, MY_PWD);
  while ((WiFi.status() != WL_CONNECTED) && (countDelay < 50)) {
  delay(1000);
  Serial.print(".");
  countDelay++;
  }
  if(countDelay==10){
    Serial.print("Failed connect WiFi UserName "+*MY_SSID);
    Serial.print("Failed connect WiFi Password "+*MY_PWD);
    return 0;
  }
  Serial.println("");
  Serial.println("Connected");
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  ThingSpeak.begin(client);  
  return 1;
}//end connect


int waterlevel()
{

  level = 0;
  for(int i = 0; i < sensors; i++) {
     if(digitalRead(sensorPin[i]) == HIGH) {
        //digitalWrite(ledPin[i], HIGH);
        Serial.println(sensorPin[i]);
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
     case 7:
       thingspeakWaterLevel.waterLevel = 3;
       if(digitalRead(motor)){
        Serial.println("motor off tank full\r\n");
#if (MOTOR_CONTROL_SWITCH == 0)

        digitalWrite(motor, LOW);
        digitalWrite(15,LOW);
        thingspeakWaterLevel.motorStatus = 0;
#endif        
       }
       else{
         }
       break;
     case 0:
        thingspeakWaterLevel.waterLevel = 0;
        if(digitalRead(motor)){
        
         }
       else{
#if (MOTOR_CONTROL_SWITCH == 0)        
#if MOTOR_CONTROL_AUTO
          digitalWrite(15,HIGH);
          digitalWrite(motor, HIGH);
          thingspeakWaterLevel.motorStatus = 1;
#endif          
#endif          
         } 
      break;
     default:
      Serial.println("motor off defalut\r\n");
      thingspeakWaterLevel.waterLevel = -1;
      if(digitalRead(motor)){
#if (MOTOR_CONTROL_SWITCH == 0)        
        digitalWrite(motor, LOW);
        digitalWrite(15,LOW);
        thingspeakWaterLevel.motorStatus = 0;
#endif        
       }
       break;
  }
  return 1;
}

int motoron(int on)
{
  
  level = 0;
  Serial.println("moto on function enter\r\n");
  if(!on)
  {
    digitalWrite(motor, LOW);
    digitalWrite(15,LOW);
    return 0;
  }
  
  for(int i = 0; i < sensors; i++) {
     if(digitalRead(sensorPin[i]) == HIGH) {
          level |= (1<< i);
          
     } else {
  
     }    
  }
  if(level == 7)
  {
    Serial.print("motor off  \r\n");
    if(digitalRead(motor)){
      digitalWrite(motor, LOW);
    digitalWrite(15, LOW);
    }
    return 0;
  }
  else{
      digitalWrite(motor, HIGH);
      Serial.print("motor on  \r\n");
  }

  return 1;
}

