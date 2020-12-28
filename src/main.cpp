// System Header Files
#include <Arduino.h>
#include <WiFi.h>
#include <esp_system.h>
#include <AsyncUDP.h>
#include <ESP32httpUpdate.h>
#include <Update.h>


// Spesific Header Files
#include "esp_uart.h"
#include "esp_services.h"
#include "esp_eeprom.h"
#include "esp_tcp_server.h"
#include "freertos/FreeRTOSConfig.h"

#define WIFI_COMMAND 0
#define CURRENT_VERSION (float)0.1

// Task Declarations
TaskHandle_t TaskRead;
TaskHandle_t TaskCredetials;
TaskHandle_t TaskScan;
TaskHandle_t TaskCheck;
TaskHandle_t TaskBroadcast;
TaskHandle_t TaskTcpRx;
TaskHandle_t TaskTcpTx;
TaskHandle_t TaskListenClient;
TaskHandle_t TaskServerStart;
TaskHandle_t TaskUpdateEsp;
TaskHandle_t TaskHttpVersion;

// Function Declarations
void scan_networks(void *parameters);
void read_uart1(void *parameters);
void check_wifi_connection(void *parameters);
void read_wifi_credential(void *parameters);
void udp_broadcast(void *parameters);
void listen_client(void *parameters);
void read_client(void *parameters);
void tcp_server_start(void *parameters);
void update_esp(void *parameters);
void http_post(void *parameters);
int which_command(String cmd);
void create_task();

/*
* ESP32 uart1 configurations
* Pin no: 16 17
* Baud Rate: 115200
*/
EspUart *espUart = EspUart::get_instance();
HardwareSerial uartSerial = espUart->uart_init();

EspTcpServer *espTcpServer = EspTcpServer::get_instance();

// Object Instances
EspServices espServices = EspServices();
EspEeprom _espEeprom = EspEeprom(96);
AsyncUDP udp;
WiFiClient client;

httpResponse _response;

bool wifiConnection = false;
bool _startServer = false;
bool updateRequired = false;

// Variables to validate
// response from S3
long contentLength = 0;
bool isValidContentType = false;
int port = 80;

// Utility to extract header value from headers
String getHeaderValue(String header, String headerName)
{
  return header.substring(strlen(headerName.c_str()));
}

void setup()
{
  uartSerial.println("Setup");
  uartSerial.println("Current Version: " + String(CURRENT_VERSION));
  if (!espServices.wifi_connect())
  {
    uartSerial.println("Wifi connection error!!!!!");
  }
  else
  {
    wifiConnection = true;
  }
  xTaskCreate(read_uart1, "readUart", (1024 * 4), NULL, configMAX_PRIORITIES,&TaskRead);
  delay(5000);
  create_task();
}

