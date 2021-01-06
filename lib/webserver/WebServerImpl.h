#ifndef WEBSERVERIMPL_H
#define WEBSERVERIMPL_H

#include <WiFiClient.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <Arduino.h>
#include <mqttconf.h>
#include <mqttimpl.h>
#include <ircodes.h>

AsyncWebServer server(80);
String htmlcontent;
const char* PARAM_MESSAGE = "message";

//this method implements the IrCode Sending without HTTP
//is implemented in main.cpp this is only a link for it
//should be moved later
extern void handleIrCode(String code);
extern String handleReceiveIr();
void serial_print_HttpInfo();

/*
    Returns HTML-Prefix for each Page
*/
String getHtmlPrefix()
{
    /*return F("<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'> \
        <title>ESP-RcOid</title><link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/bulma@0.8.2/css/bulma.min.css'> \
        <script defer src='https://use.fontawesome.com/releases/v5.3.1/js/all.js'></script> \
        <script src = 'https://ajax.googleapis.com/ajax/libs/jquery/2.1.1/jquery.min.js'></script> \
        <script>$(document).ready(function(){$('.navbar-burger').click(function() {$('.navbar-burger').toggleClass('is-active'); \
        $('.navbar-menu').toggleClass('is-active');});});</script></head> \
        <body><nav class = 'navbar has-shadow'><div class = 'navbar-brand'><a class = 'navbar-item' href = '/'>ESP-RcOid</a> \
        <a role='button' class='navbar-burger' aria-label='menu' aria-expanded='true' ><span></span><span></span><span></span> \
        </a></div><div class='navbar-menu'><div class='navbar-start'><div class='navbar-item'> \
        <a class='navbar-item' href='wifi.html'>WiFi Settings</a><a class='navbar-item' href='mqtt.html'>MQTT Settings</a> \
        <a class='navbar-item' href='reset.html'>System Reset</a><hr><a class='navbar-item' href='docu.html'>Readme</a></div> \
        </div></div><div class='navbar-end'><!-- <div class='navbar-link'>Github</div> --></div></nav><section class='section'> \
        <div class='container'><div class='content'>");
    */
    return F("<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'> \
        <title>ESP-RcOid</title><link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/bulma@0.8.2/css/bulma.min.css'> \
        <script defer src='https://use.fontawesome.com/releases/v5.3.1/js/all.js'></script> \
        <script src = 'https://ajax.googleapis.com/ajax/libs/jquery/2.1.1/jquery.min.js'></script> \
        <script>$(document).ready(function(){$('.navbar-burger').click(function() {$('.navbar-burger').toggleClass('is-active'); \
        $('.navbar-menu').toggleClass('is-active');});});</script></head> \
        <body><nav class = 'navbar has-shadow'><div class = 'navbar-brand'><a class = 'navbar-item' href = '/'>ESP-RcOid</a> \
        <a role='button' class='navbar-burger' aria-label='menu' aria-expanded='true' ><span></span><span></span><span></span> \
        </a></div><div class='navbar-menu'><div class='navbar-start'><div class='navbar-item'> \
        <a class='navbar-item' href='/mqtt'>MQTT Settings</a> \
        <hr><a class='navbar-item' href='cmds'>Commands</a> <a class='navbar-item' href='docu.html'>Readme</a></div> \
        </div></div><div class='navbar-end'></div></nav><section class='section'> \
        <div class='container'><div class='content'>");
}

/*
    Returns HTML-Prefix for each Page
*/
String getHtmlSuffix()
{
    return F("</div></div></section></body></html>");
}

/****************************************************************************************************************************
 * Handle File Upload
 * **************************************************************************************************************************/

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
#if defined ESP8266 && filesystem == littlefs
    request->_tempFile = LittleFS.open("/" + filename, "w");
#else
    request->_tempFile = SPIFFS.open("/" + filename, "w");
#endif
    Serial.println(logmessage);
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
    request->redirect("/cmds");
  }
}

