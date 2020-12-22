#ifndef IRCODES_H
#define IRCODES_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <ircode.h>
#include <cppQueue.h>

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
#define IRCODE_FILE_EXTENSION ".jcmd"
#define	IMPLEMENTATION	FIFO

#ifndef OFFSET_START
#define OFFSET_START kStartOffset // Usual rawbuf entry to start processing from.
#endif

File irCodeFile;

/***********************************************************************************************************************************************************************************
 * 
 * IR Support 
 * 
 * *********************************************************************************************************************************************************************************/
// Hardware Defines
#define IR_PORT 0
#define IR_PORT_INVERT false
#define IR_RECEIVER_PORT 2
#define IR_RECEIVE_WAIT_TIME 30000

// IR Variables
char ir[1024];
String irProtocoll;
String irValue;
String irLength;
String irAddress;
String irCommand;
String irCode;
String irReceiveState;
IRrecv irReceiver(IR_RECEIVER_PORT);
decode_results irDecoded;
bool irReceiverState=false;
bool irReceiveFinished=false;
unsigned long receiverStart=0;

typedef struct strRec {
	char data[1024];
    strRec()
    {}
    strRec(String input)
    {
        input.toCharArray(data,1024,0);
    }
    String toString()
    {
        return String(data);
    }
} Rec;

//Create Que for Strings with max 5 Entrys
cppQueue irSendQueue (sizeof(Rec), 5, IMPLEMENTATION);

/*
   Gibt die CPU Takte zurück, die seit dem Neustart vergangen sind.
   läuft ca. alle 53 Sekunden über
*/
#define RSR_CCOUNT(r) __asm__ __volatile__("rsr %0,ccount" \
                                           : "=a"(r))
static inline uint32_t get_ccount()
{
  uint32_t ccount;
  RSR_CCOUNT(ccount);
  return ccount;
}

/***********************************************************************************************************************************************************************************
 * IR Receive non blocking
 * 
 * this will be called from Webserver, and must integrated in Loop with fromLopp=true
 * *********************************************************************************************************************************************************************************/

void receive_ir_nonblock(bool fromLoop=false)
{
  if(fromLoop)
  {
      //Called from Loop, if enabled check Timout and result
      if(irReceiverState)
      {
          // Receive is enabled check the results
          if(irReceiver.decode(&irDecoded))
          {
              irReceiveFinished=true;
              irReceiveState="IR CMD Received";
              irProtocoll=typeToString(irDecoded.decode_type, false);
              irValue=uint64ToString(irDecoded.value, HEX);
              irAddress=irDecoded.address;
              irLength=irDecoded.rawlen;
              irCommand=irDecoded.command;
              irCode="";
              int freq = 38000;
                if (typeToString(irDecoded.decode_type, false).equals("SONY"))
                    freq = 40000;
                irCode += freq;
                for (int i = OFFSET_START; i < irDecoded.rawlen; i++)
                {
                    irCode += ",";
                    irCode += (int)(((irDecoded.rawbuf[i] * RAWTICK) * freq) / 1000000);
                }
                if (irDecoded.rawlen % 2 == 0)
                {
                    irCode += ",1";
                }
                //printout the captured Infos
                Serial.println(irReceiveState);
                Serial.print("Protocoll: ");
                Serial.println(irProtocoll);
                Serial.print("Value: ");
                Serial.println(irValue);
                Serial.print("Address: ");
                Serial.println(irAddress);
                Serial.print("Length: ");
                Serial.println(irLength);
                Serial.print("Command: ");
                Serial.println(irCommand);
                Serial.print("Code: ");
                Serial.println(irCode);
              irReceiver.resume();      // Receive the next value
              irReceiver.disableIRIn(); // Stopps the receiver
              irReceiverState=false;
          }
          else if(millis() < receiverStart + IR_RECEIVE_WAIT_TIME)
          {
              //Timeout stop receiver
              irReceiveFinished=true;
              irReceiveState="Timeout no IR CMD Received";
              Serial.println(irReceiveState);
              irReceiver.resume();      // Receive the next value
              irReceiver.disableIRIn(); // Stopps the receiver
              irReceiverState=false;
          }
      }
  }
  else
  {
      // is called by Webserver, enable irReceiver if it not enabled and finished is false
      if(!irReceiverState && !irReceiveFinished)
      {
          irReceiverState=true;
          irReceiveState="IR Receiver enabled, waiting for Data";
          irReceiver.enableIRIn();
      }
  }

}

