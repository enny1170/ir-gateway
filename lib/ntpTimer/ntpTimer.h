#ifndef NTPTIMER_H
#define NTPTIMER_H

#include <NTPClient.h>

extern NTPClient timeClient;

typedef void(*callBackFunc_t)(String cmd);

class ntpTimer
{
    public:
    ntpTimer();
    ~ntpTimer();
    ntpTimer(callBackFunc_t fn);
    void update();
    bool isActive();
    void cancel();
    void setTimer(size_t offsetMinutes,String cmdName);
    void setCallback(callBackFunc_t fn);
    String getTimerString();
    private:
    bool active;
    callBackFunc_t callbackFunction;
    int day;
    int hour;
    int minute;
    String cmd;
};



#endif