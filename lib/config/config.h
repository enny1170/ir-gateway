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

#define CONFIG_SIZE 145
#define CONFIG_FILE_NAME "/config.json"

String getESPDevName();

// Variables for Config
File configFile;
String ssid;
String passwd;
String deviceName=getESPDevName();

void initFileSystem()
{
#if filesystem == littlefs
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
        Serial.println("Failed to mount file system");
        return;
    }
#endif
}

/*
  Config File Helper Functions
*/

void writeConfig(String ssid,String passwd,String device=getESPDevName())
{
  const int capacity = JSON_OBJECT_SIZE(CONFIG_SIZE);
  StaticJsonDocument<capacity> doc;

#if filesystem == littlefs
  configFile=LittleFS.open(CONFIG_FILE_NAME,"w");
#else
  configFile=SPIFFS.open(CONFIG_FILE_NAME,"w");
#endif

  if(ssid.length()>1 && passwd.length()>0)
  {
    doc["ssid"]=ssid;
    doc["passwd"]=passwd;
    doc["deviceName"]=deviceName;
  }
  else
  {
    doc["ssid"]=".";
    doc["passwd"]=".";
    doc["deviceName"]=deviceName;
  }
  
  serializeJson(doc,configFile);
  configFile.flush();
  configFile.close();
}

void readConfig()
{
  const int capacity = JSON_OBJECT_SIZE(CONFIG_SIZE);
  StaticJsonDocument<capacity> doc;

  Serial.println("Try to load WiFi-Config from file");

#if filesystem == littlefs
  configFile=LittleFS.open(CONFIG_FILE_NAME,"r");
#else
  configFile=SPIFFS.open(CONFIG_FILE_NAME,"r");
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
    deviceName=doc["deviceName"].as<String>();
  }
}

void checkConfig()
{
  //check Config File is exists, or create one

#if filesystem == littlefs
  if(!LittleFS.exists(CONFIG_FILE_NAME))
  {
    Serial.println("Try to create Config File");
    writeConfig("","");
  }
#else
  if(!SPIFFS.exists(CONFIG_FILE_NAME))
  {
    Serial.println("Try to create Config File");
    writeConfig("","");
  }
#endif


}

String getESPDevName()
{
  char devName[30];
  snprintf(devName,30,"ESP-%08X",ESP.getChipId());
  return (String)devName;
}

#endif