/*
   Zeigt Informationen zur HTTP-Anfrage im Serial-Monitor an
*/
void serial_print_HttpInfo(AsyncWebServerRequest *request)
{
  String message = "\n\n";
  message += "Time: " + String(millis(), DEC) + "\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += request->methodToString();
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  Serial.println(message);
}

/*
   Diese Webseite wird angezeigt, wenn eine unbekannte URL abgerufen wird.
*/
void notFound(AsyncWebServerRequest *request) {
  Serial.println("\nURI not found");
  serial_print_HttpInfo(request);
  AsyncResponseStream *response=request->beginResponseStream("text/plain");
  response->print("File Not Found\n\n");
  response->printf("URI: %s\n",request->url().c_str());
  response->printf("Method: %s\n",request->methodToString());
  response->printf("Arguments: %i\n",request->args());
  for (size_t i = 0; i < request->args(); i++)
  {
    response->printf(" %s: %s\n",request->argName(i).c_str(),request->arg(i).c_str());
  }
    response->setCode(404);
    request->send(response);
}


/*
   setzt den ESP zurück, damit er sich neu verbindet (nur im AP- und AP_STA-Modus)
   Wenn sich der ESP dann im WLAN einwählen konnte, startet er im STA-Modus

   Achtung: funktioniert das erste mal nach dem Flashen nicht! Der ESP muss dann mit dem Reset-Taster neu gestartet werden.
*/
void handleReset()
{
  Serial.println("ESP wird neu gestartet!");
  ESP.restart();
}

/*
*
*
*
*  Configure HTTP-Server functionality
*
*
*
*/

void configureWebServer()
{
  // *******************************************************************************************************
  // Handle root
  // *******************************************************************************************************
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    IPAddress ip = WiFi.localIP();
    response->print(getHtmlPrefix());
    if (WiFi.getMode() == WIFI_STA)
    {
      response->print("<div class='field'><div class='label'>ESP8266-RcDroid</div> \
        <div class='control'>STA-Mode, IP-Address: " +
                      String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]) +
                      "</div></div>");
      response->print("<div class='field'><div class='label'>Device Name</div> \
        <div class='control'>" +
                     deviceName +
                     "</div></div>");
      response->print(F("<div class='field'><div class='buttons'><a class='button is-danger' href='/deletepass'>delete WiFi-Settings"));
      response->print(F("<a class='button is-warning' href='/reset'>Reboot Device</a></div></div>"));
      response->print(F("<a class='button is-success' href='/receiveir'>Receive IR-Signal</a></div></div>"));
    }
    else
    {
      response->print("<div class='field'><div class='label'>ESP8266-RcDroid</div> \
                  <div class='control'>AP-Mode, IP-Address: " +
                     String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]) +
                     "</div></div>");
      int n = WiFi.scanNetworks();
      if (n > 0)
      {
        response->print("<ol>");
        for (int i = 0; i < n; ++i)
        {
          // Print SSID and RSSI for each network found
          response->print("<li>");
          #ifdef ESP8266
            response->printf("<b>%s (%i) %s %s</b>", WiFi.SSID(i).c_str(), WiFi.RSSI(i),(WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "(open)" : "(closed)",(WiFi.SSID(i) == WiFi.SSID()) ? "*" :" ");
          #else
            response->printf("<b>%s (%i) %s %s</b>", WiFi.SSID(i).c_str(), WiFi.RSSI(i),(WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "(open)" : "(closed)",(WiFi.SSID(i) == WiFi.SSID()) ? "*":" ");
          #endif
          response->print("</li>");
        }
        response->print("</ol>");
      }
      else
      {
        response->print("No Networks found.");
      }
      response->print(F("<form method='GET' action='setting' ><div class='field'><div class='label'>SSID:</div> \
    <div class='control'><input class='input' type='text' name='ssid'></div></div> \
    <div class='field'><div class='label'>Password:</div><div class='control'><input class='input' type='password' name='pass'></div> \
    <div class='field'><div class='label'>Device Name:</div><div class='control'><input class='input' type='text' name='device'></div> \
    </div><div class='field'><div class='buttons'><input class='button' type='submit' value='Save'/></div></div></form>"));

      response->print(F("<div class='field'><div class='buttons'><a class='button is-danger' href='/reset'>Reboot"));
      if (WiFi.status() == WL_CONNECTED)
      {
        response->print(" and deactivate AP");
      }
      response->print(F("</a>&nbsp<a class='button is-success' href='/receiveir'>Receive IR-Signal</a></div></div>"));
    }

    response->print(getHtmlSuffix());

    //request->send(200, "text/plain", "Hello, world");
    request->send(response);
  });

