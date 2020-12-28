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
    int i = 0;
    char temp;
    EEPROM.begin(96);
    while (i < 32)
    {
        temp = char(EEPROM.read(i));
        if (temp == '\0')
        {
            break;
        }
        ssid += temp;
        i++;
    }
    EEPROM.end();
    return ssid;
}

String EspEeprom::get_password(){
    String password;
    int i = 32;
    char temp;
    EEPROM.begin(100);
    while (i < 96)
    {
        temp = char(EEPROM.read(i));
        if (temp == '\0')
        {
            break;
        }
        password += temp;
        i++;
    }
    EEPROM.end();
    return password;
}

EspEeprom::~EspEeprom(){
    EEPROM.end();
}