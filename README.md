# ESP8266-Light-Switch
an ESP01 that switches a relay, and also handles the switch wire

**There is an additional branch - 6switch - that looks after 6 (up to 8) relays, using an MCP23017**

using an ESP8266-ESP01, and a 240v 5v switching relay, this is a simple sketch that honours the manual switch 
and can also be remotely fired, using a simple HTTP get

When it starts it has no AP credentials to join your LAN, so offers up its own AP, which you can surf to
(192.168.4.1 by default) and give it credentials - it saves those in flash and will use them every time it reboots

6 quick switches of the manual wire in 3 seconds will force it to forget those credentials and again offer the join page on
its own AP

it was written using the Visual Micro addin for VS2015, but should compile perfectly well in other Arduino aware IDEs

I drive this from a Google Home, talking to HA-Bridge ( https://github.com/bwssytems/ha-bridge ) running on an RPI-zero-W

