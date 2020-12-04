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

WiFiClient wifiClient;
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

#ifdef ESP32
void wifiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("Connected to AP!");
}
void wifiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("Disconnected from Wi-Fi.");
  //reboot the ESP
  ESP.restart();
  //mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  //wifiReconnectTimer.once(2, connectToWifi);
}
void setupWiFiEvents()
{
    WiFi.onEvent(wifiStationConnected,SYSTEM_EVENT_STA_CONNECTED);
    WiFi.onEvent(wifiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
}
#else
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  //reboot the ESP
  ESP.restart();
  //mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  //wifiReconnectTimer.once(2, connectToWifi);
}
#endif
//Ticker wifiReconnectTimer;

// actually in main, should be moved to Ircodes
extern void handleIrCode(String code);

void connectToMqtt() {
  if(mqttServer==".")
  {
    Serial.println("MQTT is disabled. Do not connect.");
  }
  else
  {
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
  }
}

void onMqttConnect(bool sessionPresent)
{
    Serial.println("Connected to MQTT.");
    IPAddress ip = WiFi.localIP();
    // Once connected, publish current IP-Address
    mqttClient.publish((mqttPrefix + "/IpAddress").c_str(), 1, true, (String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3])).c_str());
    // publish available IrCmds
    mqttClient.publish((mqttPrefix + "/Cmds").c_str(), 2, true, getCmds().c_str());
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
            mqttClient.publish((mqttPrefix + "/State").c_str(), 1, false, (payloadCmd + " unknown CMD").c_str());
        }
        else
        {
            Serial.printf("Handle IrCode for CMD '%s' Descr: %s\n", cmdCode.Cmd.c_str(), cmdCode.Description.c_str());
            Serial.println(cmdCode.Code);
            handleIrCode(cmdCode.Code);
            mqttClient.publish((mqttPrefix + "/State").c_str(), 1, false, payloadCmd.c_str());
        }
    }
    else
    {
        mqttClient.publish((mqttPrefix + "/State").c_str(), 1, false, "Unexpected topic");
    }
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setupMqtt()
{
  readMqttConfig();
  mqttClient.disconnect(true);
#ifdef ESP8266
  //wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
#else
  setupWiFiEvents();
#endif
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  Serial.println("Setup MQTT-Client...");
  Serial.print("Server: ");
  Serial.println(mqttServer);
  Serial.print("Port: ");
  Serial.println(mqttPort);
  Serial.print("Prefix: ");
  Serial.println(mqttPrefix);
  mqttClient.setServer(mqttServer.c_str(), mqttPort.toInt());
  mqttClient.setClientId(mqttPrefix.c_str());
  connectToMqtt();
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