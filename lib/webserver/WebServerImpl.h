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

/*
   Diese Webseite wird angezeigt, wenn eine unbekannte URL abgerufen wird.
*/
void notFound(AsyncWebServerRequest *request) {
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
      response->print(F("</a><a class='button is-success' href='/receiveir'>Receive IR-Signal</a></div></div>"));
    }

    response->print(getHtmlSuffix());

    //request->send(200, "text/plain", "Hello, world");
    request->send(response);
  });

/*****************************************************************************************************
 * Handle /ir?code=<message>
 * ***************************************************************************************************/

  // Send a GET request to <IP>/ir?code=<message>
  server.on("/ir", HTTP_GET, [](AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam("code"))
    {
      message = request->getParam("code")->value();
      handleIrCode(message);
      message+=" - OK";
    }
    else
    {
      message = "Code not send";
    }
    request->send(200, "text/plain", "IrResult: " + message);
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
    String result = handleReceiveIr();
    if (result.length() > 0)
    {
      htmlcontent = getHtmlPrefix();
      htmlcontent += "<div class='field'><div class='control'>";
      htmlcontent += result;
      htmlcontent += "</div></div>";
      htmlcontent += F("<div class='field'><div class='buttons'><a class='button is-warning' href='/'><- back");
      htmlcontent += F("<a class='button is-success' href='/receiveir'>Receive IR-Signal</a></div></div>");
      htmlcontent += getHtmlSuffix();
      request->send(200, "application/json", htmlcontent);
    }
    else
    {
      htmlcontent = getHtmlPrefix();
      htmlcontent += F("<div class='field'><div class='buttons'><a class='button is-warning' href='/'><- back");
      htmlcontent += F("<a class='button is-success' href='/receiveir'>Receive IR-Signal</a></div></div>");
      htmlcontent += getHtmlSuffix();

      request->send(408, "text/html", htmlcontent);
    }
  });

/*****************************************************************************************************
 * Handle /mqtt
 * ***************************************************************************************************/

  server.on("/mqtt", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response=request->beginResponseStream("text/html");
  readMqttConfig();
  response->print(getHtmlPrefix());
  response->print(F("<form method='Get' action='mqttset' >"));
  response->print("<div class='field'><div class='label'>Server IP:</div> \
    <div class='control'><input class='input' type='text' name='server' value='"+ mqttServer +"'></div></div>");
  response->print("<div class='field'><div class='label'>Port:</div> \
    <div class='control'><input class='input' type='text' name='port' value='"+ mqttPort +"'></div></div>");
  response->print("<div class='field'><div class='label'>Prefix:</div> \
    <div class='control'><input class='input' type='text' name='prefix' value='"+ mqttPrefix +"'></div></div>");
  response->print("<div class='field'><div class='label'>User Id:</div> \
    <div class='control'><input class='input' type='text' name='user' value='"+ mqttUser +"'></div></div>");
  response->print("<div class='field'><div class='label'>Password:</div> \
    <div class='control'><input class='input' type='password' name='pass' value='"+ mqttPass +"'></div></div>");
  response->print(F("<div class='field'><div class='buttons'><input class='button' type='submit' value='Save'/></div></div></form>"));
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
    writeMqttConfig(qserver,qport,qprefix,quser,qpass);
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
  if(request->hasParam("submit"))
  {
    cmd=request->getParam("submit")->value();
  }
  IRcode code = readIrCmd(cmd);
  handleIrCode(code.Code);
  request->redirect("/cmds");
  });

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