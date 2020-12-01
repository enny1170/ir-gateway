#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <ArduinoJson.h>
#include <Config.h>
#include <mqttconf.h>
#include <WebServerImpl.h>
#include <ircode.h>
#include <ircodes.h>
#include <mqttimpl.h>

#define IR_PORT 0
#define IR_PORT_INVERT false
#define IR_RECEIVER_PORT 2
#define IR_RECEIVE_WAIT_TIME 30000

#ifndef OFFSET_START
  #define OFFSET_START   kStartOffset   // Usual rawbuf entry to start processing from.
#endif


// Netzwerkinformationen für Accesspoint
// Im AP-Modus ist der ESP8266 unter der IP 192.168.0.1 erreichbar
const char* ssidAP = "ESP8266 for RCoid Access Point";
const char* passwordAP = "passpass";  //Muss mindestens 8 Zeichen haben

char ir[1024];

/*
   Gibt die CPU Takte zurück, die seit dem Neustart vergangen sind.
   läuft ca. alle 53 Sekunden über
*/
#define RSR_CCOUNT(r)     __asm__ __volatile__("rsr %0,ccount":"=a" (r))
static inline uint32_t get_ccount()
{
  uint32_t ccount;
  RSR_CCOUNT(ccount);
  return ccount;
}


/*
   sendet ein RCoid-IR-Signal
   z.B.:
   http://ip.for.your.device/ir?code=38000,342,171,21,21,21,21,21,21,21,64,21,21,21,21,21,64,21,21,21,21,21,21,21,64,21,64,21,21,21,64,21,21,21,21,21,21,21,64,21,64,21,21,21,21,21,64,21,64,21,64,21,64,21,21,21,21,21,64,21,64,21,21,21,21,21,21,21,1
*/
void handleIr()
{
  serial_print_HttpInfo();

  pinMode(IR_PORT, OUTPUT);

  if (server.argName(0).equals("code"))
  {
    (server.arg(0) + ",0").toCharArray(ir, 1024);

    char *p; //Zeiger im Array
    unsigned int frequence = strtol(ir, &p, 10);
    p++; //Komma im String wird übersprungen
    unsigned int pulses = strtol(p, &p, 10);

    bool burst = true; //wir beginnen mit IR Licht

    unsigned int startTicks;
    unsigned int halfPeriodTicks = 40000000 / frequence;
    while (pulses != 0)
    {
      RSR_CCOUNT(startTicks);
      for (unsigned int i = 0 ; i < pulses * 2; i++)
      {
        if (IR_PORT_INVERT)
          digitalWrite(IR_PORT, (((i & 1) == 1) && burst) ? LOW : HIGH);
        else
          digitalWrite(IR_PORT, (((i & 1) == 1) && burst) ? HIGH : LOW);
        while (get_ccount() < startTicks + i * halfPeriodTicks) {} //Warten
      }
      burst = !burst;
      p++; //Komma im String wird übersprungen
      pulses = strtol(p, &p, 10);
    }
    digitalWrite(IR_PORT, IR_PORT_INVERT ? HIGH : LOW); //Am Ende IR immer AUS

  }
  else
  {
    handleNotFound();
    return;
  }
  htmlcontent = "OK";
  server.send(200, "text/plain", htmlcontent);
}

/*
  IRCode Handler without HTTP Server
  is used by CMD-Site and MQTT
*/
void handleIrCode(String code)
{

  pinMode(IR_PORT, OUTPUT);

  if (code.length()>0)
  {
    (code + ",0").toCharArray(ir, 1024);

    char *p; //Zeiger im Array
    unsigned int frequence = strtol(ir, &p, 10);
    p++; //Komma im String wird übersprungen
    unsigned int pulses = strtol(p, &p, 10);

    bool burst = true; //wir beginnen mit IR Licht

    unsigned int startTicks;
    unsigned int halfPeriodTicks = 40000000 / frequence;
    while (pulses != 0)
    {
      RSR_CCOUNT(startTicks);
      for (unsigned int i = 0 ; i < pulses * 2; i++)
      {
        if (IR_PORT_INVERT)
          digitalWrite(IR_PORT, (((i & 1) == 1) && burst) ? LOW : HIGH);
        else
          digitalWrite(IR_PORT, (((i & 1) == 1) && burst) ? HIGH : LOW);
        while (get_ccount() < startTicks + i * halfPeriodTicks) {} //Warten
      }
      burst = !burst;
      p++; //Komma im String wird übersprungen
      pulses = strtol(p, &p, 10);
    }
    digitalWrite(IR_PORT, IR_PORT_INVERT ? HIGH : LOW); //Am Ende IR immer AUS

  }
  else
  {
    Serial.println("Unknown Code Lngth");
    return;
  }
  Serial.println("IrCode send");
}

IRrecv irReceiver(IR_RECEIVER_PORT);
decode_results irDecoded;