/**************************************************************************************************************************************************************************
 * IRCode Handler without HTTP Server
 * is used by CMD-Site and MQTT but blocking
 * So you have to add the code to the queue and send it on next loop
***************************************************************************************************************************************************************************/
void handleIrCode(String code)
{

  pinMode(IR_PORT, OUTPUT);

  if (code.length() > 0)
  {
    (code + ",0").toCharArray(ir, 1024);

    char *p; //Pointer in array
    unsigned int frequence = strtol(ir, &p, 10);
    p++; //do not interprete comma in string
    unsigned int pulses = strtol(p, &p, 10);

    bool burst = true; //start IR Lighting

    unsigned int startTicks;
    unsigned int halfPeriodTicks = 40000000 / frequence;
    while (pulses != 0)
    {
      RSR_CCOUNT(startTicks);
      for (unsigned int i = 0; i < pulses * 2; i++)
      {
        if (IR_PORT_INVERT)
          digitalWrite(IR_PORT, (((i & 1) == 1) && burst) ? LOW : HIGH);
        else
          digitalWrite(IR_PORT, (((i & 1) == 1) && burst) ? HIGH : LOW);
        while (get_ccount() < startTicks + i * halfPeriodTicks)
        {
        } //Wait for ticks
      }
      burst = !burst;
      p++; //do not interprete comma in string
      pulses = strtol(p, &p, 10);
    }
    digitalWrite(IR_PORT, IR_PORT_INVERT ? HIGH : LOW); //Turn IR Light off
  }
  else
  {
    Serial.println("Unknown Code Length");
    return;
  }
  Serial.println("IrCode send");
}

/********************************************************************************************************************************************************************
 * Add a IrCode to the Queue, used by HTTP and MQTT to create a non blocking IrCode sending, for Async implementations
 * ******************************************************************************************************************************************************************/
void addIrCodeToQueue(String code)
{
    if(!irSendQueue.isFull())
    {
        Rec newRec(code);
        irSendQueue.push(&newRec);
        Serial.print("Add Code to Queue: ");
        Serial.println(newRec.toString());
    }
    else
    {
        Serial.println("Overflow for irSendQueue detected.");
    }
}

/*******************************************************************************************************************************************************************
 * Send IrCodes saved into the Queue. This must called from Loop
 * *****************************************************************************************************************************************************************/
void sendIrCodeFromQueue()
{
   
    if(!irSendQueue.isEmpty())
    {
        Rec tmpCode;
        //String tmpCode;
        irSendQueue.pop(&tmpCode);
        Serial.printf("Send IR-Code from Queue: %s\n",tmpCode.toString().c_str());
        if (tmpCode.toString().length()>100)
        {
            handleIrCode(tmpCode.toString());
        }
        else
        {
            Serial.println("Do not Send IR-Code, code is to short");
        }
        
    }
}

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

/******************************************************************************************************************************************
 *  Create IRCode Object by members
 * ****************************************************************************************************************************************/
void writeIrCmd(String cmd,String description,String code)
{
  const int capacity = JSON_OBJECT_SIZE(IRCODE_SIZE);
  StaticJsonDocument<capacity> doc;

#if defined ESP8266 && filesystem == littlefs
  irCodeFile=LittleFS.open("/"+cmd+IRCODE_FILE_EXTENSION,"w");
#else
  irCodeFile=SPIFFS.open("/"+cmd+IRCODE_FILE_EXTENSION,"w");
#endif

  doc["cmd"]=cmd;
  doc["description"]=description;
  doc["code"]=code;  

  serializeJson(doc,irCodeFile);
  irCodeFile.flush();
  irCodeFile.close();

}

/******************************************************************************************************************************************
 * Write IRCode Object to File
 * ****************************************************************************************************************************************/
void writeIrCmd(IRcode ircode)
{
  const int capacity = JSON_OBJECT_SIZE(IRCODE_SIZE);
  StaticJsonDocument<capacity> doc;

#if defined ESP8266 && filesystem == littlefs
  irCodeFile=LittleFS.open("/"+ircode.Cmd+IRCODE_FILE_EXTENSION,"w");
#else
  irCodeFile=SPIFFS.open("/"+ircode.Cmd+"cmd","w");
#endif

  doc["cmd"]=ircode.Cmd;
  doc["description"]=ircode.Description;
  doc["code"]=ircode.Code;  

  serializeJson(doc,irCodeFile);
  irCodeFile.flush();
  irCodeFile.close();

}

/********************************************************************************************************************************************
 *  Get IRCode Object from .cmd File
 * ******************************************************************************************************************************************/