/*****************************************************************************************************
 * Handle /ir?code=<message>
 * ***************************************************************************************************/

  // Send a Post request to <IP>/ir?code=<message>
  server.on("/ir", HTTP_POST, [](AsyncWebServerRequest *request) {
    String message;
    if (request->argName(0).equals("code"))
    {
      message = request->arg("code");
      //handleIrCode(message);
      addIrCodeToQueue(message);
      request->send(200, "text/plain", "OK");
    }
    else
    {
      message += " Code not send";
      request->send(404, "text/plain", "IrResult: " + message);
    }
  });

/*****************************************************************************************************
 * Handle /setting?code=<message>
 * ***************************************************************************************************/

  // Send a GET request to <IP>/ir?code=<message>
  server.on("/setting", HTTP_GET, [](AsyncWebServerRequest *request) {
    String qsid;
    String qpass;
    String qdevice;
    if (request->hasParam("ssid") && request->hasParam("pass") && request->hasParam("device"))
    {
      qsid = request->getParam("ssid")->value();
      qpass = request->getParam("pass")->value();
      qdevice = request->getParam("device")->value();
      if (qsid.length() > 0 && qpass.length() > 0)
      {
        if (qdevice.length() > 0)
        {
          writeConfig(qsid, qpass, qdevice);
        }
        else
        {
          writeConfig(qsid, qpass);
        }

        htmlcontent = "<html><head><meta http-equiv=\"refresh\" content=\"0; URL=../\"></head><body style='font-family: sans-serif; font-size: 12px'>";
        htmlcontent += "OK";
        htmlcontent += "</body></html>";
        request->send(200, "text/html", htmlcontent);

        Serial.println("AP_STA-Modus wird aktiviert");

        ESP.restart();
      }
      else
      {
        request->send(404, "text/plain", "Not found");
      }
    }
  });

/*****************************************************************************************************
 * Handle /reset
 * ***************************************************************************************************/

  // Send a GET request to <IP>/ir?code=<message>
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain","Reboot ESP");
    ESP.restart();
  });

/*****************************************************************************************************
 * Handle /getip
 * ***************************************************************************************************/
/*
   gibt die IP als Klartext zurück (nur im AP- und AP_STA-Modus)
   wird von RCoid abgefragt um festzustellen, ob der ESP mit dem WLAN verbunden ist
   wird von RCoid automatisch in der App eingetragen
*/

  // Send a GET request to <IP>/ir?code=<message>
  server.on("/getip", HTTP_GET, [](AsyncWebServerRequest *request) {
  IPAddress ip = WiFi.localIP();
  htmlcontent = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  request->send(200, "text/plain", htmlcontent);
  Serial.println("Get IP = " + htmlcontent);
  });

