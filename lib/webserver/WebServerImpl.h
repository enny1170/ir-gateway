#ifndef WEBSERVERIMPL_H
#define WEBSERVERIMPL_H

#include <ESP8266WiFi.h>
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

void serial_print_HttpInfo();

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

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
  Wird aufgerufen wnn die MQTT-Seite angefordert wird
*/
void handleMqtt()
{
  readMqttConfig();
  htmlcontent=getHtmlPrefix();
  htmlcontent+=F("<form method='Get' action='mqttset' >");
  htmlcontent+="<div class='field'><div class='label'>Server IP:</div> \
    <div class='control'><input class='input' type='text' name='server'>"+ mqttServer +"</div></div>";
  htmlcontent+="<div class='field'><div class='label'>Port:</div> \
    <div class='control'><input class='input' type='text' name='port'>"+ mqttPort +"</div></div>";
  htmlcontent+="<div class='field'><div class='label'>Prefix:</div> \
    <div class='control'><input class='input' type='text' name='prefix'>"+ mqttPrefix +"</div></div>";
  htmlcontent+="<div class='field'><div class='label'>User Id:</div> \
    <div class='control'><input class='input' type='text' name='user'>"+ mqttUser +"</div></div>";
  htmlcontent+="<div class='field'><div class='label'>Password:</div> \
    <div class='control'><input class='input' type='password' name='pass'>"+ mqttPass +"</div></div>";
  htmlcontent +=F("</form>");
  server.send(200,"text/html",htmlcontent);
}

/* 
  Wird aufgerufen wenn die MQTT-Seite gespeichert wird
*/
void handleMqttSettings()
{
  serial_print_HttpInfo();
  String qserver = server.arg("server");
  String qport = server.arg("port");
  String qprefix = server.arg("prefix");
  String quser = server.arg("user");
  String qpass = server.arg("pass");
  if(qserver.length()>0)
  {
    writeMqttConfig(qserver,qport,qprefix,quser,qpass);
  }
  else
  {
    //Reset Mqtt Config
    writeMqttConfig();
  }
  readMqttConfig();
  mqttConnect();
  server.sendHeader("Location", String("/mqtt"), true);
  server.send(302,"text/plain","");
}
/* 
  Wird aufgerufen wenn die CMDs-Seite abgerufen wird
*/
void handleCmds()
{
    htmlcontent=getHtmlPrefix();
    htmlcontent +=  buildCmdPage();
    htmlcontent += getHtmlSuffix();
    server.send(200,"text/html",htmlcontent);
}
/* 
  Wird aufgerufen wenn die CMD-Seite aufgerufen wird
*/
void handleCmd()
{
  String cmd= server.arg("submit");
  IRcode code = readIrCmd(cmd);
  handleIrCode(code.Code);
  server.sendHeader("Location", String("/mqtt"), true);
  server.send(302,"text/plain","");
}
/*
   Diese Webseite wird angezeigt, wenn der ESP im WLAN mit seiner lokalen IP abgerufen wird.
*/
void handleRoot()
{
    IPAddress ip = WiFi.localIP();
    htmlcontent=getHtmlPrefix();
    htmlcontent += "<div class='field'><div class='label'>ESP8266-RcDroid</div> \
        <div class='control'>STA-Mode, IP-Address: " 
        +  String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]) + 
        "</div></div>";
    htmlcontent += "<div class='field'><div class='label'>Device Name</div> \
        <div class='control'>" + deviceName +
        "</div></div>";
    htmlcontent += F("<div class='field'><div class='buttons'><a class='button is-danger' href='/deletepass'>delete WiFi-Settings");
    htmlcontent += F("<a class='button is-success' href='/receiveir'>Receive IR-Signal</a></div></div>");
    htmlcontent += getHtmlSuffix();
    server.send(200, "text/html", htmlcontent);
}

