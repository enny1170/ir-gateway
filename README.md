# Introduction 
This is a fork of the famous [ESP8266-RcOid](https://www.rcoid.de/ESP8266.html) Project

the Hardware is changed to a Wemos D1 mini pro alternativly Wemos D1 32 (pincompatible) and the is software updated to support this.
And a custom Board are developed. To easy hookup this on the Module.
Most parts are rewritten, because I want to use this also as IR-Gateway togeter with my Smart-Home.
So we change to ESP Async-Webser and AsyncMQTT. Adding a better WebUI to support Storage and Management of Codes via Web-Ui.

To have a good Support for multiple Clients at the same Device we have designet to work without any blocking functions.
All API's needed to support the Rcoid-App exist. So the App is fully suported.

This project contains also several shematics.
1. a circurit and a pcb to simply Hookup on the given Module. (shematics/kicad/) [IR-Shield](https://www.pcbway.com/project/shareproject/IR_Shield_for_wemos_D1_pro_or_Wemos_D1_32.html)
2. 3D printable STL-Files as for a Cover (shematic/3dprint)
3. a Freecad Project to build or modify the Case (shematic/freecad)


# Usage (extract from Device Readme)
    This Firmware is a complete new Implementation of the [Rcoid](https://rcoid.de/) Project.
    This implementation supports the same Interface for the RcOid App. Has a extended Web UI and REST-Api. Devices can be used as IR-Gateway trought MQTT. 

    Sending and reciving IR-Codes new implemented for blocking free work. Also the storage of IR-Codes (Commands) added. This Commands can be used for MQTT and REST. 
    Upload and Downlowad of stored commands are possible. 

    A new unconfigured Device will start in Accesspoint-Mode and provide a 'ESP8266 for RCoid Access Point' with User pass and Password pass. 
    You are able to setup SSID and Password for your local WiFi-Connection.
    From the Root-Page you have also the possibility tor reboot, remove Web Config, and reset to Factory.
    Only from this page you can enable IR-reciving to capture IR-Codes.
    From the MQTT Page you can ste the Connection to your MQTT-Server and a Prefix used for the topic on MQTT. If the MQTT-Server adress empty or a '.' the MQTT communication will be disabled.
    In MQTT-Broker the device will be provide its local IP-Adress and available commands. The device subscribes /[Prefix]/Cmd. If the device receive a String in this topic, this will be interpreted as a Command. 
    The device try to find this Command and will send the Code immedaly.
    The same Functionality can be reached by sending a GET-Request to /cmd?button=xxxx. Where xxxx the name of the command. It can be taken from the Command-Page.

    The Command page are selfexplanating. But two points you have to know. Editing a Command you have to set a Code and a Name. The Name will be used as Filename to store the command on device Flash.
    And also as Command to raise from MQTT. The Clock Icon on the Command-Page will give you the possibilty to setup one Timer to send this command later.

    Some devices needs to receive a Code multiple times. So we add a Option Repeat for stored Commands. This will repeat the IR-Output according to this setting. Minimum is 1 maximum 5.
