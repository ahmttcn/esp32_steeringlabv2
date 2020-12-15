#ifndef ESPSERCIVES_H
#define ESPSERVICES_H
#include <Arduino.h>
#include "esp_uart.h"
#include "esp_eeprom.h"

class EspServices
{
public:
    static const int SERVER_PORT = 80;
    EspServices();
    String http_post();
    bool wifi_init(String ssid, String password);
    void server_start();
    bool wifi_connect();
    String get_eeprom();
    String get_Eeprom();
    EspUart * esp_uart = EspUart::get_instance();
    HardwareSerial serial_uart = esp_uart->uart_init();
    EspEeprom espEeprom = EspEeprom(96);
    ~EspServices();
};

#endif
