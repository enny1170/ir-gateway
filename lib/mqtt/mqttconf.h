#ifndef MQTTCONF_H
#define MQTTCONF_H

#include <Arduino.h>
#include <ArduinoJson.h>

#if filesystem==littlefs
    #include <LittleFS.h>
#else
    #include <FS.h>
    #define SPIFFS_USE_MAGIC
#endif

#define MQTT_SIZE 210
#define MQTT_FILE_NAME "/mqtt.json"

String getESPDevName();
// global variables for PubSubClient
File mqttFile;
String mqttServer=".";
String mqttPort="1833";
String mqttPrefix=getESPDevName();
String mqttUser="";
String mqttPass="";

#ifndef CONFIG_H

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

#endif
/*
  Config File Helper Functions
*/

void writeMqttConfig(String server=".",String port="1883",String prefix=getESPDevName(),String user="",String pass="")
{
  const int capacity = JSON_OBJECT_SIZE(MQTT_SIZE);
  StaticJsonDocument<capacity> doc;

#if filesystem == littlefs
  mqttFile=LittleFS.open(MQTT_FILE_NAME,"w");
#else
  mqttFile=SPIFFS.open(MQTT_FILE_NAME,"w");
#endif

  if(server.length()>1 )
  {
    doc["mqttServer"]=server;
    doc["mqttPort"]=port;
    doc["mqttPrefix"]=prefix;
    doc["mqttUser"]=user;
    doc["mqttPass"]=pass;
  }
  else
  {
    doc["mqttServer"]=".";
    doc["mqttPort"]="1833";
    doc["mqttPrefix"]=getESPDevName();
    doc["mqttUser"]="";
    doc["mqttPass"]="";
  }
  
  serializeJson(doc,mqttFile);
  mqttFile.flush();
  mqttFile.close();
}

void readMqttConfig()
{
  const int capacity = JSON_OBJECT_SIZE(MQTT_SIZE);
  StaticJsonDocument<capacity> doc;

  Serial.println("Try to load MQTT-Config from file");

#if filesystem == littlefs
  mqttFile=LittleFS.open(MQTT_FILE_NAME,"r");
#else
  mqttFile=SPIFFS.open(MQTT_FILE_NAME,"r");
#endif
  DeserializationError err = deserializeJson(doc, mqttFile);
  mqttFile.close();
  if(err)
  {
    Serial.println("Unable to read Config Data (Json Error)");
    Serial.println(err.c_str());
  }
  else
  {
    mqttServer= doc["mqttServer"].as<String>();
    mqttPort=doc["mqttPort"].as<String>();
    mqttPrefix=doc["mqttPrefix"].as<String>();
    mqttUser=doc["mqttUser"].as<String>();
    mqttPass=doc["mqttPass"].as<String>();
  }
}

void checkMqttConfig()
{
  //check Config File is exists, or create one

#if filesystem == littlefs
  if(!LittleFS.exists(MQTT_FILE_NAME))
  {
    Serial.println("Try to create Config File");
    writeMqttConfig("");
  }
#else
  if(!SPIFFS.exists(MQTT_FILE_NAME))
  {
    Serial.println("Try to create Config File");
    writeMqttConfig("");
  }
#endif


}

#ifndef CONFIG_H

String getESPDevName()
{
  char devName[30];
  snprintf(devName,30,"ESP-%08X",ESP.getChipId());
  return (String)devName;
}

#endif

#endif