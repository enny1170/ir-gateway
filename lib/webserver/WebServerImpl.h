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
#include <CmdsGenerator.h>
#include <version.h>
#include <AsyncElegantOTA.h>

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
        <a class='navbar-item' href='/cmds'>Commands</a> \
        <hr><a class='navbar-item' href='/mqtt'>MQTT Settings</a> <a class='navbar-item' href='/receiveir'>Receive-IR</a> <a class='navbar-item' href='/docu'>Readme</a> \
        <a class='navbar-item' href='/update'>Firmware update</a></div> </div> \
        </div></div><div class='navbar-end'></div></nav><section class='section'> \
        <div class='container'><div class='content'>");
}

/*
    Returns HTML-Prefix for each Page
*/
String getHtmlSuffix()
{
    return "</div></div></section><footer class='footer'><div class='content has-text-centered'><p><i class='fa fa-copyright'></i> by smart-devices.cf 2021 - Version " + 
    String(VERSION_MAJOR)+"."+String(VERSION_MINOR)+"."+String(VERSION_BUILD) +"</p></div></footer></body></html>";
}

const char text_html[] = "text/html";

const char docu[] PROGMEM="<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'> \
        <title>ESP-RcOid</title><link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/bulma@0.8.2/css/bulma.min.css'> \
        <script defer src='https://use.fontawesome.com/releases/v5.3.1/js/all.js'></script> \
        <script src = 'https://ajax.googleapis.com/ajax/libs/jquery/2.1.1/jquery.min.js'></script> \
        <script>$(document).ready(function(){$('.navbar-burger').click(function() {$('.navbar-burger').toggleClass('is-active'); \
        $('.navbar-menu').toggleClass('is-active');});});</script></head> \
        <body><nav class = 'navbar has-shadow'><div class = 'navbar-brand'><a class = 'navbar-item' href = '/'>ESP-RcOid</a> \
        <a role='button' class='navbar-burger' aria-label='menu' aria-expanded='true' ><span></span><span></span><span></span> \
        </a></div><div class='navbar-menu'><div class='navbar-start'><div class='navbar-item'> \
        <a class='navbar-item' href='/cmds'>Commands</a> \
        <hr><a class='navbar-item' href='/mqtt'>MQTT Settings</a> <a class='navbar-item' href='/receiveir'>Receive-IR</a> <a class='navbar-item' href='/docu'>Readme</a> </div> </div> \
        </div></div><div class='navbar-end'></div></nav><section class='section'> \
        <div class='container'><div class='content'> \
        <div><strong>This Firmware is a complete new Implementation of the <a href='https://rcoid.de/'>Rcoid</a> Project.</strong><p> \
    This implementation supports the same Interface for the RcOid App. Has a extended Web UI and REST-Api. Devices can be used as IR-Gateway trought MQTT. \
    Sending and reciving IR-Codes new implemented for blocking free work. Also the storage of IR-Codes (Commands) added. This Commands can be used for MQTT and REST. \
    Upload and Downlowad of stored commands are possible. \
    </p><p>A new unconfigured Device will start in Accesspoint-Mode and provide a 'ESP8266 for RCoid Access Point' with User pass and Password pass. \
    You are able to setup SSID and Password for your local WiFi-Connection. </br> \
    From the Root-Page you have also the possibility tor reboot, remove Web Config, and reset to Factory. \
    Only from this page you can enable IR-reciving to capture IR-Codes. </br> \
    From the MQTT Page you can ste the Connection to your MQTT-Server and a Prefix used for the topic on MQTT. If the MQTT-Server adress empty or a '.' the MQTT communication will be disabled. \
    </p><p>In MQTT the device will be provide its local IP-Adress and available commands. The device subscribes /[Prefix]/Cmd. If the device receive a String in this topic, this will be interpreted as a Command. \
    The device try to find this Command and will send the Code immedaly. \
    </p><p>The same Functionality can be reached by sending a GET-Request to /cmd?button=xxxx. Where xxxx the name of the command. It can be taken from the Command-Page. \
    The Command page are selfexplanating. But two points you have to know. Editing a Command you have to set a Code and a Name. The Name will be used as Filename to store the command on device Flash. \
    And also as Command to raise from MQTT.</br>The Clock Icon on the Command-Page will give you the possibilty to setup one Timer to send this command later. \
    </p>If you want to upload Commands use the .jcmd-files in the data Folder as reference. and make sure the file suffix is .jcmd.</div></div></div></section></body></html>";


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
    AsyncResponseStream *response = request->beginResponseStream(text_html);
    IPAddress ip = WiFi.localIP();
    response->print(getHtmlPrefix());
    if (WiFi.getMode() == WIFI_STA)
    {
      response->print(F("<div class='field'><div class='label'>ESP8266-RcDroid</div> <div class='control'>STA-Mode, IP-Address: "));
      response->print(String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]) + "</div></div>");
      response->print(F("<div class='field'><div class='label'>Device Name</div> <div class='control'>"));
      response->print(deviceName + "</div></div>");
      response->print(F("<div class='field'><div class='label'>Timer</div><div class='control'>"));
      response->print(cmdTimer.getTimerString());
      response->print(F("</div></div>"));               
      response->print(F("<div class='field'><div class='buttons'><a class='button is-danger' href='/deletepass'>delete WiFi-Settings</a></div></div>"));
      response->print(F("<div class='field'><div class='buttons'><a class='button is-danger' href='/factory'>factory Reset</a></div></div>"));
      response->print(F("<div class='field'><div class='buttons'><a class='button is-warning' href='/reset'>Reboot Device</a></div></div>"));
      response->print(F("<div class='field'><div class='buttons'><a class='button is-success' href='/receiveir'>Receive IR-Signal</a></div></div>"));
    }
    else
    {
      response->print(F("<div class='field'><div class='label'>ESP8266-RcDroid</div> <div class='control'>AP-Mode, IP-Address: "));
      response->print(String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]) + "</div></div>");
      response->print(F("<div class='field'><div class='label'>Timer</div><div class='control'>"));
      response->print(cmdTimer.getTimerString());
      response->print(F("</div></div>"));               
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

        htmlcontent = F("<html><head><meta http-equiv=\"refresh\" content=\"0; URL=../\"></head><body style='font-family: sans-serif; font-size: 12px'>");
        htmlcontent += "OK";
        htmlcontent += F("</body></html>");
        request->send(200, text_html, htmlcontent);

        Serial.println(F("AP_STA-Modus wird aktiviert"));

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
    AsyncResponseStream *response=request->beginResponseStream(text_html);
    receive_ir_nonblock();
    response->print(getHtmlPrefix());
    response->print(F("<div class='field'><div class='control'>"));
    response->print(irReceiveState);
    response->print(F("</div></div>"));
    if(irReceiveFinished)
    {
        //Display the results
        response->print(F("<form method='Post' action='/cmd'>"));
        response->print(F("<input class='input' type='hidden' name='gccode' value='' />"));
        response->print(F("<input class='input' type='hidden' name='orgname' value='' />"));
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='protocoll' value='%s'></div></div>","Protocoll",irProtocoll.c_str());
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='value' value='%s'></div></div>","Value",irValue.c_str());
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='address' value='%s'></div></div>","Address",irAddress.c_str());
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='length' value='%s'></div></div>","Length",irLength.c_str());
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='command' value='%s'></div></div>","Command",irCommand.c_str());
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='code' value='%s'></div></div>","Code",irCode.c_str());
        response->printf("<div class='field'><div class='label'>%s:</div><div class='control'><input class='input' type='text' name='cmdname' value='%s'></div></div>","Save as CMD-Name*",irCommand.c_str());
        response->print(F("<div class='field'><div class='label'>Description:</div><div class='control'><input class='input' type='text' name='cmddescription' value=''></div></div>"));
        response->print(F("<div class='field'><div class='label'>Repeat's:</div><div class='control'><input class='input' type='text' name='repeat' value='1'></div></div>"));
        response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='Save as CMD'/></div></div>"));
        response->print(F("<div class='field'><a class='button is-success' href='/receiveir'>Receive IR-Signal</a></div></div></form>"));
        response->print(getHtmlSuffix());
        irReceiveFinished=false;
    }
    else
    {
      response->print(F("<div class='field'><div class='buttons'><a class='button is-warning' href='/'><- back</a></div></div>"));
      //Send header with Meta Tags
      response->print(F("</div></div></section></body><head><meta http-equiv='refresh' content='2'></head></html>"));
    }
    request->send(response);
  });

