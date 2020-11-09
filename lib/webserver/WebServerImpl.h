#ifndef WEBSERVERIMPL_H
#define WEBSERVERIMPL_H

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Arduino.h>

ESP8266WebServer server(80);
String htmlcontent;

/*
    Returns HTML-Prefix for each Page
*/
String getHtmlPrefix()
{
    return F("<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'> \
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
}

/*
    Returns HTML-Prefix for each Page
*/
String getHtmlSuffix()
{
    return F("</div></div></section></body></html>");
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
  if (qsid.length() > 0 && qpass.length() > 0)
  {
    writeConfig(qsid,qpass);

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


#endif