#include <esp_services.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#include "esp_eeprom.h"
#include "esp_uart.h"

HTTPClient http;
//WiFiServer server;

EspServices::EspServices(){
  
}

bool EspServices::wifi_init(String id, String pass) {
  id.trim();
  pass.trim();

  const char* ssid = (char *) id.c_str();
  const char* password = (char *) pass.c_str();
  if (check_ssid(ssid,password))
  {
    espEeprom.write_ssid(id);
    espEeprom.write_password(pass); 
    return true;
  }else
  {
    return false;
  }
  
}

bool EspServices::wifi_connect(){

  String id = espEeprom.get_ssid();
  String pass = espEeprom.get_password();
  id.trim();
  pass.trim();
  serial_uart.println("GET EEPROM");
  serial_uart.println(id);
  serial_uart.println(pass);

  return check_ssid(id,pass);
}

bool EspServices::check_ssid(String id, String password){
  int counter = 0;
  String ssid;
  uint8_t encryptionType;
  int32_t RSSI;
  uint8_t *BSSID;
  int32_t channel;

  int count = WiFi.scanNetworks();
  serial_uart.println("Found" + String(count) + "networks");

  for (uint8_t i = 0; i < count; i++)
  {
    WiFi.getNetworkInfo(i, ssid, encryptionType, RSSI, BSSID, channel);
    ssid.trim();
    serial_uart.println(ssid);
    if (id == ssid)
    {
      serial_uart.println("AP found: " + id + " == " + ssid);
      WiFi.disconnect();
      WiFi.begin(id.c_str(),password.c_str());
      serial_uart.print("Connecting Wifi..");
      while (WiFi.status() != WL_CONNECTED) { //Check for the connection
        delay(1000);
        serial_uart.print(".");
        counter++;
        if (counter > 5)
        {
          counter = 0;
          serial_uart.println("\nWifi Connect Timeout");
          return false;
        }
      }
      break;
    }
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    serial_uart.println("\nConnection is not successful");
    return false;
  }else
  {
    serial_uart.println("\nConnection is successful");
    return true;
  }
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
httpResponse EspServices::http_post() {

  struct httpResponse _httpResponse;

  if(WiFi.status() == WL_CONNECTED){                          //Check WiFi connection status   
    http.begin("http://esp32-ota-updates.s3-us-west-2.amazonaws.com/firmware.json");  //Specify destination for HTTP request
    http.addHeader("Content-Type", "text/plain");             //Specify content-type header
    
    int httpResponseCode = http.GET();                        //Send the actual POST request
    
    if(httpResponseCode>0){
    
      String response = http.getString();                     //Get the response to the request
      http.end();

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, response);
      JsonObject obj = doc.as<JsonObject>();
      float version = obj["version"];
      String host = obj["host"].as<String>();
      String bin = obj["bin"].as<String>();

      _httpResponse.version = version;
      _httpResponse.host = host;
      _httpResponse.bin = bin;
      _httpResponse.httpErrorCode = httpResponseCode;
      _httpResponse.response = response;
      
      return _httpResponse;
    
    }else{
      http.end();                                             //Free resources
      _httpResponse.version = 0.0;
      _httpResponse.host = "null";
      _httpResponse.bin = "null";
      _httpResponse.httpErrorCode = httpResponseCode;
      _httpResponse.response = "null";
      return _httpResponse;
    }
  }else{
      _httpResponse.version = 0.0;
      _httpResponse.host = "null";
      _httpResponse.bin = "null";
      _httpResponse.httpErrorCode = 0;
      _httpResponse.response = "Wifi connection lost";
    return _httpResponse;
  }
}

EspServices::~EspServices(){
  //server.stop;
}