#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

int pin = 35;
int sensorValue;
int crossed = 0;
int last = 0;

const char* ssid     = "HONOR 9X Lite";              //Main Router      
const char* password = "test0123";          //Main Router Password

const char* url = "http://storm5301.hub.ubeac.io/IK7YUJTHGRBVFDESRCX";
String payload_pattern = "[{\"id\": \"MyESP\", \"sensors\": [{\"id\": \"Laser Crossed\", \"value\": $laser$}]}]";

void setup(){
  Serial.begin(9600);
  pinMode(pin, INPUT);

  delay(4000);   //Delay needed before calling the WiFi.begin
 
  WiFi.begin(ssid, password); 
 
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
}

void wifi(String payload){
  if(WiFi.status()== WL_CONNECTED){ 

  HTTPClient http;   
 
  http.begin(url);  
  int httpResponseCode = http.POST(payload); 
 
   if(httpResponseCode>0){
     String response = http.getString(); 
     Serial.println("httpResponseCode : " + httpResponseCode);
     }
     http.end();
 
  }else{
    Serial.println("Error in WiFi connection");    
  }
  delay(1000);
}

void loop(){
  sensorValue = analogRead(pin); 
  Serial.print(sensorValue, DEC);
  Serial.print(" utnyhbr\n"); 

  last = crossed;
      
  if(sensorValue >= 0){
    crossed = 0;
  }else{
    crossed = 1;
  }
  
  String payload = payload_pattern;
  payload.replace("$laser$", String(crossed));
  
  if(crossed == 0 || (crossed == 1 && last == 0)){
    wifi(payload);
  }

}