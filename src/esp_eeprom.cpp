#include <EEPROM.h>
#include "esp_eeprom.h"

EspEeprom::EspEeprom(int size){
    //EEPROM.begin(size);
}

bool EspEeprom::write_ssid(String ssid){
    EEPROM.begin(96);
    for (int i = 0; i < 32; i++)
    {
        EEPROM.write(i,ssid[i]);
    }
    //bool resp = EEPROM.commit();
    EEPROM.end();
    return true;
}

bool EspEeprom::write_password(String password){
    EEPROM.begin(96);
    for (int i = 32; i < 96; i++)
    {
        EEPROM.write(i,password[i - 32]);
    }
    //bool resp = EEPROM.commit();
    EEPROM.end();
    return true;
}

String EspEeprom::get_ssid(){
    String ssid;
    EEPROM.begin(96);
    for (int i = 0; i < 32; ++i)
    {
        ssid += char(EEPROM.read(i));
    }
    EEPROM.end();
    return ssid;
}

String EspEeprom::get_password(){
    String password;
    EEPROM.begin(100);
    for (int i = 32; i < 96; ++i)
    {
        password += char(EEPROM.read(i));
    }
    EEPROM.end();
    return password;
}

EspEeprom::~EspEeprom(){
    EEPROM.end();
}