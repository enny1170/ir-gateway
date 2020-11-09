#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>

#if filesystem==littlefs
    #include <LittleFS.h>
#else
    #include <FS.h>
    #define SPIFFS_USE_MAGIC
#endif

// Variables for Config
File configFile;
String ssid;
String passwd;

void initFileSystem()
{
#if filesystem == littlefs
    Serial.println("Mounting SPIFFS...");
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
        Serial.println("Failed to mount file system");
        return;
    }
#endif
}

/*
  Config File Helper Functions
*/

void writeConfig(String ssid,String passwd)
{
  const int capacity = JSON_OBJECT_SIZE(93);
  StaticJsonDocument<capacity> doc;

#if filesystem == littlefs
  configFile=LittleFS.open("/config.json","w");
#else
  configFile=SPIFFS.open("/config.json","w");
#endif

  if(ssid.length()>1 && passwd.length()>0)
  {
    doc["ssid"]=ssid;
    doc["passwd"]=passwd;
  }
  else
  {
    doc["ssid"]=".";
    doc["passwd"]=".";
  }
  
  serializeJson(doc,configFile);
  configFile.flush();
  configFile.close();
}

void readConfig()
{
  const int capacity = JSON_OBJECT_SIZE(93);
  StaticJsonDocument<capacity> doc;

  Serial.println("Try to load Config from file");

#if filesystem == littlefs
  configFile=LittleFS.open("/config.json","r");
#else
  configFile=SPIFFS.open("/config.json","r");
#endif

  DeserializationError err = deserializeJson(doc, configFile);
  configFile.close();
  if(err)
  {
    Serial.println("Unable to read Config Data (Json Error)");
    Serial.println(err.c_str());
  }
  else
  {
    ssid= doc["ssid"].as<String>();
    passwd= doc["passwd"].as<String>();
  }
}

void checkConfig()
{
  //check Config File is exists, or create one

#if filesystem == littlefs
  if(!LittleFS.exists("/config.json"))
  {
    Serial.println("Try to create Config File");
    writeConfig("","");
  }
#else
  if(!SPIFFS.exists("/config.json"))
  {
    Serial.println("Try to create Config File");
    writeConfig("","");
  }
#endif


}
#endif