IRcode readIrCmd(String cmd)
{
    IRcode retval;
    const int capacity = JSON_OBJECT_SIZE(IRCODE_SIZE);
    StaticJsonDocument<capacity> doc;
    Serial.printf("Get IrCmd for '%s'\n.",cmd.c_str());
    if (!cmd.endsWith(IRCODE_FILE_EXTENSION))
    {
        // Commandname given
#if defined ESP8266 && filesystem == littlefs
        String cmdFilename="/" + cmd + IRCODE_FILE_EXTENSION;
        Serial.printf("Generated Filename:%s\n",cmdFilename.c_str());
        if (LittleFS.exists(cmdFilename.c_str()))
        {
            irCodeFile = LittleFS.open(cmdFilename.c_str(), "r");
        }
        else
        {
            retval.Cmd = cmd;
            Serial.printf("Data for CMD %s not found.\n", cmd.c_str());
            return retval;
        }

#else
        if (SPIFFS.exists(String("/" + cmd + IRCODE_FILE_EXTENSION).c_str()))
        {
            irCodeFile = SPIFFS.open("/" + cmd + IRCODE_FILE_EXTENSION, "r");
        }
        else
        {
            retval.Cmd = cmd;
            Serial.printf("Data for CMD %s not found.\n", cmd.c_str());
            return retval;
        }

#endif
    }
    else
    {
#if defined ESP8266 && filesystem == littlefs
        if (LittleFS.exists(cmd.c_str()))
        {
            irCodeFile = LittleFS.open(cmd.c_str(), "r");
        }
        else
        {
            retval.Cmd = cmd;
            Serial.printf("Data for CMD %s not found.\n", cmd.c_str());
            return retval;
        }

#else
        if (SPIFFS.exists(cmd))
        {
            irCodeFile = SPIFFS.open(cmd, "r");
        }
        else
        {
            retval.Cmd = cmd;
            Serial.printf("Data for CMD %s not found.\n", cmd.c_str());
            return retval;
        }

#endif
    }

    DeserializationError err = deserializeJson(doc, irCodeFile);
    irCodeFile.close();
    if (err)
    {
        Serial.println("Unable to read IrCode Data (Json Error)");
        Serial.println(err.c_str());
    }
    else
    {
        retval.Cmd = doc["cmd"].as<String>();
        retval.Description = doc["description"].as<String>();
        retval.Code = doc["code"].as<String>();
    }

    return retval;
}

/********************************************************************************************************************
 * List available CMD'S as string for serial Console
 * ******************************************************************************************************************/