/*
  wartet eine Zeit ab, bis am Receiver ein IR Signal decodiert wurde
  blockiert den ESP
*/
void handleReceiveIr()
{

  irReceiver.enableIRIn();  // Start the receiver
  unsigned long start = millis();

  String irData;

  while (millis() < start + IR_RECEIVE_WAIT_TIME)
  {
    if (irReceiver.decode(&irDecoded))
    {
      //Read and format IrData
      irData = "{\n";
      irData += "  \"Protocol\" : ";
      irData += "\"";
      irData += typeToString(irDecoded.decode_type, false);
      irData += "\",\n";
      irData += "  \"Value\" : \"";
      irData += uint64ToString(irDecoded.value, HEX);
      irData += "\",\n";
      irData += "  \"Length\" : \"";
      irData += irDecoded.rawlen;
      irData += "\",\n";
      irData += "  \"Address\" : \"";
      irData += irDecoded.address;
      irData += "\",\n";
      irData += "  \"Command\" : \"";
      irData += irDecoded.command;
      irData += "\",\n";
      irData += "  \"RCoid IR Code\" : \"";
      int freq = 38000;
      if (typeToString(irDecoded.decode_type, false).equals("SONY"))
        freq = 40000;
      irData += freq;
      for (int i = OFFSET_START; i < irDecoded.rawlen; i++)
      {
        irData += ",";
        irData += (int)(((irDecoded.rawbuf[i] * RAWTICK) * freq) / 1000000);
      }
      if (irDecoded.rawlen % 2 == 0)
      {
        irData += ",1";
      }
      irData += "\"\n";
      irData += "}";
      // create HTML
      htmlcontent = getHtmlPrefix();
      htmlcontent += "<div class='field'><div class='control'>";
      htmlcontent += irData;
      htmlcontent += "</div></div>";
      htmlcontent += F("<div class='field'><div class='buttons'><a class='button is-warning' href='/'><- back");
      htmlcontent += F("<a class='button is-success' href='/receiveir'>Receive IR-Signal</a></div></div>");
      htmlcontent += getHtmlSuffix();

      Serial.println(irData);

      server.send(200, "application/json", htmlcontent);

      irReceiver.resume();  // Receive the next value
      irReceiver.disableIRIn();  // Stopps the receiver

      return;
    }
    delay(100);
  };
  htmlcontent = getHtmlPrefix();
  htmlcontent += F("<div class='field'><div class='buttons'><a class='button is-warning' href='/'><- back");
  htmlcontent += F("<a class='button is-success' href='/receiveir'>Receive IR-Signal</a></div></div>");
  htmlcontent += getHtmlSuffix();

  server.send(408, "text/plain", htmlcontent);

}


/*
   Zeigt eine Liste mit verfügbaren Netzwerken im Serial-Monitor an
*/
void serial_print_Networks()
{
  int n = WiFi.scanNetworks();

  Serial.println("\nScan abgeschossen");
  if (n == 0)
    Serial.println("Kein WLAN gefunden");
  else
  {
    for (int i = 0; i < n; ++i)
    {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "" : " (verschlüsselt)");
    }
  }
  Serial.println("");
}



/*
   Wartet die gegebene Zeit t(in s)ab, bis die Verbindung zum WLAN besteht.
   gibt "true" zurück, wenn die Verbindung besteht.
   Anderenfalls wird "false" zurück gegeben.
*/
bool WaitForConnection(int t)
{
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    counter++;
    if (counter > t * 2)
      return false;
    delay(500);
    Serial.print(".");
  }
  return true;
}

/*
    ////////////////////////////////////   SETUP Access Point  ///////////////////////////////////////////

    Startet einen AP.
    Settings:
        SSID = "ESP8266 for RCoid Access Point"
        Pass = "passpass"
        IP   = "192.168.0.1"

    Wird ausgeführt, wenn der ESP keine Verbindung zum WLAN aufbauen konnte.

*/

void setupAP(void) {
  WiFi.mode(WIFI_AP_STA);

  serial_print_Networks();

  Serial.println("Setup Soft AP...");
  IPAddress apIP(192, 168, 0, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  WiFi.softAP(ssidAP, passwordAP, 3, false);
  delay(100);

  server.on("/", handleAPRoot);
  server.on("/setting", handleSetting);
  server.on("/ir", handleIr);
  server.on("/out", handleOut);
  server.on("/reset", handleReset);
  server.on("/getip", handleGetIp);
  server.on("/receiveir", handleReceiveIr);
  server.on("/mqtt", handleMqtt);
  server.on("/mqttset", handleMqttSettings);
  server.on("/cmds",handleCmds);
  server.on("/cmd",handleCmd);

  server.onNotFound(handleNotFound);

  server.begin();
  digitalWrite(LED_BUILTIN,LOW);
  Serial.println("HTTP server started");
}



/*
   ////////////////////////////////////   SETUP   ///////////////////////////////////////////
*/
void setup(void) {

  pinMode(IR_PORT, OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(IR_PORT, IR_PORT_INVERT ? HIGH : LOW);
  digitalWrite(LED_BUILTIN,HIGH);
  Serial.begin(115200);
  // Serial.println("Init Filesystem");
  
  initFileSystem();


  Serial.println("Test File");
  checkConfig();
  // try to load Config
  readConfig();

  checkMqttConfig();
  readMqttConfig();
  Serial.println(getESPDevName());
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("PASS: ");
  Serial.println(passwd);
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  if (ssid.length() > 1)
  {

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), passwd.c_str());  //Starte WIFI mit den Zugangsdaten aus dem EEPROM
    Serial.println("");

    if (WaitForConnection(10))
    {
      Serial.println("");
      Serial.print("Connected to ");
      Serial.println(ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      server.on("/", handleRoot);
      server.on("/ir", handleIr);
      server.on("/out", handleOut);
      server.on("/deletepass", handleDeletePass);
      server.on("/receiveir", handleReceiveIr);
      server.on("/mqtt", handleMqtt);
      server.on("/mqttset", handleMqttSettings);
      server.on("/cmds",handleCmds);
      server.on("/cmd",handleCmd);
      server.onNotFound(handleNotFound);

      server.begin();
      Serial.println("HTTP server started");
      Serial.println("Start MqttClient");
      //mqttConnect();
      return;
    }
  }
  setupAP();  //wenn keine Verbindung zum WLAN hergestellt werden konnte, wird der Accespoint aufgespannt.
}


/*
   Hauptschleife
*/
void loop()
{
  Serial.print(".");
  //server.handleClient();
  //mqttClient.loop();
}