#ifndef MQTTIMPL_H
#define MQTTIMPL_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <mqttconf.h>
#include <ircodes.h>

WiFiClient wifiClient;


extern void handleIrCode(String code);

// this callback will called by received MQTT Message
void mqttMsgCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String payloadCmd;
  for (uint i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadCmd+=(char)payload[i];
  }
  Serial.println();
  IRcode cmdCode=readIrCmd(payloadCmd);
  if(cmdCode.Description=="empty")
  {
      Serial.printf("'%s' unknown CMD\n",payloadCmd.c_str());
      mqttClient.publish((mqttPrefix +"/State").c_str(),(payloadCmd + " unknown CMD").c_str());
  }
  else
  {
      Serial.printf("Handle IrCode for CMD '%s' Descr: %s\n",cmdCode.Cmd.c_str(),cmdCode.Description.c_str());
      Serial.println(cmdCode.Code);
      handleIrCode(cmdCode.Code);
      mqttClient.publish((mqttPrefix+"/State").c_str(),payloadCmd.c_str());
  }
  
}


String mqttReconnect()
{
    IPAddress ip = WiFi.localIP();
    if (WiFi.getMode() == WiFiMode::WIFI_STA)
    {   
        // Loop until we're reconnected
        while (!mqttClient.connected())
        {
            Serial.print("Attempting MQTT connection...");
            // Create a random client ID
            String clientId = getESPDevName();
            // Attempt to connect
            if (mqttClient.connect(clientId.c_str()))
            {
                Serial.println("connected");
                // Once connected, publish current IP-Address
                mqttClient.publish((mqttPrefix + "/IpAddress").c_str(), (String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3])).c_str());
                // publish available IrCmds
                mqttClient.publish((mqttPrefix + "/Cmds").c_str(), getCmds().c_str());
                // ... and resubscribe
                mqttClient.subscribe((mqttPrefix + "/Cmd").c_str());
            }
            else
            {
                Serial.print("failed, rc=");
                Serial.print(mqttClient.state());
                Serial.println(" try again in 5 seconds");
                // Wait 5 seconds before retrying
                delay(5000);
            }
        }
        return "Connected";
    }
    else
    {
        Serial.println("Do not connect MQTT WiFi in AP Mode");
        return "Do not connect MQTT WiFi in AP Mode";
    }
}

// reset Connection and create a new one
void mqttConnect()
{
    readMqttConfig();
    Serial.print("MQTT Server: ");
    Serial.println(mqttServer);
    Serial.print("MQTT Port: ");
    Serial.println(mqttPort);
    if(mqttClient.connected())
    {
        mqttClient.disconnect();
    }
    if(mqttServer!=".")
    {
        mqttClient.setServer(mqttServer.c_str(),(uint16_t)mqttPort.toInt());
        mqttClient.setCallback(mqttMsgCallback);
        if(mqttUser.length()>0)
        {
            // User settings ignored at this moment
        }
        mqttReconnect();
    }
}

#endif