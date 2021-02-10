/*
 This code is product of many other sample code  (WiFI connect, BME280, APRS data, WEB Server etc.).
esp8266(NodeMCU) with BME280
- upload weather data on aprs servers
- local web page for weather informations
 9A4AM Mario Ančić mancic12@gmail.com +385 98 256 372
 08.12.2020. ----> V1
*/
// Library included in this project

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


//Variable defined in this project
  float temperatureC;
  int temperatureF;
  int pression;
  float pressionHTML;
  int humidite;
  int offset_pres = 11; //Offset for pressure
  int offset_temp = -1.5; //Offset for temperature
  int offset_hum = 0; //Offset for humidity
  unsigned long previousMillis = 0;
  /*#include <SPI.h>
#define BME_SCK 14
#define BME_MISO 12
#define BME_MOSI 13
#define BME_CS 15*/

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C
//NodeMCU use D1 to SCL, D2 to SDA
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI


//**************************************************************SET YOUR DATA FROM HERE***************************************************************
// Set your GEO Location
const char* latitude = "XXXX.XXN";
const char* longitude = "XXXXX.XXE";
  


  
// Set your WiFi (Defoult use static IP - reason - WEB server)
const char* ssid = "XXXX";
const char* password = "XXXXX";
// Set your Static IP address
IPAddress local_IP(192, 168, 1, 88); //Must be outside DHCP range -------- for the second device change this IP address to other one
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional


// Set your Call without SSID
const char* call = "XXXXX";

// APRS data
// Set your Call-SSID (SSID 13 for WX station)
const char* APRScall = "XXXXX-13";       //-------- for the second device change this SSID number to other one
// Set your password
const char* APRSpassword = "XXXXX";
const char* host = "finland.aprs2.net";
int port = 14580;
const long interval = 600000;           // interval at which to send APRS data (milliseconds) 600000 for 10min
ESP8266WebServer server(80); // Start WEB server on port 80, if you need, change port nummber

//*********************************************************************TO HERE**************************************************************************



void setup()
{
 
  
  
  Serial.begin(115200);
  Serial.println();
   //Intro
  Serial.println("Program APRS WX ststion and WEB with ESP8266-NodeMCU - by 9A4AM - Mario @2020"); 
  //Init BME280
   bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  // START WiFI connection
  Serial.printf("Connecting to %s ", ssid);
  WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);  //Comment if use DHCP
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
    BME280_Read();
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature() + offset_temp);
    Serial.println(" *C");
    Serial.print("Pressure = ");
    Serial.print((bme.readPressure() / 100.0F) + offset_pres);
    Serial.println(" hPa");
    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity() + offset_hum);
    Serial.println(" %");
  //Calculate temperatureC to temperatureF for APRS
    temperatureF = (temperatureC*1.8)+32;
  APRS_Send();   // for local station, without APRS, disable this option with comment //
  //Serial.println("Okino ga sad");
}


void loop()
{
   //BME280 read data
    BME280_Read();
    
  
 // HTTP Server start 
     server.handleClient();
   
     unsigned long currentMillis = millis();
     
  //Routine for send data to APRS server periodic (10 minute)
  if (currentMillis - previousMillis >= interval) {
    // save the last time you send data
    previousMillis = currentMillis;
    //Serial.println("Okino ga sad");
    // Send APRS data to server
    APRS_Send();   // for local station, without APRS, disable this option with comment //
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature() + offset_temp);
    Serial.println(" *C");
    Serial.print("Pressure = ");
    Serial.print((bme.readPressure() / 100.0F) + offset_pres);
    Serial.println(" hPa");
    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity() + offset_hum);
    Serial.println(" %");
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
  sprintf(sentence, "%s>APRS,TCPIP*:@090247z%s/%s_.../...t%03dh%02db%05dWX-Station", APRScall, latitude, longitude, temperatureF, humidite, (pression + 100));
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
  pressionHTML = (pression/10.0F) + offset_pres;
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
    <p>Temperature: %3.1f degC</p><br>\
    <p>Pressure: %4.1f hPa</p><br>\
    <p>Humidity: %d rH</p><br>\
  </body>\
</html>",
             call,call, temperatureC, pressionHTML, humidite
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
void BME280_Read() {
  //Serial.print("Temperature = ");
  //Serial.print(bme.readTemperature());
  //Serial.println(" *C");
  temperatureC = (bme.readTemperature())+ offset_temp;
  
   //Calculate temperatureC to temperatureF for APRS
    temperatureF = (temperatureC*1.8)+32;
    
  
  // Convert temperature to Fahrenheit
  /*Serial.print("Temperature = ");
  Serial.print(1.8 * bme.readTemperature() + 32);
  Serial.println(" *F");*/
  
  //Serial.print("Pressure = ");
  //Serial.print(bme.readPressure() / 100.0F);
  //Serial.println(" hPa");
  pression = (bme.readPressure()/10.0F)+ offset_pres;
 
  

  //Serial.print("Approx. Altitude = ");
  //Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  //Serial.println(" m");

  //Serial.print("Humidity = ");
  //Serial.print(bme.readHumidity());
  //Serial.println(" %");
  humidite = (bme.readHumidity())+ offset_hum;
 
  
  //Serial.println();
}
