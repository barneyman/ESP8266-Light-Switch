# ESP8266-Light-Switch
an ESP01 that switches 6 relays, and also handles the switch wires

using an NodeMCU, MCP23017 and 6 gang 240v 5v switching relay, this is a simple sketch that honours the manual switchs 
and can also be remotely fired, using a simple HTTP get

When it starts it has no AP credentials to join your LAN, so offers up its own AP, which you can surf to
(192.168.4.1 by default) and give it credentials - it saves those in flash and will use them every time it reboots

it was written using the Visual Micro addin for VS2015, but should compile perfectly well in other Arduino aware IDEs

I drive this from a Google Home, talking to HA-Bridge ( https://github.com/bwssytems/ha-bridge ) running on an RPI-zero-W

