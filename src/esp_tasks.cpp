#include <Arduino.h>
#include <AsyncUDP.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>

#include "esp_tasks.h"
#include "esp_eeprom.h"
#include "esp_services.h"
#include "esp_uart.h"
/*
#define WIFI_COMMAND 0

EspUart * espUart = EspUart::get_instance();
HardwareSerial uart_serial = espUart->uart_init();

EspEeprom _espEeprom = EspEeprom(96);

EspServices espServices = EspServices();

AsyncUDP udp;

bool wifiConnection = false;
EspTasks::EspTasks(){
  
}

void EspTasks::read_uart1( void * parameters) {
  String incomingString;
  for(;;){
    if (uart_serial.available()) {
      // read the incoming byte:
      incomingString = uart_serial.readString();
      if (incomingString.startsWith("#"))
      {
        int firstSign = incomingString.indexOf(':',0);

        // say what you got:
        String commandInString = incomingString.substring(1,firstSign);
        
        int command = which_command(commandInString);

        switch (command) {
          case WIFI_COMMAND:
            {
              int ssidStart = firstSign + 1;
              int ssidEnd = incomingString.indexOf("~");

              String ssid = incomingString.substring(ssidStart,ssidEnd);
              String password = incomingString.substring((ssidEnd + 1));
              uart_serial.println("SSID: " + ssid);
              uart_serial.println("Password: " + password);
              //vTaskSuspend(TaskCredetials);
              if(espServices.wifi_init(ssid,password)){
                uart_serial.println("Wifi connection successfull: " + ssid + " : " + password);
              }else
              {
                uart_serial.println("SSID or PASSWORD is not correct!");
              }
              //vTaskResume(TaskCredetials);
              break;
            }
        
          default:
            uart_serial.println("Undefined Command Request");
            break;
          }
      }else
      {
        uart_serial.print("I received: ");
        uart_serial.println(incomingString); 
      }
    }else
    {
      vTaskDelay(5 / portTICK_PERIOD_MS);
      continue;
    }
  }
}

void EspTasks::read_wifi_credential(void * parameters){
  for(;;){
    uart_serial.println("Get SSID\n");
    uart_serial.println(_espEeprom.get_ssid() + "\n");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    uart_serial.println("Get PASSWORD\n");
    uart_serial.println(_espEeprom.get_password() + "\n");
    vTaskDelay(2000/portTICK_PERIOD_MS);
  }
}

void EspTasks::scan_networks(void * parameters ) {
  int8_t count;
  for (;;){
    count = WiFi.scanNetworks();
    uart_serial.println("Found" + String(count) + "networks");
    for (uint8_t i=0; i<count; i++) {
      String ssid;
      uint8_t encryptionType;
      int32_t RSSI;
      uint8_t *BSSID;
      int32_t channel;
      WiFi.getNetworkInfo(i, ssid, encryptionType, RSSI, BSSID, channel);
      uart_serial.println("ssid = " + ssid);
    }
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void EspTasks::check_wifi_connection(void * parameters){
  for(;;){
    if (WiFi.status() != WL_CONNECTED)
    {
      uart_serial.println("Wifi Communication Error");
      if (espServices.wifi_connect()){
        uart_serial.println("Reconnected to the AP");
      }
      wifiConnection = false;
      vTaskDelay(20000 / portTICK_PERIOD_MS);
    }else
    {
      wifiConnection = true;
      uart_serial.println("Wifi Communication Successful");
      vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    
  }
}

void EspTasks::udp_broadcast(void * parameters){
  String ip;
  udp.connect(IPAddress(192,168,1,255), 1234);
  for(;;)
  {
    if (wifiConnection)
    {
      ip = WiFi.localIP().toString();
      udp.broadcastTo(ip.c_str(), 1234);
      uart_serial.println("UDP Broadcast");
      vTaskDelay(2000 / portTICK_PERIOD_MS );
    }else
    {
      vTaskDelay(5000 / portTICK_PERIOD_MS );
    }
  }
}

int EspTasks::which_command(String cmd){
  if (cmd.equals("WiFi"))
  {
    return WIFI_COMMAND;
  }else
  {
    return 1;
  }
  return 1;
}*/
