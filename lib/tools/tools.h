// Generic tools for Config Management
#ifndef TOOLS_H
#define TOOLS_H

#include <Arduino.h>
#ifndef ESP32
#if filesystem==littlefs
    #include <LittleFS.h>
#else
    #include <FS.h>
    #define SPIFFS_USE_MAGIC
#endif
#else
    #include <FS.h>
    #include <SPIFFS.h>
    #define SPIFFS_USE_MAGIC
#endif
// Init Filesystem
// in Case of ESP8266 we have to init liitlefs because SPIFFS is deprecated
// You must add board_build.filesystem = littlefs and board_build.ldscript = eagle.flash.4m3m.ld to your platform.ini
// in case of ESP32 no special flags needed SPIFFS will be initialized 
void initFileSystem()
{
#if defined ESP8266 && filesystem == littlefs
    Serial.println("Mounting Flash...");
    if (!LittleFS.begin())
    {
        Serial.println("Failed to mount file system. Format it");
        if(!LittleFS.format())
        {
            Serial.println("Failed to format file system");
        }
        if(!LittleFS.begin())
        {
            Serial.println("Failed to mount file system after format");
        }
        return;
    }
#else
    Serial.println("Mounting SPIFFS...");
    if (!SPIFFS.begin())
    {
        Serial.println("Failed to mount file system. Format it.");
        if(!SPIFFS.format())
        {
          Serial.println("Failedto format file system");
        }
        if(!SPIFFS.begin())
        {
            Serial.println("Failed to mount file system after format");
        }
        return;
    }
#endif
}

String getESPDevName()
{
  char devName[30];
  #ifdef ESP8266
  snprintf(devName,30,"ESP-%08X",ESP.getChipId());
  #else
  uint32_t chipId=0;
  for(int i=0; i<17; i=i+8) {
	  chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
  snprintf(devName,30,"ESP-%08X",chipId);
  #endif
  return (String)devName;
}

#endif