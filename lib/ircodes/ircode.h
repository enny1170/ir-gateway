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
    String GcCode;
};

IRcode::IRcode()
{
    Cmd="";
    Description="empty";
    Code="";
    GcCode="";
}

IRcode::IRcode(String cmd,String description,String code,String gcCode="")
{
    Cmd=cmd;
    Description=description;
    Code=code;
    GcCode=gcCode;
}

IRcode::~IRcode()
{
}


#endif