/*
   Diese Webseite wird angezeigt, wenn der ESP im AP-Modus mit seiner IP 192.168.0.1 abgerufen wird.
   Hier kann man dann die SSID und das Password des WLAN eingeben und den ESP neu starten.
*/
void handleAPRoot()
{

  IPAddress ip = WiFi.softAPIP();
  htmlcontent = getHtmlPrefix();
  htmlcontent += "<div class='field'><div class='label'>ESP8266-RcDroid</div> \
                  <div class='control'>AP-Mode, IP-Address: " 
                  +  String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]) + 
                  "</div></div>";

  int n = WiFi.scanNetworks();
  if (n > 0)
  {
    htmlcontent += "<ol>";
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      htmlcontent += "<li>";
      if (WiFi.SSID(i) == WiFi.SSID())
        htmlcontent += "<b>";
      htmlcontent += WiFi.SSID(i);
      htmlcontent += " (";
      htmlcontent += WiFi.RSSI(i);
      htmlcontent += ")";
      htmlcontent += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
      if (WiFi.SSID(i) == WiFi.SSID())
      {
        IPAddress ip = WiFi.localIP();
        htmlcontent += "</b>  connected mit local IP " + String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      }
      htmlcontent += "</li>";
    }
    htmlcontent += "</ol>";
  }
  else
  {
    htmlcontent += "No Networks found.";
  }
  htmlcontent += F("<form method='GET' action='setting' ><div class='field'><div class='label'>SSID:</div> \
    <div class='control'><input class='input' type='text' name='ssid'></div></div> \
    <div class='field'><div class='label'>Password:</div><div class='control'><input class='input' type='password' name='pass'></div> \
    <div class='field'><div class='label'>Device Name:</div><div class='control'><input class='input' type='text' name='device'></div> \
    </div><div class='field'><div class='buttons'><input class='button' type='submit' value='Save'/></div></div></form>");

  htmlcontent += F("<div class='field'><div class='buttons'><a class='button is-danger' href='/reset'>Reboot");
  if (WiFi.status() == WL_CONNECTED)
  {
    htmlcontent += " and deactivate AP";
  }
  htmlcontent += F("</a><a class='button is-success' href='/receiveir'>Receive IR-Signal</a></div></div>");
  
  htmlcontent += getHtmlSuffix();
  server.send(200,"text/html",htmlcontent);
}

/*
   diese Funktion löscht die Zugangsdaten aus dem EEPROM
*/
void handleDeletePass()
{
  htmlcontent = "<!DOCTYPE HTML>\r\n<html>";
  htmlcontent += "<p>Clearing the Config</p></html>";
  server.send(200, "text/html", htmlcontent);
  Serial.println("clearing config");
  writeConfig("","");
  ESP.restart();
}

/*
   Diese Webseite wird angezeigt, wenn eine unbekannte URL abgerufen wird.
*/
void handleNotFound() {
  htmlcontent = "File Not Found\n\n";
  htmlcontent += "URI: ";
  htmlcontent += server.uri();
  htmlcontent += "\nMethod: ";
  htmlcontent += (server.method() == HTTP_GET) ? "GET" : "POST";
  htmlcontent += "\nArguments: ";
  htmlcontent += server.args();
  htmlcontent += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    htmlcontent += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", htmlcontent);
}

/*
   Gibt den Interger-Wert einers Argumentes zurück.
   oder -1. falls das Argument nicht existiert
*/
int getArgValue(String name)
{
  for (uint8_t i = 0; i < server.args(); i++)
    if (server.argName(i) == name)
      return server.arg(i).toInt();
  return -1;
}

