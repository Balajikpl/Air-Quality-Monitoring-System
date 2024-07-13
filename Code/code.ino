#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
SoftwareSerial ESP8266(2, 3); // Rx,  Tx
LiquidCrystal_I2C lcd(0x27, 16, 2);
  
long writingTimer = 17; 
long startTime = 0;
long waitTime = 0;

int buz = 8;
int led1 = 9;
int led2 = 10;
int led3 = 11;
int ppm = 0;
int aql = 0;
const int aqs = A0;
unsigned char check_connection=0;
unsigned char times_check=0;
boolean error;

String myAPIkey = "4DJ1BJL9F38UB7ZQ";  //Your Write API Key from Thingsspeak

void setup()
{
  pinMode (buz, OUTPUT);
  pinMode (led1, OUTPUT);
  pinMode (led2, OUTPUT);
  pinMode (led3, OUTPUT);
  pinMode (aqs, INPUT);

  lcd.init();
  lcd.backlight();
  Serial.begin(9600); 
  ESP8266.begin(9600); 
  startTime = millis(); 
  delay(2000);
  Serial.println("Connecting to Wifi");
   while(check_connection==0)
  {
    Serial.print(".");
  ESP8266.print("AT+CWJAP=\"Balaji\",\"12345678\"\r\n");
  ESP8266.setTimeout(5000);
 if(ESP8266.find("WIFI CONNECTED\r\n")==1)
 {
 Serial.println("WIFI CONNECTED");
 break;
 }
 times_check++;
 if(times_check>3) 
 {
  times_check=0;
   Serial.println("Trying to Reconnect..");
  }
  }
}

void loop()
{
  ppm = analogRead(aqs);
  lcd.setCursor(0, 0);
  lcd.print("Air Quality :");
  lcd.println(ppm);
  delay(1000);
  waitTime = millis()-startTime;   
  if (waitTime > (writingTimer*1000)) 
  {
    writeThingSpeak();
    startTime = millis();   
  }
  if (ppm>300){
    tone(1000,200);
    digitalWrite(buz, HIGH);
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    lcd.setCursor(1, 1);
    lcd.print("Polluted !");
  }
  else if (ppm>200){
    digitalWrite(buz, LOW);
    digitalWrite(led2, HIGH);
    digitalWrite(led1, LOW);
    digitalWrite(led3, LOW);
    lcd.setCursor(1, 1);
    lcd.print("Average  !");
  }
  else{
    digitalWrite(buz, LOW);
    digitalWrite(led3, HIGH);
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    lcd.setCursor(1, 1);
    lcd.print("Good Air !");
  }
}

void writeThingSpeak(void)
{
  startThingSpeakCmd();
  String getStr = "GET /update?api_key=";
  getStr += myAPIkey;
  getStr +="&field1=";
  getStr += String(ppm);
  getStr += "\r\n\r\n";
  GetThingspeakcmd(getStr); 
}

void startThingSpeakCmd(void)
{
  ESP8266.flush();
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com IP address
  cmd += "\",80";
  ESP8266.println(cmd);
  Serial.print("Start Commands: ");
  Serial.println(cmd);

  if(ESP8266.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }
}

String GetThingspeakcmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ESP8266.println(cmd);
  Serial.println(cmd);

  if(ESP8266.find(">"))
  {
    ESP8266.print(getStr);
    Serial.println(getStr);
    delay(500);
    String messageBody = "";
    while (ESP8266.available()) 
    {
      String line = ESP8266.readStringUntil('\n');
      if (line.length() == 1) 
      { 
        messageBody = ESP8266.readStringUntil('\n');
      }
    }
    Serial.print("MessageBody received: ");
    Serial.println(messageBody);
    return messageBody;
  }
  else
  {
    ESP8266.println("AT+CIPCLOSE");     
    Serial.println("AT+CIPCLOSE"); 
  } 
}
