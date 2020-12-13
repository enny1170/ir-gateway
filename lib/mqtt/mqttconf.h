#ifndef MQTTCONF_H
#define MQTTCONF_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <tools.h>

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

#define MQTT_SIZE 210
#define MQTT_FILE_NAME "/mqtt.json"

// global variables for MQTTClient
File mqttFile;
String mqttServer=".";
String mqttPort="1833";
String mqttPrefix=getESPDevName();
String mqttUser="";
String mqttPass="";

/*
  Config File Helper Functions
*/

void writeMqttConfig(String server=".",String port="1883",String prefix=getESPDevName(),String user="",String pass="")
{
  const int capacity = JSON_OBJECT_SIZE(MQTT_SIZE);
  StaticJsonDocument<capacity> doc;

#if defined ESP8266 && filesystem == littlefs
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
    doc["mqttPort"]="1883";
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

#if defined ESP8266 && filesystem == littlefs
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

#if defined ESP8266 && filesystem == littlefs
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

#endif