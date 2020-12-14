#ifndef ESPEEPROM_H
#define ESPEEPROM_H

class EspEeprom
{
private:

public:
    EspEeprom(int);
    bool write_ssid(String ssid);
    bool write_password(String password);
    String get_ssid();
    String get_password();
    ~EspEeprom();
};

#endif