/*****************************************************************************************************
 * Handle /receiveir
 * ***************************************************************************************************/

  server.on("/receiveir", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response=request->beginResponseStream("text/html");
    receive_ir_nonblock();
    response->print(getHtmlPrefix());
    response->print("<div class='field'><div class='control'>");
    response->print(irReceiveState);
    response->print("</div></div>");
    if(irReceiveFinished)
    {
        //Display the results
        response->print("<form method='Post' action='/cmd'>");
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='protocoll' value='%s'></div></div>","Protocoll",irProtocoll.c_str());
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='value' value='%s'></div></div>","Value",irValue.c_str());
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='address' value='%s'></div></div>","Address",irAddress.c_str());
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='length' value='%s'></div></div>","Length",irLength.c_str());
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='command' value='%s'></div></div>","Command",irCommand.c_str());
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='code' value='%s'></div></div>","Code",irCode.c_str());
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='cmdname' value='%s'></div></div>","Save as CMD-Name",irCommand.c_str());
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='cmddescription' value='%s'></div></div>","Description","");
        response->print("<input class='input' type='hidden' name='orgname' value='' />");
        response->print("<div class='field'><div class='buttons'><input class='button' type='submit' value='Save as CMD'/></div></div></form>");
    }
    response->print(F("<div class='field'><div class='buttons'><a class='button is-warning' href='/'><- back</a></div></div>"));
    if(irReceiveFinished)
    {
      response->print(F("<div class='field'><a class='button is-success' href='/receiveir'>Receive IR-Signal</a></div></div>"));
      response->print(getHtmlSuffix());
      irReceiveFinished=false;
    }
    else
    {
      //Send header with Meta Tags
      response->print(F("</div></div></section></body><head><meta http-equiv='refresh' content='2'></head></html>"));
    }
      request->send(response);
  });

/*****************************************************************************************************
 * Handle /mqtt
 * ***************************************************************************************************/

  server.on("/mqtt", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response=request->beginResponseStream("text/html");
  readMqttConfig();
  Serial.println("Config read success");
  response->print(getHtmlPrefix());
  response->print("<form method='Get' action='mqttset' >");
  response->print("<div class='field'><div class='label'>Server IP:</div> \
    <div class='control'><input class='input' type='text' name='server' value='"+ mqttServer +"'></div></div>");
  response->print("<div class='field'><div class='label'>Port:</div> \
    <div class='control'><input class='input' type='text' name='port' value='"+ String(mqttPort) +"'></div></div>");
  response->print("<div class='field'><div class='label'>Prefix:</div> \
    <div class='control'><input class='input' type='text' name='prefix' value='"+ mqttPrefix +"'></div></div>");
  response->print("<div class='field'><div class='label'>User Id:</div> \
    <div class='control'><input class='input' type='text' name='user' value='"+ mqttUser +"'></div></div>");
  response->print("<div class='field'><div class='label'>Password:</div> \
    <div class='control'><input class='input' type='password' name='pass' value='"+ mqttPass +"'></div></div>");
  response->print("<div class='field'><div class='buttons'><input class='button' type='submit' value='Save'/></div></div></form>");
  response->print(getHtmlSuffix());
  Serial.println("Sending Response");
  request->send(response);
  });

/*****************************************************************************************************
 * Handle /mqttset?server=192.168.17.1&port=1883&...
 * ***************************************************************************************************/

  server.on("/mqttset", HTTP_GET, [](AsyncWebServerRequest *request) {
  String qserver;
  String qport ;
  String qprefix ;
  String quser ;
  String qpass ;
  if(request->hasParam("server") && request->hasParam("port"))
  {
    qserver=request->getParam("server")->value();
    qport=request->getParam("port")->value();
    qprefix=request->getParam("prefix")->value();
    quser=request->getParam("user")->value();
    qpass=request->getParam("pass")->value();
  }
  if(qserver.length()>0)
  {
    writeMqttConfig(qserver,qport.toInt(),qprefix,quser,qpass);
  }
  else
  {
    //Reset Mqtt Config
    writeMqttConfig();
  }
  setupMqtt();
  request->redirect("/mqtt");
  });

/*****************************************************************************************************
 * Handle /cmds
 * ***************************************************************************************************/

  server.on("/cmds", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response=request->beginResponseStream("text/html");
    response->print(getHtmlPrefix());
    response->print(buildCmdPage());
    response->print(getHtmlSuffix());
    request->send(response);
  });