/*****************************************************************************************************
 * Handle /mqtt
 * ***************************************************************************************************/

  server.on("/mqtt", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response=request->beginResponseStream(text_html);
  readMqttConfig();
  Serial.println("Config read success");
  response->print(getHtmlPrefix());
  response->print(F("<form method='Get' action='mqttset' ><div class='field'><div class='label'>Server IP:</div> <div class='control'><input class='input' type='text' name='server' value='"));
  response->print(mqttServer);
  response->print(F("'></div></div> <div class='field'><div class='label'>Port:</div> <div class='control'><input class='input' type='text' name='port' value='"));
  response->print(String(mqttPort));
  response->print(F("'></div></div> <div class='field'><div class='label'>Prefix:</div> <div class='control'><input class='input' type='text' name='prefix' value='"));
  response->print(mqttPrefix);
  response->print(F("'></div></div> <div class='field'><div class='label'>User Id:</div> <div class='control'><input class='input' type='text' name='user' value='"));
  response->print(mqttUser);
  response->print(F("'></div></div> <div class='field'><div class='label'>Password:</div> <div class='control'><input class='input' type='password' name='pass' value='"));
  response->print(mqttPass);
  response->print(F("'></div></div><div class='field'><div class='buttons'><input class='button' type='submit' value='Save'/></div></div></form>"));
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
// this will not work every time, because of missing ram to buffer the output stream

//   server.on("/cmds", HTTP_GET, [](AsyncWebServerRequest *request) {
//     AsyncResponseStream *response=request->beginResponseStream(text_html);
//     response->print(getHtmlPrefix());
//     int count=0;
//     Serial.println("buildCmdPage");
//     response->print(F("<form method='GET' action='cmd' >"));
// #if defined ESP8266 && filesystem == littlefs
//     Dir dir = LittleFS.openDir("/");
//     while(dir.next())
//     {
//         // Serial.print(dir.fileName());
//         // Serial.print("  ");
//         // Serial.println(dir.fileSize());
//         if( dir.fileName().endsWith(IRCODE_FILE_EXTENSION))
//         {
//             int pointPos=dir.fileName().indexOf('.');
//             response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='"));
//             response->print(dir.fileName().substring(0,pointPos));
//             response->print(F("' name='button'/>"));
//             response->print("<a href='editcmd?cmd="+dir.fileName().substring(0,pointPos)+"'>&nbsp&nbsp<i class='fa fa-edit'></i></a>");
//             response->print("<a href='downloadcmd?cmd="+dir.fileName().substring(0,pointPos)+"'>&nbsp&nbsp<i class='fa fa-download'></i></a>");
//             response->print("<a href='delcmd?cmd="+dir.fileName().substring(0,pointPos)+"'>&nbsp&nbsp<i class='fa fa-trash'></i></a>");
//             response->print("<a href='timer?cmd="+dir.fileName().substring(0,pointPos)+"'>&nbsp&nbsp<i class='fa fa-clock'></i></a>");
//             response->print(F("</div></div>"));
//             count ++;
//         }
//     }
//     response->print(F("</form>"));
// #else
//     File dir = SPIFFS.open("/");
//     File file=dir.openNextFile();
// //    Serial.println(file.name());
//     while(file)
//     {
//         String fileName=String(file.name());
//         if(fileName.endsWith(IRCODE_FILE_EXTENSION))
//         {
//             int pointPos=fileName.indexOf('.');
//             response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='"));
//             response->print(fileName.substring(1,pointPos));
//             response->print(F("' name='button'/>"));
//             response->print("<a href='editcmd?cmd="+fileName.substring(1,pointPos)+"'>&nbsp&nbsp<i class='fa fa-edit'></i></a>");
//             response->print("<a href='downloadcmd?cmd="+fileName.substring(1,pointPos)+"'>&nbsp&nbsp<i class='fa fa-download'></i></a>");
//             response->print("<a href='delcmd?cmd="+fileName.substring(1,pointPos)+"'>&nbsp&nbsp<i class='fa fa-trash'></i></a>");
//             response->print("<a href='timer?cmd="+fileName.substring(1,pointPos)+"'>&nbsp&nbsp<i class='fa fa-clock'></i></a>");
//             response->print(F("</div></div>"));
//             count ++;
//         }
//         file=dir.openNextFile();
//     }
//     response->print(("</form>"));
// #endif
//     response->print(F("<div class='field'><div class='label'>"));
//     response->print(String(count));
//     response->print(F("&nbsp CMD's found on device.</div></div>"));
//     response->print(F("<div class='field'><div class='buttons'><a class='button is-warning' href='/uploadcmd'>Upload CMD to Device</a></div></div>"));
//     response->print(F("<div class='field'><div class='buttons'><a class='button is-success' href='/editcmd'>Create CMD</a></div></div>"));

//     response->print(getHtmlSuffix());
//     request->send(response);
//   });

//Try to do it with chunked response
  server.on("/cmds", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("buildCmdPage");
      CmdsGenerator* gen=new CmdsGenerator();
      AsyncWebServerResponse *response= request->beginChunkedResponse(text_html, 
      [gen](uint8_t * buffer,size_t maxlen,size_t index) -> size_t
      {
        size_t len=gen->readData(buffer,maxlen);
        //Serial.printf("\nIndex: %i\n",index);
        if(len == 0) delete gen;
        return len;
      });
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
  for (int i = 0; i < code.Repeat; i++)
  {
    addIrCodeToQueue(code.Code);
  }
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
    int pRepeat;
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
    request->hasParam("gccode",true,false) &&
    request->hasParam("repeat",true,false))
    {
      Serial.println("Save Cmd Values found");
      pCode = request->getParam("code",true)->value();
      pCmdName=request->getParam("cmdname",true)->value();
      pCmdDescription=request->getParam("cmddescription",true)->value();
      pOrgName=request->getParam("orgname",true)->value();
      pGcCode=request->getParam("gccode",true)->value();
      pRepeat=request->getParam("repeat",true)->value().toInt();
      if(pOrgName!=pCmdName && pOrgName!="")
      {
        deleteCmd(pOrgName);
      }
      IRcode tmpCode=IRcode(pCmdName,pCmdDescription,pCode,pGcCode,pRepeat);
      writeIrCmd(tmpCode);
      request->redirect("/cmds");
    }
    else
    {
      //Values missing
      Serial.println("Save Cmd missing values");
      AsyncResponseStream *response=request->beginResponseStream(text_html);
      response->print(getHtmlPrefix());
      response->print(F("<form method='Post' action='/cmd'> <div class='field'><div class='label'>CMD-Name and Code must be filled</div></div>"));
      if(request->hasParam("cmdName"))
      {
        response->print(F("<div class='field'><div class='label'>CMD-Name*:</div><div class='control'><input class='input' type='text' name='cmdname' value='"));
        response->print(request->getParam("cmdname")->value()+"'></div></div>");
        response->print(F("<input class='input' type='hidden' name='orgname' value='"));
        response->printf("%s'>",request->getParam("cmdname")->value().c_str());
      }
      else
      {
        response->print(F("<div class='field'><div class='label'>CMD-Name*:</div><div class='control'><input class='input' type='text' name='cmdname' value=''></div></div>"));
        response->print(F("<input class='input' type='hidden' name='orgname' value=''>"));
      }
      if(request->hasParam("cmddescription"))
      {
        response->print(F("<div class='field'><div class='label'>Description:</div><div class='control'><input class='input' type='text' name='cmddescription' value='"));
        response->printf("%s'></div></div>",request->getParam("cmddescription")->value().c_str());
      }
      else
      {
        response->print(F("<div class='field'><div class='label'>Description:</div><div class='control'><input class='input' type='text' name='cmddescription' value=''></div></div>"));
      }
      if(request->hasParam("code"))
      {
        response->print(F("<div class='field'><div class='label'>Code*:</div><div class='control'><input class='input' type='text' name='code' value='"));
        response->printf("%s'></div></div>",request->getParam("code")->value().c_str());
      }
      else
      {
        response->print(F("<div class='field'><div class='label'>Code*:</div><div class='control'><input class='input' type='text' name='code' value=''></div></div>"));
      }
      if(request->hasParam("gccode"))
      {
        response->print(F("<div class='field'><div class='label'>GC Code:</div><div class='control'><input class='input' type='text' name='gccode' value="));
        response->printf("%s'></div></div>",request->getParam("code")->value().c_str());
      }
      else
      {
        response->print(F("<div class='field'><div class='label'>GC Code:</div><div class='control'><input class='input' type='text' name='gccode' value=''></div></div>"));
      }
      response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='Save'/></div></div></form>"));
      response->print(getHtmlSuffix());
      request->send(response);
    }
  });

  /***********************************************************************************************************************************************
   * Edit Command
   * *********************************************************************************************************************************************/
  server.on("/editcmd",HTTP_GET,[](AsyncWebServerRequest *request){
    AsyncResponseStream *response=request->beginResponseStream(text_html);
    String qcmd;
    if (request->hasParam("cmd"))
    {
      qcmd = request->getParam("cmd")->value();
      response->print(getHtmlPrefix());
      response->print(buildCmdEditPage(qcmd));
      response->print(getHtmlSuffix());
      request->send(response);
    }
    else
    {
      response->print(getHtmlPrefix());
      response->print(buildCmdEditPage(""));
      response->print(getHtmlSuffix());
      request->send(response);
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
    AsyncResponseStream *response=request->beginResponseStream(text_html);
    response->print(getHtmlPrefix());
    response->print(F("<form method='Post' action='/uploadcmd' enctype='multipart/form-data'><div class='field'><div class='label'>Upload a .jcmd File to Device</div></div> \
    <div class='field'><div class='label'>CMD-File:</div><div class='file'><input type='file' name='data' multiple></div></div><div class='field'><div class='buttons'><input class='button' type='submit' value='Upload'/></div></div></form>"));
    response->print(getHtmlSuffix());
    request->send(response);
  });
  
  server.on("/uploadcmd",HTTP_POST,[](AsyncWebServerRequest *request){
    request->send(200,"text/plain","Data send");
      }, handleUpload);

/*****************************************************************************************************
 * Handle /deletepass
 * ***************************************************************************************************/

  server.on("/deletepass", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response=request->beginResponseStream(text_html);
  response->print(F("<!DOCTYPE HTML>\r\n<html><p>Clearing the Config</p></html>"));
  request->send(response);
  Serial.println("clearing config");
  writeConfig("","");
  ESP.restart();
  });

