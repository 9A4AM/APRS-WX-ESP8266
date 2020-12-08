/*
 This code is product of many other code sample (WiFI connect, BME280, APRS data, WEB Server etc.
esp8266 with BME280
- upload weather data on aprs servers
- local web page for weather informations
 9A4AM Mario Anèiæ mancic12@gmail.com +385 98 256 372
*/
// Library included in this project

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


//Variable defined in this project
  int temperatureC = 20;
  int temperatureF;
  int pression = 10000;
  int humidite = 55;
  unsigned long previousMillis = 0;
  


// Set your GEO Location
const char* latitude = "xxxx.xxN";
const char* longitude = "xxxxx.xxE";
  


  
// Set your WiFi
const char* ssid = "Your router SSID";
const char* password = "Your router password";
// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

// Set your Call without SSID
const char* call = "Your call";

// APRS data
// Set your Call-SSID if CWOP then EWxxxx
const char* APRScall = "Your call-x";
// Set your APRS password if CWOP then -1
const char* APRSpassword = "xxxxx";
const char* host = "finland.aprs2.net";
int port = 14580;
const long interval = 600000;           // interval at which to send APRS data (milliseconds) 600000 for 10min

ESP8266WebServer server(80);
void setup()
{
 
  
  // START WiFI connection
  Serial.begin(115200);
  Serial.println();
   //Intro
  Serial.println("Program APRS WX ststion and WEB with ESP8266-NodeMCU - by 9A4AM - Mario @2020"); 

  Serial.printf("Connecting to %s ", ssid);
  //WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  //Serial.println(local_IP);
   Serial.print("IP address is: ");
   Serial.println(WiFi.localIP());
  // START WEB Server
server.on("/", handleRoot);
  /*
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
server.on ( "/test.svg", drawGraph );
*/
  server.onNotFound(handleNotFound);
  server.begin();
  // First time send data to APRS server
  //BME280 read data
    //BME280_Read();
  //Calculate temperatureC to temperatureF for APRS
    temperatureF = (temperatureC*1.8)+32;
  APRS_Send();
  //Serial.println("Okino ga sad");
}


void loop()
{
   //BME280 read data
    //BME280_Read();
  //Calculate temperatureC to temperatureF for APRS
    temperatureF = (temperatureC*1.8)+32;
 // HTTP Server start 
    server.handleClient();
   
     unsigned long currentMillis = millis();
     
  //Routine for send data to APRS server periodic (10 minute)
  if (currentMillis - previousMillis >= interval) {
    // save the last time you send data
    previousMillis = currentMillis;
    //Serial.println("Okino ga sad");
    // Send APRS data to server
    APRS_Send();
  }
    
   
}
void APRS_Send(){

  WiFiClient client;

  Serial.printf("\n[Connecting to %s ", host);
  Serial.print (":");
  Serial.print (port);
  Serial.println("....");
  if (client.connect(host, port))
  {
    Serial.println("[Connected]");
    Serial.println("Login to APRS server");
    char login[60];
    char sentence[150];

  sprintf(login, "user %s pass %s vers WX_Station 0.1 filter m/1", APRScall, APRSpassword);
  //sprintf(sentence, "%s>APRS,TCPXX*:@%02d%02d%02dz%s/%s_.../...g...t%03dr...p...P...h%02db%05d", APRScall, dateTime.hour, dateTime.minute, dateTime.second, station.latitude, station.longitude, wx.temperatureF / 10, wx.humidite, wx.pression / 10);
  sprintf(sentence, "%s>APRS,TCPIP*:@090247z%s/%s_.../...t%03dr...p...P...h%02db%05dWX-ESP8266-BME280", APRScall, latitude, longitude, temperatureF, humidite, pression);
    client.println(login);
     Serial.println(sentence);           
                

    Serial.println("[Response:]");

  
    while (client.connected() || client.available())
    {
      
      if (client.available())
      {
       
        String line = client.readStringUntil('\n');
        Serial.println(line);
        delay(3000);
        client.println(sentence);
        Serial.println(sentence);
        delay(1000);
        client.stop();
      }
    }
    client.stop();
    Serial.println("\n[Disconnected]");
  }
  else
  {
    Serial.println("[Connection failed!]");
    client.stop();
  }
 
}
void handleRoot() {
  char temp[600];
  snprintf ( temp, 600,
             "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>%s --- Weather Station</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; font-size: 50px;}\
    </style>\
  </head>\
  <body>\
    <h1>%s --- Weather Station</h1>\
    <p>Temperature: %d degC</p><br>\
    <p>Pressure: %d pa</p><br>\
    <p>Humidity: %d rH</p><br>\
  </body>\
</html>",
             call,call, temperatureC, pression, humidite
           );
  server.send ( 200, "text/html", temp );
}





void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
 
  server.send(404, "text/plain", message);
 
}
