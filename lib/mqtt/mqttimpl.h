///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation Module for MQTT Client
// YOU must define MQTTENABLE to enable this Module
// This can be done by #define MQTTENABLE
// ord as build_flags=-DMQTTENABLE in platform.ini
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MQTTIMPL_H
#define MQTTIMPL_H

#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <Ticker.h>
#include <AsyncMqttClient.h>

#include <mqttconf.h>
#include <ircodes.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ntpTimer.h>

WiFiClient wifiClient;
AsyncMqttClient mqttClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

callBackFunc_t funcPointer;
void testCallback(String cmd)
{
  Serial.println("\n ...... Timmer Callback received ........");
  IRcode code = readIrCmd(cmd);
  for (int i = 0; i < code.Repeat; i++)
  {
    addIrCodeToQueue(code.Code);  
  }
}
ntpTimer cmdTimer(testCallback);

Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;

char mqttLastWillTopic[1024];

void connectToMqtt() {
  if(mqttServer==".")
  {
    Serial.println("MQTT is disabled. Do not connect.");
  }
  else
  {
    Serial.println("Connecting to MQTT...");
    // delay(500);
    mqttClient.connect();
  }
}

void connectToWiFi()
{
  Serial.printf("\nConnecting Wifi %s ... ",ssid.c_str());
  WiFi.begin(ssid.c_str(), passwd.c_str());
  // if(WiFi.waitForConnectResult()!=WL_CONNECTED)
  // {
  //   Serial.println("+");
  //   // delay(1000);
  //   // WiFi.begin(ssid.c_str(), passwd.c_str());
  // }
}

#ifdef ESP32
void wifiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
      Serial.printf("\nWiFi connected to %s\n",ssid.c_str());
      IPAddress ip = WiFi.localIP();
      Serial.printf("Got IP-Address %s\n",(String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3])).c_str());
      Serial.printf("Try start Mqtt-Client to %s\n",mqttServer.c_str());
      timeClient.begin();
      connectToMqtt();
}

void wifiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWiFi);
}

#else

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  Serial.print("IP-Address: ");
  Serial.println(WiFi.localIP().toString());
  timeClient.begin();
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWiFi);
}

// void onWifiConnect(const WiFiEventStationModeGotIP& event)
// {
//       // Serial.printf("\nWiFi connected to %s\n",ssid.c_str());
//       // Serial.printf("IP Info: %s, Mask: %s, Gateway: %s\n",event.ip.toString().c_str(),event.mask.toString().c_str(),event.gw.toString().c_str());
//       // Serial.printf("Try start Mqtt-Client to %s\n",mqttServer.c_str());
//       // delay(500);
//       Serial.print("WiFi connected");
//       connectToMqtt();
// }

// void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
//   Serial.println("Disconnected from Wi-Fi.");
//   mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
//   wifiReconnectTimer.once(2, connectToWiFi);
// }
#endif

