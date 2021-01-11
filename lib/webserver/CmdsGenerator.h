#ifndef CMDSGENERATOR_H
#define CMDSGENERATOR_H

#ifndef ESP32
#if filesystem == littlefs
#include <LittleFS.h>
#else
#include <SPIFFS.h>
#include <FS.h>
#define SPIFFS_USE_MAGIC
#endif
#else
#include <SPIFFS.h>
#include <FS.h>
#define SPIFFS_USE_MAGIC
#endif

#include <ircodes.h>

extern String getHtmlPrefix();
extern String getHtmlSuffix();

enum CmdsPart
{
    None,
    Prefix,
    Data,
    Suffix,
    Finished
};

class CmdsGenerator
{
private:
#if defined ESP8266 && filesystem == littlefs
    Dir dir;
#else
    File dir;
    File file;
#endif

public:
    CmdsGenerator(/* args */);
    ~CmdsGenerator();
    CmdsPart ActivePart;
    size_t readData(uint8_t *buffer, size_t maxlen);
    size_t offSet = 0;
    String tmpData;
    size_t count=0;
};

CmdsGenerator::CmdsGenerator(/* args */)
{
    ActivePart = CmdsPart::None;
}

CmdsGenerator::~CmdsGenerator()
{
}

size_t CmdsGenerator::readData(uint8_t *buffer, size_t maxlen)
{
    size_t bytesWritten=0;
    String dataToWrite;
    Serial.printf("-- read maxlen: %i, offSet: %i, state: %s \n", maxlen, offSet, String(ActivePart).c_str());
    switch (ActivePart)
    {
    case CmdsPart::None:
        ActivePart = CmdsPart::Prefix;
        tmpData = getHtmlPrefix();
        //tmpData.toCharArray((char *)buffer,maxlen,offSet);
        dataToWrite=tmpData.substring(offSet, offSet+maxlen);
        bytesWritten=dataToWrite.length();
//        Serial.printf("bytes: %i, data: %s",bytesWritten,dataToWrite.c_str());
        dataToWrite.getBytes(buffer,bytesWritten);
        offSet = offSet + bytesWritten;

        break;
    case CmdsPart::Prefix:
        if (offSet < tmpData.length())
        {
            //tmpData.toCharArray((char *)buffer,maxlen,offSet);
            dataToWrite=tmpData.substring(offSet, offSet+maxlen);
            bytesWritten=dataToWrite.length();
//            Serial.printf("bytes: %i, data: %s",bytesWritten,dataToWrite.c_str());
            dataToWrite.getBytes(buffer,bytesWritten);
            offSet = offSet + bytesWritten;
            //Prepare next Data
            if(offSet+bytesWritten>= tmpData.length())
            {
                ActivePart = CmdsPart::Data;
                tmpData = "";
#if defined ESP8266 && filesystem == littlefs
                dir = LittleFS.openDir("/");
#else
                dir = SPIFFS.open("/");
#endif
            }
        }
        else
        {
            ActivePart = CmdsPart::Data;
            tmpData = "";
#if defined ESP8266 && filesystem == littlefs
            dir = LittleFS.openDir("/");
#else
            dir = SPIFFS.open("/");
#endif
        }
        break;
    case CmdsPart::Data:
        if (tmpData == "")
        {
#if defined ESP8266 && filesystem == littlefs
            if (dir.next())
            {
                Serial.print(dir.fileName());
                Serial.print("  ");
                Serial.println(dir.fileSize());
                if (dir.fileName().endsWith(IRCODE_FILE_EXTENSION))
                {
                    int pointPos = dir.fileName().indexOf('.');
                    tmpData = F("<div class='field'><div class='buttons'><input class='button' type='submit' value='");
                    tmpData += dir.fileName().substring(0, pointPos);
                    tmpData += F("' name='button'/>");
                    tmpData += ("<a href='editcmd?cmd=" + dir.fileName().substring(0, pointPos) + "'>&nbsp&nbsp<i class='fa fa-edit'></i></a>");
                    tmpData += ("<a href='downloadcmd?cmd=" + dir.fileName().substring(0, pointPos) + "'>&nbsp&nbsp<i class='fa fa-download'></i></a>");
                    tmpData += ("<a href='delcmd?cmd=" + dir.fileName().substring(0, pointPos) + "'>&nbsp&nbsp<i class='fa fa-trash'></i></a>");
                    tmpData += ("<a href='timer?cmd=" + dir.fileName().substring(0, pointPos) + "'>&nbsp&nbsp<i class='fa fa-clock'></i></a>");
                    tmpData += F("</div></div>");
                    count++;
                    offSet=0;
                    dataToWrite=tmpData.substring(offSet, offSet+maxlen);
                    bytesWritten=dataToWrite.length();
//                    Serial.printf("bytes: %i, data: %s",bytesWritten,dataToWrite.c_str());
                    dataToWrite.getBytes(buffer,bytesWritten);
                    if(offSet+bytesWritten<tmpData.length())
                    {
                        offSet = offSet + bytesWritten;
                    }
                    else
                    {
                        //Cleanup for next run
                        tmpData="";
                        offSet=0;
                    }
                }
                else
                {
                    //return a space if is not a Command File
                    tmpData="";
                    offSet=0;
                    dataToWrite=" ";
                    bytesWritten=dataToWrite.length();
//                    Serial.printf("bytes: %i, data: %s",bytesWritten,dataToWrite.c_str());
                }
            }
            else
            {
                //no more files
                Serial.println("\n no more files");
                ActivePart = CmdsPart::Suffix;
                tmpData=F("</form>");
                tmpData+=F("<div class='field'><div class='label'>");
                tmpData+=(String(count));
                tmpData+=F("&nbsp CMD's found on device.</div></div>");
                tmpData+=F("<div class='field'><div class='buttons'><a class='button is-warning' href='/uploadcmd'>Upload CMD to Device</a></div></div>");
                tmpData+=F("<div class='field'><div class='buttons'><a class='button is-success' href='/editcmd'>Create CMD</a></div></div>");
                tmpData+=getHtmlSuffix()+" ";
                offSet = 0;
                dataToWrite=tmpData.substring(offSet, offSet+maxlen);
                bytesWritten=dataToWrite.length();
//                Serial.printf("bytes: %i, data: %s",bytesWritten,dataToWrite.c_str());
                dataToWrite.getBytes(buffer,bytesWritten);
                offSet = offSet + bytesWritten;
            }

#else
            file = dir.openNextFile();
            if (file)
            {
                String fileName = String(file.name());
                if (fileName.endsWith(IRCODE_FILE_EXTENSION))
                {
                    int pointPos = fileName.indexOf('.');
                    tmpData = (F("<div class='field'><div class='buttons'><input class='button' type='submit' value='"));
                    tmpData += (fileName.substring(1, pointPos));
                    tmpData += (F("' name='button'/>"));
                    tmpData += ("<a href='editcmd?cmd=" + fileName.substring(1, pointPos) + "'>&nbsp&nbsp<i class='fa fa-edit'></i></a>");
                    tmpData += ("<a href='downloadcmd?cmd=" + fileName.substring(1, pointPos) + "'>&nbsp&nbsp<i class='fa fa-download'></i></a>");
                    tmpData += ("<a href='delcmd?cmd=" + fileName.substring(1, pointPos) + "'>&nbsp&nbsp<i class='fa fa-trash'></i></a>");
                    tmpData += ("<a href='timer?cmd=" + fileName.substring(1, pointPos) + "'>&nbsp&nbsp<i class='fa fa-clock'></i></a>");
                    tmpData += (F("</div></div>"));
                    count++;
                    offSet=0;
                    dataToWrite=tmpData.substring(offSet, offSet+maxlen);
                    bytesWritten=dataToWrite.length();
//                    Serial.printf("bytes: %i, data: %s",bytesWritten,dataToWrite.c_str());
                    dataToWrite.getBytes(buffer,bytesWritten);
                    if(offSet+bytesWritten<tmpData.length())
                    {
                        offSet = offSet + bytesWritten;
                    }
                    else
                    {
                        //Cleanup for next run
                        tmpData="";
                        offSet=0;
                    }
                }
                else
                {
                    //return a space if is not a Command File
                    tmpData="";
                    offSet=0;
                    dataToWrite=" ";
                    bytesWritten=dataToWrite.length();
//                    Serial.printf("bytes: %i, data: %s",bytesWritten,dataToWrite.c_str());
                }
            }
            else
            {
                Serial.println("\n no more files");
                ActivePart = CmdsPart::Suffix;
                tmpData=F("</form>");
                tmpData+=F("<div class='field'><div class='label'>");
                tmpData+=(String(count));
                tmpData+=F("&nbsp CMD's found on device.</div></div>");
                tmpData+=F("<div class='field'><div class='buttons'><a class='button is-warning' href='/uploadcmd'>Upload CMD to Device</a></div></div>");
                tmpData+=F("<div class='field'><div class='buttons'><a class='button is-success' href='/editcmd'>Create CMD</a></div></div>");
                tmpData+=getHtmlSuffix()+" ";
                offSet = 0;
                dataToWrite=tmpData.substring(offSet, offSet+maxlen);
                bytesWritten=dataToWrite.length();
//                Serial.printf("bytes: %i, data: %s",bytesWritten,dataToWrite.c_str());
                dataToWrite.getBytes(buffer,bytesWritten);
                offSet = offSet + bytesWritten;
            }
#endif
        }
        else
        {
            if (offSet < tmpData.length())
            {
                dataToWrite=tmpData.substring(offSet, offSet+maxlen);
                bytesWritten=dataToWrite.length();
//                Serial.printf("bytes: %i, data: %s",bytesWritten,dataToWrite.c_str());
                dataToWrite.getBytes(buffer,bytesWritten);
                offSet = offSet + bytesWritten;
            }
            else
            {
                    dataToWrite=" ";
                    bytesWritten=dataToWrite.length();
//                    Serial.printf("bytes: %i, data: %s",bytesWritten,dataToWrite.c_str());
            }
        }
        break;
    case CmdsPart::Suffix:
        if (offSet < tmpData.length())
        {
            dataToWrite=tmpData.substring(offSet, offSet+maxlen);
            bytesWritten=dataToWrite.length();
//        Serial.printf("bytes: %i, data: %s",bytesWritten,dataToWrite.c_str());
            dataToWrite.getBytes(buffer,bytesWritten);
            offSet = offSet + bytesWritten;
        }
        else
        {
            ActivePart = CmdsPart::Finished;
            tmpData = "";
            count = 0;
        }
        break;
    case CmdsPart::Finished:
        return 0;
    default:
        break;
    }
    
    return bytesWritten;
}

#endif