/*
   Zeigt Informationen zur HTTP-Anfrage im Serial-Monitor an
*/
void serial_print_HttpInfo()
{
  String message = "\n\n";
  message += "Time: " + String(millis(), DEC) + "\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  Serial.println(message);
}
/**
   Schaltete einen beliebigen GPIO Pin.
   z.B.: http://ip.of.your.device/out?port=0&value=1     //schaltet GPIO0 an
         http://ip.of.your.device/out?port=5&value=t     //wechselt den Zustand von GPIO5
*/
void handleOut()
{
  serial_print_HttpInfo();

  if (server.arg("port").length() == 0
      || server.arg("value").length() == 0)
  {
    handleNotFound();
    return;
  }

  int port = getArgValue("port");
  Serial.print("Port ");
  Serial.println(port);

  if (port < 0 || port > 15)
  {
    Serial.println("Port out of range. Abort!");
    htmlcontent = "Port out of range.";
    server.send(400, "text/plain", htmlcontent);
    return;
  }

  Serial.print("Value ");

  if (server.arg("value") == "t")
  {
    Serial.println("t (Toggle)");
    if (port != -1)
    {
      pinMode(port, OUTPUT);
      digitalWrite(port, !digitalRead(port));
    }
  }
  else
  {
    int value = getArgValue("value");
    Serial.println(value);

    if (port != -1 && value != -1)
    {
      pinMode(port, OUTPUT);
      digitalWrite(port, value == 0 ? LOW : HIGH);
    }
  }

  htmlcontent = "OK";
  server.send(200, "text/plain", htmlcontent);

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
   gibt die IP als Klartext zurück (nur im AP- und AP_STA-Modus)
   wird von RCoid abgefragt um festzustellen, ob der ESP mit dem WLAN verbunden ist
   wird von RCoid automatisch in der App eingetragen
*/
void handleGetIp()
{
  IPAddress ip = WiFi.localIP();
  htmlcontent = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  server.send(200, "text/plain", htmlcontent);
  Serial.println("Get IP = " + htmlcontent);
}



/*
   speicher die SSID und das Password in den EEPROM
   und versucht sich anschließend im WLAN einzuloggen.

   Wenn das gelingt wird RCoid die IP abrufen und den ESP neu startet
*/
void handleSetting()
{
  serial_print_HttpInfo();
  String qsid = server.arg("ssid");
  String qpass = server.arg("pass");
  String qdevice = server.arg("device");
  if (qsid.length() > 0 && qpass.length() > 0)
  {
    if(qdevice.length()>0)
    {
      writeConfig(qsid,qpass,qdevice);
    }
    else
    {
      writeConfig(qsid,qpass);
    }
    

    htmlcontent = "<html><head><meta http-equiv=\"refresh\" content=\"0; URL=../\"></head><body style='font-family: sans-serif; font-size: 12px'>";
    htmlcontent += "OK";
    htmlcontent += "</body></html>";
    server.send(200, "text/html", htmlcontent);

    Serial.println("AP_STA-Modus wird aktiviert");

    ESP.restart();

    // WiFi.mode(WIFI_AP_STA);
    // delay(100);

    // WiFi.begin(qsid.c_str(), qpass.c_str());
    // Serial.println("Wifi Restarted");
    // Serial.println(WiFi.status());
    // Serial.println(WiFi.localIP());
  }
  else
  {
    handleNotFound();
  }

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
    response->print(getHtmlPrefix);
    if (WiFi.getMode == WIFI_STA)
    {
      response->print("<div class='field'><div class='label'>ESP8266-RcDroid</div> \
        <div class='control'>STA-Mode, IP-Address: " +
                      String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]) +
                      "</div></div>");
      response.print("<div class='field'><div class='label'>Device Name</div> \
        <div class='control'>" +
                     deviceName +
                     "</div></div>");
      response.print(F("<div class='field'><div class='buttons'><a class='button is-danger' href='/deletepass'>delete WiFi-Settings"));
      response.print(F("<a class='button is-success' href='/receiveir'>Receive IR-Signal</a></div></div>"));
    }
    else
    {
      response.print("<div class='field'><div class='label'>ESP8266-RcDroid</div> \
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
          if (WiFi.SSID(i) == WiFi.SSID())
            response->printf("<b>%s (%i) %s /<b>", WiFi.SSID(i), WiFi.RSSI(i), (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
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

    response.print(getHtmlSuffix());

    //request->send(200, "text/plain", "Hello, world");
    request->send(response);
  });

  // Send a GET request to <IP>/get?message=<message>
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam(PARAM_MESSAGE))
    {
      message = request->getParam(PARAM_MESSAGE)->value();
    }
    else
    {
      message = "No message sent";
    }
    request->send(200, "text/plain", "Hello, GET: " + message);
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam(PARAM_MESSAGE, true))
    {
      message = request->getParam(PARAM_MESSAGE, true)->value();
    }
    else
    {
      message = "No message sent";
    }
    request->send(200, "text/plain", "Hello, POST: " + message);
  });

  server.onNotFound(notFound);

  server.begin();

}





#endif