void setupWiFiEvents()
{
#ifdef ESP8266
    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
#else
    WiFi.onEvent(wifiStationConnected,SYSTEM_EVENT_STA_GOT_IP);
    WiFi.onEvent(wifiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
#endif
}

void onMqttConnect(bool sessionPresent)
{
    Serial.println("Connected to MQTT.");
    IPAddress ip = WiFi.localIP();
    // Once connected, publish current IP-Address
    mqttClient.publish((mqttPrefix + "/IpAddress").c_str(), 1, false, (String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3])).c_str());
    // publish available IrCmds
    mqttClient.publish((mqttPrefix + "/Cmds").c_str(), 2, false, getCmds().c_str());
    // publish online State
    mqttClient.publish((mqttPrefix + "/State").c_str(),2,true,"ONLINE");
    // publisch Device Name
    mqttClient.publish((mqttPrefix + "/DeviceName").c_str(),2,false,deviceName.c_str());
    // publisch Client Id
    mqttClient.publish((mqttPrefix + "/ClientId").c_str(),2,false,mqttClient.getClientId());
    // ... and resubscribe
    mqttClient.subscribe((mqttPrefix + "/Cmd").c_str(), 2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.print("Disconnected from MQTT Code ");
  Serial.println((int)reason);
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    String payloadCmd;
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (uint i = 0; i < len; i++)
    {
        Serial.print((char)payload[i]);
        payloadCmd += (char)payload[i];
    }
    Serial.println();
    if (String(topic).equals(mqttPrefix + "/Cmd"))
    {
        Serial.print("Expected Topic '");
        Serial.print((mqttPrefix + "/Cmd").c_str());
        Serial.println("' found");
        IRcode cmdCode = readIrCmd(payloadCmd);
        if (cmdCode.Description == "empty")
        {
            Serial.printf("'%s' unknown CMD\n", payloadCmd.c_str());
            mqttClient.publish((mqttPrefix + "/Message").c_str(), 1, false, (payloadCmd + " unknown CMD").c_str());
        }
        else
        {
            Serial.printf("Handle IrCode for CMD '%s' Descr: %s\n", cmdCode.Cmd.c_str(), cmdCode.Description.c_str());
            Serial.println(cmdCode.Code);
            for (int i = 0; i < cmdCode.Repeat; i++)
            {
              addIrCodeToQueue(cmdCode.Code);
            }
            mqttClient.publish((mqttPrefix + "/Message").c_str(), 1, false, payloadCmd.c_str());
        }
    }
    else
    {
        mqttClient.publish((mqttPrefix + "/Message").c_str(), 1, false, "Unexpected topic");
    }
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setupMqttEvents()
{
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
}

void setupMqtt()
{
  readMqttConfig();
  if(mqttClient.connected())
  {
    mqttClient.disconnect(true);
  }
  Serial.println("Setup MQTT-Client...");
  Serial.print("Server: ");
  Serial.println(mqttServer);
  Serial.print("Port: ");
  Serial.println(mqttPort);
  Serial.print("Prefix: ");
  Serial.println(mqttPrefix);
  Serial.println("Set Server");
  mqttClient.setServer(mqttServer.c_str(), mqttPort);
  Serial.print("ClientId: ");
  Serial.println(mqttClient.getClientId());
  //last Will topic must be a global variable, because set will gives obly the pointer to the content
  (mqttPrefix+"/State").toCharArray(mqttLastWillTopic,1024);
  Serial.print("LastWillTopic: ");
  Serial.println(mqttLastWillTopic);
  mqttClient.setWill(mqttLastWillTopic,2,true,"OFFLINE");
}

// Old implementations
// this callback will called by received MQTT Message
// void mqttMsgCallback(char* topic, byte* payload, unsigned int length) {
//   Serial.print("Message arrived [");
//   Serial.print(topic);
//   Serial.print("] ");
//   String payloadCmd;
//   for (uint i = 0; i < length; i++) {
//     Serial.print((char)payload[i]);
//     payloadCmd+=(char)payload[i];
//   }
//   Serial.println();
//   IRcode cmdCode=readIrCmd(payloadCmd);
//   if(cmdCode.Description=="empty")
//   {
//       Serial.printf("'%s' unknown CMD\n",payloadCmd.c_str());
//       mqttClient.publish((mqttPrefix +"/State").c_str(),(payloadCmd + " unknown CMD").c_str());
//   }
//   else
//   {
//       Serial.printf("Handle IrCode for CMD '%s' Descr: %s\n",cmdCode.Cmd.c_str(),cmdCode.Description.c_str());
//       Serial.println(cmdCode.Code);
//       handleIrCode(cmdCode.Code);
//       mqttClient.publish((mqttPrefix+"/State").c_str(),payloadCmd.c_str());
//   }
  
// }


// String mqttReconnect()
// {
//     IPAddress ip = WiFi.localIP();
//     if (WiFi.getMode() == WiFiMode::WIFI_STA)
//     {   
//         // Loop until we're reconnected
//         while (!mqttClient.connected())
//         {
//             Serial.print("Attempting MQTT connection...");
//             // Create a random client ID
//             String clientId = getESPDevName();
//             // Attempt to connect
//             if (mqttClient.connect(clientId.c_str()))
//             {
//                 Serial.println("connected");
//                 // Once connected, publish current IP-Address
//                 mqttClient.publish((mqttPrefix + "/IpAddress").c_str(), (String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3])).c_str());
//                 // publish available IrCmds
//                 mqttClient.publish((mqttPrefix + "/Cmds").c_str(), getCmds().c_str());
//                 // ... and resubscribe
//                 mqttClient.subscribe((mqttPrefix + "/Cmd").c_str());
//             }
//             else
//             {
//                 Serial.print("failed, rc=");
//                 Serial.print(mqttClient.state());
//                 Serial.println(" try again in 5 seconds");
//                 // Wait 5 seconds before retrying
//                 delay(5000);
//             }
//         }
//         return "Connected";
//     }
//     else
//     {
//         Serial.println("Do not connect MQTT WiFi in AP Mode");
//         return "Do not connect MQTT WiFi in AP Mode";
//     }
// }

// // reset Connection and create a new one
// void mqttConnect()
// {
//     readMqttConfig();
//     Serial.print("MQTT Server: ");
//     Serial.println(mqttServer);
//     Serial.print("MQTT Port: ");
//     Serial.println(mqttPort);
//     if(mqttClient.connected())
//     {
//         mqttClient.disconnect();
//     }
//     if(mqttServer!=".")
//     {
//         mqttClient.setServer(mqttServer.c_str(),(uint16_t)mqttPort.toInt());
//         mqttClient.setCallback(mqttMsgCallback);
//         if(mqttUser.length()>0)
//         {
//             // User settings ignored at this moment
//         }
//         mqttReconnect();
//     }
// }

#endif