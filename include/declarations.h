#ifndef DECLARATION_H
#define DECLARATION_H

#include <Arduino.h>

struct httpResponse
{
    int httpErrorCode;
    String response; 
    float version;
    String host;
    String bin;
};

#endif