/*************************************************************************************************************************************************
 * Handle /timer
 * **********************************************************************************************************************************************/

  server.on("/timer",HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String qcmd;
    AsyncResponseStream *response=request->beginResponseStream(text_html);
    cmdTimer.update();
    response->print(getHtmlPrefix());
    if (request->hasParam("cmd"))
    {
      qcmd = request->getParam("cmd")->value();
      response->print(F("<form method='Post' action='/timer'>"));
      response->print(F("<div class='field'><div class='label'>Timer for Command</div><div class='control'><input class='input' type='text' name='cmdname' value='"));
      response->print(qcmd + "'></div></div>");
      response->print("<input class='input' type='hidden' name='cmd' value='"+qcmd+"'>");
      response->print("<div class='field'><div class='label'>current Time:</div><div class='control'>"+timeClient.getFormattedTime()+"</div></div>");
      response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='10 Minutes' name='button'/></div></div>"));
      response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='20 Minutes' name='button'/></div></div>"));
      response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='30 Minutes' name='button'/></div></div>"));
      response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='40 Minutes' name='button'/></div></div>"));
      response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='50 Minutes' name='button'/></div></div>"));
      response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='1 Hour' name='button'/></div></div>"));
      response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='1.5 Hours' name='button'/></div></div>"));
      response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='2 Hours' name='button'/></div></div>"));
      response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='2.5 Hours' name='button'/></div></div>"));
      response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='3 Hours' name='button'/></div></div> </form>"));
    }
    else
    {
      response->print(F("<div class='field'><div class='label'>Error:</div><div class='control'>Parameter CMD not given.</div></div>")); //"<input class='input' type='text' name='cmdname' value='"+request->getParam("cmdname")->value()+"'></div></div>");
    }
    response->print(getHtmlSuffix());
    request->send(response);
  });

  server.on("/timer",HTTP_POST, [](AsyncWebServerRequest *request)
  {
    String qcmd;
    String qValue;
    size_t offsetMinutes=0;
    if(request->hasParam("cmdname",true,false) && request->hasParam("button",true,false))
    {
      qcmd=request->getParam("cmdname",true)->value();
      qValue=request->getParam("button",true)->value();
      if(qValue=="10 Minutes")
        offsetMinutes=10;
      else if(qValue=="20 Minutes")
        offsetMinutes=20;
      else if(qValue=="30 Minutes")
        offsetMinutes=30;
      else if(qValue=="40 Minutes")
        offsetMinutes=40;
      else if(qValue=="50 Minutes")
        offsetMinutes=50;
      else if(qValue=="1 Hour")
        offsetMinutes=60;
      else if(qValue=="1.5 Hours")
        offsetMinutes=90;
      else if(qValue=="2 Hours")
        offsetMinutes=120;
      else if(qValue=="2.5 Hours")
        offsetMinutes=150;
      else if(qValue=="3 Hours")
        offsetMinutes=180;

      if(offsetMinutes>0)
      {
        Serial.printf("\nSetup Timer for %s in %i Minutes",qcmd.c_str(),offsetMinutes);
        cmdTimer.setTimer(offsetMinutes,qcmd);
      }
    }
    request->redirect("/");
  });

  /**************************************************************************************************************
   * Handle /factory (Factory Reset)
   * ***********************************************************************************************************/

  server.on("/factory",HTTP_GET,[](AsyncWebServerRequest *request)
  {
      AsyncResponseStream *response=request->beginResponseStream(text_html);
      response->print(getHtmlPrefix());
      response->print(F("<form method='Post' action='/factory'><div class='field'><div class='label'>Factory Reset</div><div class='control'>This operation will be remove all configuration from device. Do you want to do this ?</div></div><div class='field'><div class='buttons'><input class='button is-danger' type='submit' value='YES' name='button'/><a class='button is-success' href='/'>NO</a></div></div>"));
      response->print(getHtmlSuffix());
      request->send(response);
  });

  server.on("/factory",HTTP_POST,[](AsyncWebServerRequest *request)
  {
    // remove all files and reboot
    removeAllConfigFiles();
    request->redirect("/reset");
  });

  /*******************************************************************************************************************
   * Handle /docu 
   * ****************************************************************************************************************/
  server.on("/docu",HTTP_GET,[](AsyncWebServerRequest *request)
  {
    // AsyncResponseStream *response=request->beginResponseStream("text/html");
    // response->print(getHtmlPrefix());
    // response->print(getHtmlSuffix());
    // request->send(response);
    request->send_P(200,text_html,docu);
  });

  server.on("/heap",HTTP_GET,[](AsyncWebServerRequest *request)
  {
    AsyncResponseStream *response=request->beginResponseStream("text/plain");
    response->printf("Free Heap: %i bytes\n",ESP.getFreeHeap());
    request->send(response);
  });

  server.onNotFound(notFound);
  AsyncElegantOTA.begin(&server);
  server.begin();
  Serial.println("HTTP-Server setup finished");
}


#endif