/*****************************************************************************************************
 * Handle /cmd
 * ***************************************************************************************************/

  server.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request) {
  String cmd;
  if(request->hasParam("button"))
  {
    cmd=request->getParam("button")->value();
  }
  IRcode code = readIrCmd(cmd);
  addIrCodeToQueue(code.Code);
  //handleIrCode(code.Code);
  request->redirect("/cmds");
  });

  /***************************************************************************************************
   * Save a Cmd with values by Post Request
   * if Param oldname!=cmdname we try to delte oldname and save with cmdname
   * *************************************************************************************************/
  server.on("/cmd",HTTP_POST,[](AsyncWebServerRequest *request){
    int params= request->params();
    String pCode;
    String pCmdName;
    String pCmdDescription;
    String pOrgName;
    String pGcCode;
    Serial.println("/cmd Post-Parameters");
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile())
      { //p->isPost() is also true
        Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } 
      else if(p->isPost())
      {
        Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } 
      else 
      {
        Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    } // for(int i=0;i<params;i++)

    if (request->hasParam("cmdname",true,false) && request->hasParam("cmddescription",true,false) && 
    request->hasParam("code",true,false) && 
    request->hasParam("orgname",true,false) && 
    request->hasParam("gccode",true,false))
    {
      Serial.println("Save Cmd Values found");
      pCode = request->getParam("code",true)->value();
      pCmdName=request->getParam("cmdname",true)->value();
      pCmdDescription=request->getParam("cmddescription",true)->value();
      pOrgName=request->getParam("orgname",true)->value();
      pGcCode=request->getParam("gccode",true)->value();
      if(pOrgName!=pCmdName && pOrgName!="")
      {
        deleteCmd(pOrgName);
      }
      IRcode tmpCode=IRcode(pCmdName,pCmdDescription,pCode,pGcCode);
      writeIrCmd(tmpCode);
      request->redirect("/cmds");
    }
    else
    {
      //Values missing
      Serial.println("Save Cmd missing values");
      AsyncResponseStream *response=request->beginResponseStream("text/html");
      response->print(getHtmlPrefix());
      response->print("<form method='Post' action='/cmd'>");
      response->print("<div class='field'><div class='label'>CMD-Name and Code must be filled</div></div>");
      if(request->hasParam("cmdName"))
      {
        response->print("<div class='field'><div class='label'>CMD-Name*:</div><div class='control'><input class='input' type='text' name='cmdname' value='"+request->getParam("cmdname")->value()+"'></div></div>");
        response->print("<input class='input' type='hidden' name='orgname' value='"+request->getParam("cmdname")->value()+"'>");
      }
      else
      {
        response->print("<div class='field'><div class='label'>CMD-Name*:</div><div class='control'><input class='input' type='text' name='cmdname' value=''></div></div>");
        response->print("<input class='input' type='hidden' name='orgname' value=''>");
      }
      if(request->hasParam("cmddescription"))
      {
        response->print("<div class='field'><div class='label'>Description:</div><div class='control'><input class='input' type='text' name='cmddescription' value='"+request->getParam("cmddescription")->value()+"'></div></div>");
      }
      else
      {
        response->print("<div class='field'><div class='label'>Description:</div><div class='control'><input class='input' type='text' name='cmddescription' value=''></div></div>");
      }
      if(request->hasParam("code"))
      {
        response->print("<div class='field'><div class='label'>Code*:</div><div class='control'><input class='input' type='text' name='code' value='"+request->getParam("code")->value()+"'></div></div>");
      }
      else
      {
        response->print("<div class='field'><div class='label'>Code*:</div><div class='control'><input class='input' type='text' name='code' value=''></div></div>");
      }
      if(request->hasParam("gccode"))
      {
        response->print("<div class='field'><div class='label'>GC Code:</div><div class='control'><input class='input' type='text' name='gccode' value='"+request->getParam("code")->value()+"'></div></div>");
      }
      else
      {
        response->print("<div class='field'><div class='label'>GC Code:</div><div class='control'><input class='input' type='text' name='gccode' value=''></div></div>");
      }
      response->print("<div class='field'><div class='buttons'><input class='button' type='submit' value='Save'/></div></div></form>");
      response->print(getHtmlSuffix());
      request->send(response);
    }
  });

  /***********************************************************************************************************************************************
   * Edit Command
   * *********************************************************************************************************************************************/
  server.on("/editcmd",HTTP_GET,[](AsyncWebServerRequest *request){
    String qcmd;
    if (request->hasParam("cmd"))
    {
      qcmd = request->getParam("cmd")->value();
      AsyncResponseStream *response=request->beginResponseStream("text/html");
      response->print(getHtmlPrefix());
      response->print(buildCmdEditPage(qcmd));
      response->print(getHtmlSuffix());
      request->send(response);
    }
    else
    {
      request->redirect("/cmds");
    }
  });

  /**************************************************************************************************************************************************
   * Delete Command
   * ************************************************************************************************************************************************/
  server.on("/delcmd",HTTP_GET,[](AsyncWebServerRequest *request){
    String qcmd;
    if (request->hasParam("cmd"))
    {
      qcmd = request->getParam("cmd")->value();
      deleteCmd(qcmd);
    }
    request->redirect("/cmds");
  });

  /**************************************************************************************************************************************************
   * Download Cmd-File
   * ************************************************************************************************************************************************/
  server.on("/downloadcmd",HTTP_GET,[](AsyncWebServerRequest *request){
    String qcmd;
    String cmdFileName;
    String fileContent;
    File cmdFile;
    if (request->hasParam("cmd"))
    {
      qcmd = request->getParam("cmd")->value();
      cmdFileName=getCmdFileName(qcmd);
      if(cmdFileName.length()>1)
      {
        Serial.print("Download Cmd for ");
        Serial.println(cmdFileName);
#if defined ESP8266 && filesystem == littlefs
        if (LittleFS.exists(cmdFileName.c_str()))
        {
            cmdFile=LittleFS.open(cmdFileName.c_str(), "r");
            fileContent=cmdFile.readString();
            cmdFile.close();
        }
#else
        if (SPIFFS.exists(cmdFileName))
        {
            cmdFile=SPIFFS.open(cmdFileName.c_str(), "r");
            fileContent=cmdFile.readString();
            cmdFile.close();
        }
#endif

        AsyncWebServerResponse *response = request->beginResponse(200,"application/json",fileContent);
        response->addHeader("Content-Disposition","attachment; filename=\""+cmdFileName.substring(1)+"\"");
        request->send(response);
      }
      else
      {
        Serial.print("CMD-File not found for ");
        Serial.println(qcmd);
        AsyncResponseStream *response=request->beginResponseStream("text/plain");
        response->print("CMD-File Not Found\n\n");
        response->printf("URI: %s\n",request->url().c_str());
        response->printf("Method: %s\n",request->methodToString());
        response->printf("Arguments: %i\n",request->args());
        for (size_t i = 0; i < request->args(); i++)
        {
          response->printf(" %s: %s\n",request->argName(i).c_str(),request->arg(i).c_str());
        }
        response->setCode(404);
        request->send(response);
      }
    }
  });

  /***********************************************************************************************************************************************
   * Upload CMD-File
   * *********************************************************************************************************************************************/
  server.on("/uploadcmd",HTTP_GET,[](AsyncWebServerRequest *request){
    AsyncResponseStream *response=request->beginResponseStream("text/html");
    response->print(getHtmlPrefix());
    response->print("<form method='Post' action='/uploadcmd' enctype='multipart/form-data'>");
    response->print("<div class='field'><div class='label'>Upload a .jcmd File to Device</div></div>");
    response->print("<div class='field'><div class='label'>CMD-File:</div><div class='file'><input type='file' name='data' multiple></div></div>");
    response->print("<div class='field'><div class='buttons'><input class='button' type='submit' value='Upload'/></div></div></form>");
    response->print(getHtmlSuffix());
    request->send(response);
  });
  
  server.on("/uploadcmd",HTTP_POST,[](AsyncWebServerRequest *request){
    request->send(200,"text/plain","Data send");
      }, handleUpload);

/*****************************************************************************************************
 * Handle //deletepass
 * ***************************************************************************************************/

  server.on("/deletepass", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response=request->beginResponseStream("text/html");
  response->print("<!DOCTYPE HTML>\r\n<html>");
  response->print("<p>Clearing the Config</p></html>");
  request->send(response);
  Serial.println("clearing config");
  writeConfig("","");
  ESP.restart();
  });


  server.onNotFound(notFound);

  server.begin();
  Serial.println("HTTP-Server setup finished");
}





#endif