// System Header Files
#include <Arduino.h>
#include <WiFi.h>
#include <esp_system.h>
#include <AsyncUDP.h>

// Spesific Header Files
#include "esp_uart.h"
#include "esp_services.h"
#include "esp_eeprom.h"
#include "esp_tcp_server.h"

#define WIFI_COMMAND 0

// Task Declarations
TaskHandle_t TaskRead;
TaskHandle_t TaskCredetials;
TaskHandle_t TaskScan;
TaskHandle_t TaskCheck;
TaskHandle_t TaskBroadcast;
TaskHandle_t TaskTcpRx;
TaskHandle_t TaskTcpTx;
TaskHandle_t TaskListenClient;

// Function Declarations
void scan_networks(void * parameters);
void read_uart1(void * parameters);
void check_wifi_connection(void * parameters);
void read_wifi_credential(void * parameters);
void udp_broadcast(void * parameters);
void listen_client(void * parameters);
void read_client(void * parameters);
int which_command(String cmd);
void create_task();

/*
* ESP32 uart1 configurations
* Pin no: 16 17
* Baud Rate: 115200
*/
EspUart * espUart = EspUart::get_instance();
HardwareSerial uartSerial = espUart->uart_init();

EspTcpServer * espTcpServer = EspTcpServer::get_instance();

// Object Instances
EspServices espServices = EspServices();
EspEeprom _espEeprom = EspEeprom(96);
AsyncUDP udp;

bool wifiConnection = false;
bool _startServer = false;

void setup() {
  if(espServices.wifi_connect()){
    uartSerial.println("Wifi connection is successful");
    uartSerial.println("Server Started");
    espTcpServer->startServer();
    /*
    * Listen Tcp Client
    * Param: NULL
    * Priority: 25
    * Stack size: 4Kb 
    */
    uartSerial.println("Task Created");
    xTaskCreate(
      listen_client,
      "listenClient",
      (1024 * 4),
      NULL,
      25,
      &TaskListenClient
    );

    uartSerial.println("Task Created");
    xTaskCreate(
      read_client,
      "readMessages",
      (1024 * 4),
      NULL,
      25,
      &TaskTcpRx
    );
    
    uartSerial.println(WiFi.localIP().toString());
  }else
  {
    uartSerial.println("Wifi connection error!");
  }
  create_task();
}

void loop() {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

// RTOS Tasks
void create_task(){
  /*
  * Reads uart messages
  * Param: NULL
  * Priority: 25
  * Stack size: 4Kb 
  */
  xTaskCreate(
      read_uart1,
      "readUart",
      (1024 * 4),
      NULL,
      25,
      &TaskRead
  );

  /*
  * Reads wifi credentials
  * Param: NULL
  * Priority: 1
  * Stack size: 2Kb
  * Pinned Core 0 
  */
  xTaskCreatePinnedToCore(
    read_wifi_credential,
    "readWifi",
    (1024 * 2),
    NULL,
    1,
    &TaskCredetials,
    0
  );

  /*
  * Scans wifi networks
  * Param: NULL
  * Priority: 1
  * Stack size: 2Kb 
  */
  xTaskCreate(
    scan_networks,
    "scanNetworks",
    (1024 * 2),
    NULL,
    1,
    &TaskScan
  );

  /*
  * Check WiFi connection
  * Param: NULL
  * Priority: 1
  * Stack size: 1Kb 
  */
  xTaskCreate(
    check_wifi_connection,
    "checkWifi",
    (1024),
    NULL,
    1,
    &TaskCheck
  );

  xTaskCreate(
    udp_broadcast,
    "udpBroadcast",
    (1024 * 2),
    NULL,
    1,
    &TaskBroadcast
  );
}

void read_uart1( void * parameters) {
  String incomingString;
  for(;;){
    if (uartSerial.available()) {
      // read the incoming byte:
      incomingString = uartSerial.readString();
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
              uartSerial.println("SSID: " + ssid);
              uartSerial.println("Password: " + password);
              vTaskSuspend(TaskCredetials);
              if(espServices.wifi_init(ssid,password)){
                uartSerial.println("Wifi connection successfull: " + ssid + " : " + password);
              }else
              {
                uartSerial.println("SSID or PASSWORD is not correct!");
              }
              vTaskResume(TaskCredetials);
              break;
            }
        
          default:
            uartSerial.println("Undefined Command Request");
            break;
          }
      }else
      {
        uartSerial.print("I received: ");
        uartSerial.println(incomingString); 
        espTcpServer->sendTcpMessage(incomingString);
      }
    }else
    {
      vTaskDelay(5 / portTICK_PERIOD_MS);
      continue;
    }
  }
}

void listen_client(void * parameters){
  for(;;){
    if (_startServer && espTcpServer->clientNumber <= 5)
    {
      espTcpServer->getWiFiClient();
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }else
    {
      uartSerial.println("Start Server False");
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}

void read_client(void * parameters){
  for(;;)
  {
    if (_startServer)
    {
      espTcpServer->readTcpMessage();
      vTaskDelay(5 / portTICK_PERIOD_MS);
      espTcpServer->checkClients();
      vTaskDelay(5 / portTICK_PERIOD_MS);
    }else
    {
      uartSerial.println("Start Server False");
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}

void scan_networks(void * parameters ) {
  int8_t count;
  for (;;){
    count = WiFi.scanNetworks();
    uartSerial.println("Found" + String(count) + "networks");
    for (uint8_t i=0; i<count; i++) {
      String ssid;
      uint8_t encryptionType;
      int32_t RSSI;
      uint8_t *BSSID;
      int32_t channel;
      WiFi.getNetworkInfo(i, ssid, encryptionType, RSSI, BSSID, channel);
      uartSerial.println("ssid = " + ssid);
    }
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void check_wifi_connection(void * parameters){
  for(;;){
    if (WiFi.status() != WL_CONNECTED)
    {
      uartSerial.println("Wifi Communication Error");
      wifiConnection = false;
      _startServer = false;
      if (espServices.wifi_connect()){
        uartSerial.println("Reconnected to the AP");
        wifiConnection = true;
        _startServer = true;
      }
      vTaskDelay(20000 / portTICK_PERIOD_MS);
    }else
    {
      wifiConnection = true;
      _startServer = true;
      uartSerial.println("Wifi Communication Successful");
      vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    
  }
}

int which_command(String cmd){
  if (cmd.equals("WiFi"))
  {
    return WIFI_COMMAND;
  }else
  {
    return 1;
  }
  return 1;
}

void read_wifi_credential(void * parameters){
  for(;;){
    uartSerial.println("Get SSID\n");
    uartSerial.println(_espEeprom.get_ssid() + "\n");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    uartSerial.println("Get PASSWORD\n");
    uartSerial.println(_espEeprom.get_password() + "\n");
    vTaskDelay(2000/portTICK_PERIOD_MS);
  }
}

void udp_broadcast(void * parameters){
  String ip;
  udp.connect(IPAddress(192,168,1,255), 1234);
  for(;;)
  {
    if (wifiConnection)
    {
      ip = WiFi.localIP().toString();
      udp.broadcastTo(ip.c_str(), 1234);
      uartSerial.println("UDP Broadcast");
      vTaskDelay(2000 / portTICK_PERIOD_MS );
    }else
    {
      vTaskDelay(5000 / portTICK_PERIOD_MS );
    }
  }
}



