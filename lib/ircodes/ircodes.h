#ifndef IRCODES_H
#define IRCODES_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ircode.h>

#ifndef ESP32
#if filesystem==littlefs
    #include <LittleFS.h>
#else
    #include <FS.h>
    #define SPIFFS_USE_MAGIC
#endif
#else
    #include <FS.h>
    #define SPIFFS_USE_MAGIC
#endif

#define IRCODE_SIZE 309
#define IRCODE_FILE_NAME "/ircodes.json"

File irCodeFile;

#ifndef CONFIG_H

void initFileSystem()
{
#if defined ESP8266 && filesystem == littlefs
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
  Data File Helper Functions
*/

// Create IRCode Object by members
void writeIrCmd(String cmd,String description,String code,String gcCode="")
{
  const int capacity = JSON_OBJECT_SIZE(IRCODE_SIZE);
  StaticJsonDocument<capacity> doc;

#if defined ESP8266 && filesystem == littlefs
  irCodeFile=LittleFS.open("/"+cmd+".cmd","w");
#else
  irCodeFile=SPIFFS.open("/"+cmd+"cmd","w");
#endif

  doc["cmd"]=cmd;
  doc["description"]=description;
  doc["code"]=code;
  doc["gccode"]=gcCode;  

  serializeJson(doc,irCodeFile);
  irCodeFile.flush();
  irCodeFile.close();

}

//Write IRCode Object to File
void writeIrCmd(IRcode ircode)
{
  const int capacity = JSON_OBJECT_SIZE(IRCODE_SIZE);
  StaticJsonDocument<capacity> doc;

#if defined ESP8266 && filesystem == littlefs
  irCodeFile=LittleFS.open("/"+ircode.Cmd+".cmd","w");
#else
  irCodeFile=SPIFFS.open("/"+ircode.Cmd+"cmd","w");
#endif

  doc["cmd"]=ircode.Cmd;
  doc["description"]=ircode.Description;
  doc["code"]=ircode.Code;
  doc["gccode"]=ircode.GcCode;  

  serializeJson(doc,irCodeFile);
  irCodeFile.flush();
  irCodeFile.close();

}

// Get IRCode Object from .cmd File
IRcode readIrCmd(String cmd)
{
    IRcode retval;
    const int capacity = JSON_OBJECT_SIZE(IRCODE_SIZE);
    StaticJsonDocument<capacity> doc;

    if(!cmd.endsWith(".cmd"))
    {
        // Commandname given
#if defined ESP8266 && filesystem == littlefs
    if(LittleFS.exists("/"+cmd+".cmd"))
    {
        irCodeFile=LittleFS.open("/"+cmd+".cmd","r");
    }
    else
    {
        retval.Cmd=cmd;
        Serial.printf("Data for CMD %s not found.\n",cmd.c_str());
        return retval;
    }
    
#else
    if(SPIFFS.exists(String("/"+cmd+".cmd").c_str()))
    {
        irCodeFile=SPIFFS.open("/"+cmd+".cmd","r");
    }
    else
    {
        retval.Cmd=cmd;
        Serial.printf("Data for CMD %s not found.\n",cmd.c_str());
        return retval;
    }
    
#endif
    }
    else
    {
#if defined ESP8266 && filesystem == littlefs
    if(LittleFS.exists(cmd))
    {
        irCodeFile=LittleFS.open(cmd,"r");
    }
    else
    {
        retval.Cmd=cmd;
        Serial.printf("Data for CMD %s not found.\n",cmd.c_str());
        return retval;
    }
    
#else
    if(SPIFFS.exists(cmd))
    {
        irCodeFile=SPIFFS.open(cmd,"r");
    }
    else
    {
        retval.Cmd=cmd;
        Serial.printf("Data for CMD %s not found.\n",cmd.c_str());
        return retval;
    }
    
#endif
    }
    
  DeserializationError err = deserializeJson(doc, irCodeFile);
  irCodeFile.close();
  if(err)
  {
    Serial.println("Unable to read IrCode Data (Json Error)");
    Serial.println(err.c_str());
  }
  else
  {
    retval.Cmd=doc["cmd"].as<String>();
    retval.Description=doc["description"].as<String>();
    retval.Code=doc["code"].as<String>();
    retval.GcCode=doc["gccode"].as<String>();
  }


    return retval;
}

String listCmds()
{
    String retval="";
    Serial.println("listCmds");

#if defined ESP8266 && filesystem == littlefs
    Dir dir = LittleFS.openDir("/");
    while(dir.next())
    {
        if(dir.isFile() && dir.fileName().endsWith(".cmd"))
        {
            int pointPos=dir.fileName().indexOf('.');
            retval += dir.fileName();
            retval += " - ";
            retval += dir.fileName().substring(1,pointPos);
            retval += "\n";
        }
    }
#else
    File dir = SPIFFS.open("/");
    File file=dir.openNextFile();
    while(file)
    {
        String fileName=String(file.name());
        if(fileName.endsWith(".cmd"))
        {
            int pointPos=fileName.indexOf('.');
            retval += fileName;
            retval += " - ";
            retval += fileName.substring(1,pointPos);
            retval += "\n";
        }
        file=dir.openNextFile();
    }
    dir.close();
#endif
   
    return retval;

}

String getCmds()
{
    String retval="";
    Serial.println("getCmds");
#if defined ESP8266 && filesystem == littlefs
    Dir dir = LittleFS.openDir("/");
    while(dir.next())
    {
        if(dir.isFile() && dir.fileName().endsWith(".cmd"))
        {
            int pointPos=dir.fileName().indexOf('.');
            retval += dir.fileName().substring(1,pointPos);
            retval += ", ";
        }
    }
#else
    File dir = SPIFFS.open("/");
    File file=dir.openNextFile();
    while(file)
    {
        String fileName=String(file.name());
        if(fileName.endsWith(".cmd"))
        {
            int pointPos=fileName.indexOf('.');
            retval += fileName.substring(1,pointPos);
            retval += ", ";
        }
        file=dir.openNextFile();
    }
    dir.close();
#endif
    return retval;
}

// Build a HTML Form with Buttons for each CMD
String buildCmdPage()
{
    String retval="";
    Serial.println("buildCmdPage");
    retval += F("<form method='GET' action='cmd' >");
    //"</form>");
#if defined ESP8266 && filesystem == littlefs
    Dir dir = LittleFS.openDir("/");
    while(dir.next())
    {
        if(dir.isFile() && dir.fileName().endsWith(".cmd"))
        {
            int pointPos=dir.fileName().indexOf('.');
            retval += F("<div class='field'><div class='buttons'><input class='button' type='submit' value='");
            retval += dir.fileName().substring(0,pointPos-1);
            retval += F("'/></div>");
        }
    }
    retval +=F("</form>");
#else
    File dir = SPIFFS.open("/");
    File file=dir.openNextFile();
    while(file)
    {
        String fileName=String(file.name());
        if(fileName.endsWith(".cmd"))
        {
            int pointPos=fileName.indexOf('.');
            retval += F("<div class='field'><div class='buttons'><input class='button' type='submit' value='");
            retval += fileName.substring(0,pointPos-1);
            retval += F("'/></div>");
        }
        file=dir.openNextFile();
    }
    retval +=F("</form>");
#endif

    return retval;

}

#endif