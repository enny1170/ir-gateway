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

// The IR Hardware are defined in ircodes.h

// Netzwerkinformationen für Accesspoint
// Im AP-Modus ist der ESP8266 unter der IP 192.168.0.1 erreichbar
const char *ssidAP = "ESP8266 for RCoid Access Point";
const char *passwordAP = "passpass"; //Muss mindestens 8 Zeichen haben


/***********************************************************************************************************
    ////////////////////////////////////   SETUP Access Point  ///////////////////////////////////////////

    Create Soft AP.
    Settings:
        SSID = "ESP8266 for RCoid Access Point"
        Pass = "passpass"
        IP   = "192.168.0.1"

    Will be done if no Wifi configured.
************************************************************************************************************/
void setupAP(void)
{
  WiFi.mode(WIFI_AP_STA);

  serial_print_Networks();

  Serial.println("Setup Soft AP...");
  IPAddress apIP(192, 168, 0, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  WiFi.softAP(ssidAP, passwordAP, 3, false);
  delay(100);
  configureWebServer();
#ifdef ESP8266
  digitalWrite(LED_BUILTIN, LOW);
#endif
  Serial.println("Soft AP 'ESP8266 for RCoid Access Point' online");
}

/********************************************************************************************
 * ////////////////////////////////////   SETUP   ///////////////////////////////////////////
*********************************************************************************************/
void setup(void)
{

  pinMode(IR_PORT, OUTPUT);
  digitalWrite(IR_PORT, IR_PORT_INVERT ? HIGH : LOW);
#ifdef ESP8266
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
#else
  //ESP32 devKit V2 has no onboard LED only Power
#endif
  Serial.begin(115200);
  Serial.print("Last Reset Cause: ");
  Serial.println(getResetReason());
  initFileSystem();
  checkConfig();
  // try to load Config
  readConfig();
  Serial.println("-- WiFi - Config --");
  Serial.println(getESPDevName());
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("PASS: ");
  Serial.println(passwd);
  Serial.print("DeviceName: ");
  Serial.println(deviceName);
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  checkMqttConfig();
  readMqttConfig();
  setupWiFiEvents();
  Serial.print("Last Reset Cause: ");
  Serial.println(getResetReason());

  if(ssid.length()>1)
  {
    //Try to connect configured WiFi
    WiFi.mode(WIFI_STA);
    connectToWiFi();
    setupMqtt();

  }
  else
  {
    setupAP();
  }
  configureWebServer();
  Serial.println("Setup finished");
}

/***********************************************************************************
 * Main Loop
************************************************************************************/
void loop()
{
  Serial.print(".");
  delay(500);
  //Call the IR Receiver Handler
  receive_ir_nonblock(true);
  sendIrCodeFromQueue();
  
}