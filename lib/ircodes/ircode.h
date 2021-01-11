#ifndef IRCODE_H
#define IRCODE_H

#include <Arduino.h>

class IRcode
{
private:
    /* nothing */
public:
    IRcode();
    IRcode(String cmd,String description,String code,String gcCode,int repeat=1);
    ~IRcode();
    String Cmd;
    String Description;
    String Code;
    String GcCode;
    int Repeat=1;
};

IRcode::IRcode()
{
    Cmd="";
    Description="empty";
    Code="";
    GcCode="";
    Repeat=1;
}

IRcode::IRcode(String cmd,String description,String code,String gcCode,int repeat)
{
    Cmd=cmd;
    Description=description;
    Code=code;
    GcCode=gcCode;
    Repeat=repeat;
}

IRcode::~IRcode()
{
}


#endif