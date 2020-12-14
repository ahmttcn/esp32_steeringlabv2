#include <esp_services.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "esp_eeprom.h"
#include "esp_uart.h"

EspEeprom espEeprom = EspEeprom(96);
HTTPClient http;
//WiFiServer server;

EspUart * esp_uart = EspUart::get_instance();
HardwareSerial serial_uart = esp_uart->uart_init();

EspServices::EspServices(){
  
}

bool EspServices::wifi_init(String id, String pass) {
  int counter = 0;
  id.trim();
  pass.trim();

  const char* ssid = (char *) id.c_str();
  const char* password = (char *) pass.c_str();

  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); 

  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
      delay(1000);
      counter++;
      if (counter > 2)
      {
        counter = 0;
        return false;
      }
  }

  while (!espEeprom.write_ssid(id))
  {}
  
  while (!espEeprom.write_password(pass))
  {}

  return true;
}

bool EspServices::wifi_connect(){
  int counter = 0;
  String id = espEeprom.get_ssid();
  String pass = espEeprom.get_password();

  //WiFi.mode(WIFI_STA);
  WiFi.begin(id.c_str(), pass.c_str());

  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
      delay(1000);
      counter++;
      if (counter > 2)
      {
        counter = 0;
        return false;
      }
  }
  serial_uart.println("Wifi Connect Function");
  return true;
}

String EspServices::get_eeprom(){
  return espEeprom.get_password();
}

String EspServices::get_Eeprom(){
  return espEeprom.get_ssid();
}

void EspServices::server_start(){
  if (WiFi.status() == WL_CONNECTED)
  {
    //server.begin();
  }
}

/*
* Http post function is used for post operations.
*/
String EspServices::http_post() {

  if(WiFi.status() == WL_CONNECTED){                          //Check WiFi connection status   
    
    http.begin("http://jsonplaceholder.typicode.com/posts");  //Specify destination for HTTP request
    http.addHeader("Content-Type", "text/plain");             //Specify content-type header
    
    int httpResponseCode = http.POST("POSTING from ESP32");   //Send the actual POST request
    
    if(httpResponseCode>0){
    
      String response = http.getString();                     //Get the response to the request
      return response;
    
    }else{
      http.end();                                             //Free resources
      return String(httpResponseCode);
    }
    
  }else{
    return "Error in wifi connection";
  }
}

EspServices::~EspServices(){
  //server.stop;
}