void loop()
{
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

// RTOS Tasks
void create_task()
{
  xTaskCreate(check_wifi_connection, "checkWifi", (1024 * 2), NULL, 1, &TaskCheck);
  xTaskCreate(udp_broadcast, "udpBroadcast", (1536), NULL, 1, &TaskBroadcast);
  xTaskCreate(tcp_server_start, "tcpServer", 1024, NULL, 1, &TaskServerStart);
  xTaskCreate(listen_client, "listenClient", (1024 * 2), NULL, 15, &TaskListenClient);
  xTaskCreate(read_client, "readMessages", (1024 * 4), NULL, configMAX_PRIORITIES, &TaskTcpRx);
  xTaskCreate(http_post, "httpPost", 1024 * 2, NULL, 5, &TaskHttpVersion);
}

void read_uart1(void *parameters)
{
  String incomingString;
  char newChar;
  uartSerial.println("UART READY");
  for (;;)
  {
    if (uartSerial.available())
    {
      incomingString = "";
      // read the incoming byte:
      while (uartSerial.available())
      {
        newChar = static_cast<char>(uartSerial.read());
        incomingString += newChar;
      }
      if (incomingString.startsWith("#"))
      {
        int firstSign = incomingString.indexOf(':', 0);

        // say what you got:
        String commandInString = incomingString.substring(1, firstSign);

        int command = which_command(commandInString);

        switch (command)
        {
        case WIFI_COMMAND:
        {
          int ssidStart = firstSign + 1;
          int ssidEnd = incomingString.indexOf("~");

          String ssid = incomingString.substring(ssidStart, ssidEnd);
          String password = incomingString.substring((ssidEnd + 1));
          uartSerial.println("SSID: " + ssid);
          uartSerial.println("Password: " + password);
          if (espServices.wifi_init(ssid, password))
          {
            uartSerial.println("Wifi connection successfull: " + ssid + " : " + password);
          }
          else
          {
            uartSerial.println("SSID or PASSWORD is not correct!");
          }
          break;
        }

        default:
          uartSerial.println("Undefined Command Request");
          break;
        }
      }
      else
      {
        uartSerial.print("I received: ");
        uartSerial.println(incomingString);
        espTcpServer->sendTcpMessage(incomingString);
      }
    }
    else
    {
      vTaskDelay(5 / portTICK_PERIOD_MS);
      //continue;
    }
  }
  vTaskDelete(NULL);
}

void listen_client(void *parameters)
{
  for (;;)
  {
    if (_startServer && espTcpServer->clientNumber <= 5)
    {
      espTcpServer->getWiFiClient();
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    else
    {
      uartSerial.println("Server is not active!");
      vTaskDelay(20000 / portTICK_PERIOD_MS);
    }
  }
  vTaskDelete(NULL);
}

void read_client(void *parameters)
{
  for (;;)
  {
    if (_startServer)
    {
      espTcpServer->readTcpMessage();
      vTaskDelay(5 / portTICK_PERIOD_MS);
      espTcpServer->checkClients();
      vTaskDelay(5 / portTICK_PERIOD_MS);
    }
    else
    {
      uartSerial.println("Server is not active!");
      vTaskDelay(20000 / portTICK_PERIOD_MS);
    }
  }
  vTaskDelete(NULL);
}

void check_wifi_connection(void *parameters)
{
  for (;;)
  {
    if (!WiFi.isConnected())
    {
      uartSerial.println("Wifi Communication Error");
      wifiConnection = false;

      vTaskDelay(20000 / portTICK_PERIOD_MS);
      vTaskSuspend(TaskBroadcast);
      vTaskSuspend(TaskServerStart);
      if (espServices.wifi_connect())
      {
        uartSerial.println("Reconnected to the AP");
        wifiConnection = true;
      }
      vTaskResume(TaskBroadcast);
      vTaskResume(TaskServerStart);
      vTaskDelay(20000 / portTICK_PERIOD_MS);
    }
    else
    {
      wifiConnection = true;
      uartSerial.println("Wifi Communication Successful");
      vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
  }
  vTaskDelete(NULL);
}

int which_command(String cmd)
{
  if (cmd.equals("WiFi"))
  {
    return WIFI_COMMAND;
  }
  else
  {
    return 1;
  }
  return 1;
}

void udp_broadcast(void *parameters)
{
  String ip;
  for (;;)
  {
    if (WiFi.isConnected())
    {
      udp.connect(IPAddress(192, 168, 1, 255), 1234);
      ip = WiFi.localIP().toString();
      udp.broadcastTo(ip.c_str(), 1234);
      uartSerial.println("UDP Broadcast");
      vTaskDelay(20000 / portTICK_PERIOD_MS);
    }
    else
    {
      vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
  }
  vTaskDelete(NULL);
}

void tcp_server_start(void *parameters)
{
  for (;;)
  {
    if (WiFi.isConnected())
    {
      if (!_startServer)
      {
        espTcpServer->startServer();
        uartSerial.println("Server Started");
        _startServer = true;
      }
      vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    else
    {
      _startServer = false;
      vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
  }
  vTaskDelete(NULL);
}

void update_esp(void *parameters)
{
  for (;;)
  {
    if ( (_response.version > CURRENT_VERSION) && wifiConnection)
    {
      vTaskSuspend(TaskHttpVersion);
      uartSerial.println("Connecting to: " + String(_response.host));

      if (client.connect(_response.host.c_str(), port))
      {

        // Connection Succeed.
        // Fecthing the bin
        uartSerial.println("Fetching Bin: " + String(_response.bin));

        // Get the contents of the bin file
        client.print(String("GET ") + _response.bin + " HTTP/1.1\r\n" +
                     "Host: " + _response.host + "\r\n" +
                     "Cache-Control: no-cache\r\n" +
                     "Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (client.available() == 0)
        {
          if (millis() - timeout > 5000)
          {
            uartSerial.println("Client Timeout !");
            client.stop();
            return;
          }
        }
        // Once the response is available,
        // check stuff
        while (client.available())
        {
          // read line till /n
          String line = client.readStringUntil('\n');
          // remove space, to check if the line is end of headers
          line.trim();

          // if the the line is empty,
          // this is end of headers
          // break the while and feed the
          // remaining `client` to the
          // Update.writeStream();
          if (!line.length())
          {
            //headers ended
            break; // and get the OTA started
          }

          // Check if the HTTP Response is 200
          // else break and Exit Update
          if (line.startsWith("HTTP/1.1"))
          {
            if (line.indexOf("200") < 0)
            {
              uartSerial.println("Got a non 200 status code from server. Exiting OTA Update.");
              break;
            }
          }

          // extract headers here
          // Start with content length
          if (line.startsWith("Content-Length: "))
          {
            contentLength = atol((getHeaderValue(line, "Content-Length: ")).c_str());
            uartSerial.println("Got " + String(contentLength) + " bytes from server");
          }

          // Next, the content type
          if (line.startsWith("Content-Type: "))
          {
            String contentType = getHeaderValue(line, "Content-Type: ");
            uartSerial.println("Got " + contentType + " payload.");
            if (contentType == "application/octet-stream")
            {
              isValidContentType = true;
            }
          }
        }
      }
      else
      {
        // Connect to S3 failed
        // May be try?
        // Probably a choppy network?
        uartSerial.println("Connection to " + String(_response.host) + " failed. Please check your setup");
        // retry??
        // execOTA();
      }

      // Check what is the contentLength and if content type is `application/octet-stream`
      uartSerial.println("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));

      // check contentLength and content type
      if (contentLength && isValidContentType)
      {
        // Check if there is enough to OTA Update
        bool canBegin = Update.begin(contentLength);

        // If yes, begin
        if (canBegin)
        {
          uartSerial.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!");
          // No activity would appear on the Serial monitor
          // So be patient. This may take 2 - 5mins to complete
          size_t written = Update.writeStream(client);

          if (written == contentLength)
          {
            uartSerial.println("Written : " + String(written) + " successfully");
          }
          else
          {
            uartSerial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
            // retry??
            // execOTA();
            vTaskSuspend(TaskHttpVersion);
            vTaskDelete(NULL);
          }

          if (Update.end())
          {
            uartSerial.println("OTA done!");
            if (Update.isFinished())
            {
              uartSerial.println("Update successfully completed. Rebooting.");
              ESP.restart();
            }
            else
            {
              uartSerial.println("Update not finished? Something went wrong!");
            }
          }
          else
          {
            uartSerial.println("Error Occurred. Error #: " + String(Update.getError()));
          }
        }
        else
        {
          // not enough space to begin OTA
          // Understand the partitions and
          // space availability
          uartSerial.println("Not enough space to begin OTA");
          client.flush();
        }
      }
      else
      {
        uartSerial.println("There was no content in the response");
        client.flush();
      }
      vTaskResume(TaskHttpVersion);
      vTaskDelete(NULL);
    }
  }
}

void http_post(void *parameters){
  for(;;){
    if (WiFi.isConnected())
    {
      _response = espServices.http_post();
      uartSerial.println(_response.httpErrorCode);
      uartSerial.println(_response.version);
      uartSerial.println(_response.host);
      uartSerial.println(_response.bin);
      uartSerial.println(_response.response);
      
      if (_response.version > CURRENT_VERSION)
      {
        xTaskCreatePinnedToCore(
          update_esp,
         "updateEsp",
          (1024 * 4),
          NULL,
          configMAX_PRIORITIES,
          &TaskUpdateEsp,
          1
        );
      }
      
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }else
    {
      vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    
  }
}