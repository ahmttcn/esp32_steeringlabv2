#ifndef ESPUART_H
#define ESPUART_H
#include <Arduino.h>
#include <HardwareSerial.h>

class EspUart{
    private:
        EspUart();
        static EspUart* _espUart;
    public:
        static EspUart* get_instance();
        HardwareSerial uart_init();
};

#endif