String listCmds()
{
    String retval="";
    Serial.println("listCmds");

#if defined ESP8266 && filesystem == littlefs
    Dir dir = LittleFS.openDir("/");
    while(dir.next())
    {
        if(dir.isFile() && dir.fileName().endsWith(IRCODE_FILE_EXTENSION))
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
        if(fileName.endsWith(IRCODE_FILE_EXTENSION))
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

/*****************************************************************************************************************************************
 * Get available CMD's as comma seperated string
 * ***************************************************************************************************************************************/
String getCmds()
{
    String retval="";
    Serial.println("getCmds");
#if defined ESP8266 && filesystem == littlefs
    Dir dir = LittleFS.openDir("/");
    while(dir.next())
    {
        if(dir.isFile() && dir.fileName().endsWith(IRCODE_FILE_EXTENSION))
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
        if(fileName.endsWith(IRCODE_FILE_EXTENSION))
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

/***********************************************************************************************************************************
 *  Build a HTML Form with Buttons for each CMD
 * *********************************************************************************************************************************/
String buildCmdPage()
{
    String retval="";
    int count=0;
    Serial.println("buildCmdPage");
    retval += F("<form method='GET' action='cmd' >");
    //"</form>");
#if defined ESP8266 && filesystem == littlefs
    Dir dir = LittleFS.openDir("/");
    while(dir.next())
    {
        if(dir.isFile() && dir.fileName().endsWith(IRCODE_FILE_EXTENSION))
        {
            int pointPos=dir.fileName().indexOf('.');
            retval += F("<div class='field'><div class='buttons'><input class='button' type='submit' value='");
            retval += dir.fileName().substring(0,pointPos);
            retval += F("' name='button'");
            retval += F("/>");
            retval += "<a href='editcmd?cmd="+dir.fileName().substring(0,pointPos)+"'>&nbsp<i class='fa fa-edit'></i></a>";
            retval += "<a href='downloadcmd?cmd="+dir.fileName().substring(0,pointPos)+"'>&nbsp<i class='fa fa-download'></i></a>";
            retval += "<a href='delcmd?cmd="+dir.fileName().substring(0,pointPos)+"'>&nbsp<i class='fa fa-trash'></i></a>";
            retval += "</div></div>";
            count ++;
        }
    }
    retval +=F("</form>");
#else
    File dir = SPIFFS.open("/");
    File file=dir.openNextFile();
    while(file)
    {
        String fileName=String(file.name());
        if(fileName.endsWith(IRCODE_FILE_EXTENSION))
        {
            int pointPos=fileName.indexOf('.');
            retval += F("<div class='field'><div class='buttons'><input class='button' type='submit' value='");
            retval += fileName.substring(1,pointPos);
            retval += F("' name='button'");
            retval += F("/>");
            retval += "<a href='editcmd?cmd="+fileName.substring(1,pointPos)+"'><i class='fa fa-edit'></i></a>";
            retval += "<a href='delcmd?cmd="+fileName.substring(1,pointPos)+"'><i class='fa fa-trash'></i></a>";
            retval += "</div></div>";
            count ++;
        }
        file=dir.openNextFile();
    }
    retval +=F("</form>");
#endif
    retval +="<div class='field'><div class='label'>"+String(count)+"&nbsp CMD's found on device.</div></div>";
    retval +="<div class='field'><div class='buttons'><a class='button is-warning' href='/uploadcmd'>Upload CMD to Device</a></div></div>";

    return retval;

}

/***********************************************************************************************************************************
 *  Build a HTML Form for editing a CMD
 * *********************************************************************************************************************************/
String buildCmdEditPage(String cmd)
{
    String retval="";
    IRcode tmpCode=readIrCmd(cmd);
    Serial.println("buildCmdEditPage");
    retval += F("<form method='Post' action='/cmd' >");
    retval += "<input type='hidden' name='orgname' value='"+tmpCode.Cmd+"' />";
    retval += "<div class='field'><div class='label'>CMD-Name*:</div><div class='control'><input class='input' type='text' name='cmdname' value='"+tmpCode.Cmd+"'></div></div>";
    retval += "<div class='field'><div class='label'>Description:</div><div class='control'><input class='input' type='text' name='cmddescription' value='"+tmpCode.Description+"'></div></div>";
    retval += "<div class='field'><div class='label'>Code*:</div><div class='control'><input class='input' type='text' name='code' value='"+tmpCode.Code+"'></div></div>";
    retval += "<div class='field'><div class='buttons'><input class='button' type='submit' value='Save'/>&nbsp<a href='delcmd?cmd="+tmpCode.Cmd+"'><i class='fa fa-trash'></i></a></div></div></form>";
    return retval;
}

/**********************************************************************************************************************************
 *  Delete CMD-File from Flash Storage
 * ********************************************************************************************************************************/
void deleteCmd(String cmd)
{
    Serial.printf("Delete IrCmd for '%s'\n.",cmd.c_str());
    if (!cmd.endsWith(IRCODE_FILE_EXTENSION))
    {
        // Commandname given
#if defined ESP8266 && filesystem == littlefs
        String cmdFilename="/" + cmd + IRCODE_FILE_EXTENSION;
        Serial.printf("Generated Filename:%s\n",cmdFilename.c_str());
        if (LittleFS.exists(cmdFilename.c_str()))
        {
            LittleFS.remove(cmdFilename.c_str());
        }
#else
        if (SPIFFS.exists(String("/" + cmd + IRCODE_FILE_EXTENSION).c_str()))
        {
            SPIFFS.remove("/" + cmd + IRCODE_FILE_EXTENSION);
        }
#endif
    }
    else
    {
#if defined ESP8266 && filesystem == littlefs
        if (LittleFS.exists(cmd.c_str()))
        {
            LittleFS.remove(cmd.c_str());
        }
#else
        if (SPIFFS.exists(cmd))
        {
            SPIFFS.remove(cmd);
        }
#endif
    }
}

String getCmdFileName(String cmd)
{
    String retVal;
    String cmdFilename;
    if (!cmd.endsWith(IRCODE_FILE_EXTENSION))
    {
        // Commandname given
#if defined ESP8266 && filesystem == littlefs
        cmdFilename="/" + cmd + IRCODE_FILE_EXTENSION;
        Serial.printf("Generated Filename:%s\n",cmdFilename.c_str());
        if (LittleFS.exists(cmdFilename.c_str()))
        {
            retVal=cmdFilename;
        }
#else
        if (SPIFFS.exists(cmdFilename))
        {
            retVal=cmdFilename;
        }
#endif
    }
    else
    {
#if defined ESP8266 && filesystem == littlefs
        cmdFilename="/"+cmd;
        if (LittleFS.exists(cmdFilename.c_str()))
        {
            retVal=cmdFilename;
        }
#else
        if (SPIFFS.exists(cmdFilename))
        {
            retVal=cmdFilename;
        }
#endif
    }
    return retVal;
}

#endif