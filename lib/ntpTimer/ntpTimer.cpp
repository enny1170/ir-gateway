// Implementation NTPTimer
#include <ntpTimer.h>

ntpTimer::ntpTimer()
{

}

ntpTimer::ntpTimer(callBackFunc_t fn)
{
    callbackFunction=fn;
}

void ntpTimer::setTimer(size_t offsetMinutes,String cmdName)
{
    int tmpDay=0;
    int tmpMinute=0;
    int tmpHour=0;
    timeClient.update();
    tmpDay=timeClient.getDay();
    tmpHour=timeClient.getHours();
    tmpMinute=timeClient.getMinutes();
    for (size_t i = 0; i < offsetMinutes; i++)
    {
        if(tmpMinute+1 <60)
        {
            tmpMinute++;
        }
        else
        {
            //Minute overflow
            tmpMinute=0;
            if(tmpHour+1<24)
            {
                tmpHour++;
            }
            else
            {
                //Day Overflow will be ignored
                tmpHour=0;
                tmpDay++;
            }
        }
    }
    day=tmpDay;
    hour=tmpHour;
    minute=tmpMinute;
    cmd=cmdName;
    active=true;
    Serial.printf("\nTimerActive: %s, TimerCmd: %s, TimerTime %i:%i\n",String(active).c_str(),cmd.c_str(),hour,minute);

}

bool ntpTimer::isActive()
{
    return active;
}

void ntpTimer::cancel()
{
    active=false;
}

String ntpTimer::getTimerString()
{
    String retval="";
    if(active)
    {
        retval="Timer active for Command '" + cmd + "' on " + String(hour) + ":" +String(minute)+".";
    }
    else
    {
        retval="Timer not active.";
    }
    return retval;
}

void ntpTimer::update()
{
    timeClient.update();
    if(active)
    {
        // Serial.printf("\nNtpTime: %s, TimerActive: %s, TimerCmd: %s, TimerTime %i:%i\n",
        // timeClient.getFormattedTime().c_str(),String(active).c_str(),cmd.c_str(),hour,minute);
        if(timeClient.getDay()== day && timeClient.getHours()==hour && timeClient.getMinutes()>minute)
        {
            if(callbackFunction!=NULL)
            {
                callbackFunction(cmd);
            }
            active=false;
        }
    }
}

void ntpTimer::setCallback(callBackFunc_t fn)
{
    callbackFunction=fn;
}

ntpTimer::~ntpTimer()
{

}
