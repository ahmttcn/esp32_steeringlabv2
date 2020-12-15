#include "esp_uart.h"
#include <HardwareSerial.h>

EspUart * EspUart::_espUart = nullptr;

EspUart * EspUart::get_instance(){
    if (_espUart == nullptr)
    {
        _espUart = new EspUart();
    }
    return _espUart;
}

EspUart::EspUart(){
    uart1.begin(115200,SERIAL_8N1,16,17);
}

HardwareSerial EspUart::uart_init(){
    return uart1;
}