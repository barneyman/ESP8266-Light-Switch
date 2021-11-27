# ESP8266-Light-Switch

I now use Home Assistant to drive this, it's also changed to support other devices - it's on my list to update this readme

When it starts it has no AP credentials to join your LAN, so offers up its own AP, which you can surf to
(192.168.4.1 by default) and give it credentials - it saves those in flash and will use them every time it reboots

It is possible to OTA update the firmware, and web pages

it was written using the Arduino extension for VSCode, but should compile perfectly well in other Arduino aware IDEs

It talks to [Home Assistant](https://github.com/home-assistant/core) using my [custom component](https://github.com/barneyman/custom_components) - there is also an update [Addon](https://github.com/barneyman/updateServerAddOnHA)

Example six-way switch
![Board - 6switch](https://github.com/barneyman/ESP8266-Light-Switch/blob/master/6switch.jpg)

