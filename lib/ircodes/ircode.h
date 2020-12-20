#ifndef IRCODE_H
#define IRCODE_H

#include <Arduino.h>

class IRcode
{
private:
    /* nothing */
public:
    IRcode();
    IRcode(String cmd,String description,String code);
    ~IRcode();
    String Cmd;
    String Description;
    String Code;
};

IRcode::IRcode()
{
    Cmd="";
    Description="empty";
    Code="";
}

IRcode::IRcode(String cmd,String description,String code)
{
    Cmd=cmd;
    Description=description;
    Code=code;
}

IRcode::~IRcode()
{
}


#endif