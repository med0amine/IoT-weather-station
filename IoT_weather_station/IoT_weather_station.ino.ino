/*
   Copyright [2023] [Mohamed Amine Ben Ammar]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
/*
_name_: IoT weather station
_author_: Mohamed Amine Ben Ammar
_date-: 23/07/2023
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <BMP180MI.h>
#include <Wire.h>
 
#include "WeatherStationIndex.h" 
#include "DHTesp.h" 
 
#define LED 2 
#define DHTpin 14 
 
SFE_BMP180 pressure;
 
#define ALTITUDE 175.0
 
DHTesp dht;
 

const char* ssid = "***********";
const char* password = "************";
 
ESP8266WebServer server(80); 
 
void handleRoot() 
{
  String s = MAIN_page; 
  server.send(200, "text/html", s); //Send web page
}
 
float humidity, temperature;
 
void handleADC() 
{
  char status;
  double T,P,p0,a;
  double Tdeg, Tfar, phg, pmb;
  
  status = pressure.startTemperature();
  if (status != 0)
  {
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      Serial.print("temperature: ");
      Serial.print(T,2);
      Tdeg = T;
      Serial.print(" deg C, ");
      Tfar = (9.0/5.0)*T+32.0;
      Serial.print((9.0/5.0)*T+32.0,2);
      Serial.println(" deg F");
      
      status = pressure.startPressure(3);
      if (status != 0)
      {
        delay(status);
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          Serial.print("absolute pressure: ");
          Serial.print(P,2);
          pmb = P;
          Serial.print(" mb, ");
          phg = P*0.0295333727;
          Serial.print(P*0.0295333727,2);
          Serial.println(" inHg");

          p0 = pressure.sealevel(P,ALTITUDE); 
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0,2);
          Serial.print(" mb, ");
          Serial.print(p0*0.0295333727,2);
          Serial.println(" inHg");
          
          a = pressure.altitude(P,p0);
          Serial.print("computed altitude: ");
          Serial.print(a,0);
          Serial.print(" meters, ");
          Serial.print(a*3.28084,0);
          Serial.println(" feet");
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
  
  int rain = analogRead(A0);

  String data = "{\"Rain\":\""+String(rain)+"\",\"Pressuremb\":\""+String(pmb)+"\",\"Pressurehg\":\""+String(phg)+"\", \"Temperature\":\""+ String(temperature) +"\", \"Humidity\":\""+ String(humidity) +"\"}";
  
  digitalWrite(LED,!digitalRead(LED)); 
  server.send(200, "text/plane", data); 
  delay(dht.getMinimumSamplingPeriod());
  humidity = dht.getHumidity();
  temperature = dht.getTemperature();
  
  Serial.print("H:");
  Serial.println(humidity);
  Serial.print("T:");
  Serial.println(temperature); 
  Serial.print("R:");
  Serial.println(rain);
}
 
void setup()
{
  Serial.begin(115200);
  Serial.println();
  
  dht.setup(DHTpin, DHTesp::DHT11); 
  pinMode(LED,OUTPUT);
  
  if (pressure.begin()){
    Serial.println("BMP180 init success");
  }
  else
  {
    Serial.println("BMP180 init fail\n\n");
    while(1); 
  }
  
  WiFi.begin(ssid, password);
  Serial.println("");
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 
  
  server.on("/", handleRoot); 
  server.on("/readADC", handleADC);
  
  server.begin(); 
  Serial.println("HTTP server started");
}
 
void loop()
{
  server.handleClient(); 
}