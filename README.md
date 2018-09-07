# ESP8266-Light-Switch

Replacement firmware for Sonoff Basic, or firmware for your own build of esp8266 and solenoids 

This can handle (with #defines) 1 to 6 switches

Test Harness uses an NodeMCU, MCP23017 and 6 gang 240v 5v switching relay, this is a simple sketch that honours the manual switches 
and can also be remotely fired, using a simple HTTP get

When it starts it has no AP credentials to join your LAN, so offers up its own AP, which you can surf to
(192.168.4.1 by default) and give it credentials - it saves those in flash and will use them every time it reboots

It is possible to OTA update the firmware, and web pages

it was written using the Visual Micro addin for VS2015, but should compile perfectly well in other Arduino aware IDEs

I drive this from a Google Home, talking to Home Assistant ( https://www.home-assistant.io/ ) running on an RPI-3B


![Board - 6switch](https://github.com/barneyman/ESP8266-Light-Switch/blob/master/6switch.jpg)

