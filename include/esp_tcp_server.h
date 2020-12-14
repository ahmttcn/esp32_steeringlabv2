#ifndef ESPTCPSERVER_H
#define ESPTCPSERVER_H
#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include "esp_uart.h"
#include <HardwareSerial.h>

#define MAX_CLIENT 5

class EspTcpServer{
    private:
        EspTcpServer();
        static EspTcpServer* _espTcpServer;
    public:
        static EspTcpServer* get_instance();
        EspUart * espUart1 = EspUart::get_instance();
        HardwareSerial serialUart = espUart1->uart_init();
        int clientNumber;
        void startServer();
        void stopServer();
        void getWiFiClient();
        void readTcpMessage();
        void sendTcpMessage(String message);
        WiFiClient * clients[MAX_CLIENT] = { NULL };
        WiFiServer server = WiFiServer(80);
        void checkClients();
};

#endif