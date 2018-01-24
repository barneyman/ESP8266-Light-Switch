#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

// if you're just using a switch driven driectly by the GPIO pins, enable this define
#define _SIMPLE_ONE_SWITCH


#include "mcp23017.h"

#include "debug_defines.h"

#include <ArduinoJson.h>
#include <FS.h>

#include <vector>
#include <algorithm> 

//#define _ERASE_JSON_CONFIG
#define _JSON_CONFIG_FILE "/config.json"

#define JSON_STATIC_BUFSIZE	2048
StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;

#ifdef _DEBUG
#define _TEST_WFI_STATE	
#endif




//#define _USING_OLED

#ifdef _USING_OLED
#include <ACROBOTIC_SSD1306.h>

class oledDebug : public debugBase
{
private:

	ACROBOTIC_SSD1306 *m_driver;

public:

	oledDebug(ACROBOTIC_SSD1306 *driver) :debugBase(), m_driver(driver)
	{}

protected:

	void internalDebug(String out)
	{
		if (m_driver)
			m_driver->putString(out);
	}
};



#endif

// first board that worked, although the pins were swapped around on the output
#define _BOARD_VER_1_1


// my libs
#include <hostname.h>

hostName esphostname;






// we run MDNS so we can be found by "esp8266_<last 3 bytes of MAC address>.local" by the RPI
MDNSResponder mdns;

#ifdef _SIMPLE_ONE_SWITCH

#define GPIO_RELAY	D1
#define GPIO_SWITCH D2

#define NUM_SOCKETS	1

#else
// number of relays & switches
#define NUM_SOCKETS	6
#endif

// millis timeouts
#define QUICK_SWITCH_TIMEOUT_DEFAULT	6000
#define BOUNCE_TIMEOUT_DEFAULT			100

#define _RESET_VIA_QUICK_SWITCH
//#define _IGNORE_BOUNCE_LOGIC	


#ifdef _RESET_VIA_QUICK_SWITCH
// how many transitions we have to see (on -> off and v-v) within Details.resetWindowms before we assume a resey has been asked for
#define RESET_ARRAY_SIZE 12
bool resetWIFI = false;
#endif


struct 
{
	// store network credentials in eeprom
	struct
	{
		String ssid;
		String password;
		bool configured;

		bool dhcp;
		// if not dhcp
		IPAddress ip, netmask, gateway;

	} wifi;

	// how long to wait for the light switch to settle
	unsigned long debounceThresholdms;

	// 6 switches in this time force an AP reset
	unsigned long resetWindowms;

	// the switches are named
	struct {

		String name;
		unsigned relay;

#ifndef _IGNORE_BOUNCE_LOGIC
		// when we saw this switch change state - used to debounce the switch
		unsigned long last_seen_bounce;
#endif

#ifdef _RESET_VIA_QUICK_SWITCH
		unsigned long lastSwitchesSeen[RESET_ARRAY_SIZE];
#endif

		// how many times we've see this PHYSICAL switch .. er ... switch
		unsigned long switchCount;

	} switches[NUM_SOCKETS];

} Details = {

	{
		"","",false, true
	},
	BOUNCE_TIMEOUT_DEFAULT,
	QUICK_SWITCH_TIMEOUT_DEFAULT,

#ifdef _SIMPLE_ONE_SWITCH

	{
		{ "Device A", 0 }
	}
#else

#ifdef _BOARD_VER_1_1
	{
		{ "Device A", 0 },
		{ "Device B", 5 },
		{ "Device C", 4 },
		{ "Device D", 3 },
		{ "Device E", 2 },
		{ "Device F", 1 }
	}
#else
	{
		{ "Device A", 0 },
		{ "Device B", 1 },
		{ "Device C", 2 },
		{ "Device D", 3 },
		{ "Device E", 4 },
		{ "Device F", 5 }
	}
#endif

#endif

};

// needs to be persisted or the event is unsubscribed
WiFiEventHandler onConnect, onDisconnect, onIPgranted;


volatile bool busyDoingSomethingIgnoreSwitch = false;


enum wifiMode { modeOff, modeAP, modeSTA, modeSTAspeculative, modeUnknown } ;
wifiMode currentMode = modeUnknown;

wifiMode ConnectWifi(wifiMode intent);


ESP8266WebServer server(80);

#ifdef _SIMPLE_ONE_SWITCH

#else

// light manual trigger IN, driven by the INT pin on the MCP
int inputSwitchPin = 14; // D5
// pin that controls power to the MCP
int resetMCPpin = 16;// D0;
// pin that controls power to the relay board
int powerRelayBoardNPN = 0; // d3


mcp23017AndRelay mcp(4, 5, resetMCPpin, powerRelayBoardNPN);

#endif


// how long we slow the web hots down for (millis)
#define _WEB_TAR_PIT_DELAY 200


#ifndef _SIMPLE_ONE_SWITCH

unsigned MapSwitchToRelay(unsigned switchNumber)
{
	unsigned relayNumber = switchNumber;

	if (relayNumber > NUM_SOCKETS || relayNumber < 0)
	{
		DEBUG(DEBUG_ERROR, Serial.printf("MapSwitchToRelay called out of bounds %u\n\r", relayNumber));
	}
	else
	{
		relayNumber=Details.switches[switchNumber].relay;
	}

	DEBUG(DEBUG_VERBOSE, Serial.printf("	MapSwitchToRelay %u -> %u\r\n", switchNumber, relayNumber));

	return relayNumber;

}

#endif


// ICACHE_RAM_ATTR  makes it ISR safe
void ICACHE_RAM_ATTR OnSwitchISR()
{
	// if we're up to our neck in something else (normally WIFI negotiation) ignore this
	if (busyDoingSomethingIgnoreSwitch)
	{
		DEBUG(DEBUG_INFO, Serial.println("	OnSwitchISR redundant"));

#ifndef _SIMPLE_ONE_SWITCH
		// ask what changed, clear interrupt, so we don't leave the INT hanging
		mcp.InterruptCauseAndCurrentState(true);
#endif
		return;
	}

	DEBUG(DEBUG_VERBOSE, Serial.println("	OnSwitchISR in"));


	// ask what changed, clear interrupt
	int causeAndState =
#ifdef _SIMPLE_ONE_SWITCH
		// fake the cause and reflect state of switch0
		(1<<8) | (digitalRead(GPIO_SWITCH)==HIGH?1:0);
#else
		mcp.InterruptCauseAndCurrentState(false);
#endif

	for (unsigned switchPort = 0; switchPort < NUM_SOCKETS; switchPort++)
	{
		DEBUG(DEBUG_VERBOSE,Serial.printf("Checking port %d [%04x]\r\n", switchPort, causeAndState));

		// +8 to get HIBYTE to see if this port CAUSED the interrupt
		if (causeAndState & (1 << (switchPort + 8)))
		{

#if defined(_RESET_VIA_QUICK_SWITCH) || !defined(_IGNORE_BOUNCE_LOGIC)
			unsigned long now = micros(), interval=0;
#endif

#ifndef _IGNORE_BOUNCE_LOGIC
			// gate against messy tactile/physical switches
			interval = now - Details.switches[switchPort].last_seen_bounce;

			DEBUG(DEBUG_VERBOSE, Serial.printf("%lu ms ", interval / 1000UL));

			if (interval >= (unsigned long)(Details.debounceThresholdms * 1000))
#endif
			{
#ifdef _RESET_VIA_QUICK_SWITCH
				// move the last seens along
				for (unsigned mover = 0; mover < RESET_ARRAY_SIZE - 1; mover++)
					Details.switches[switchPort].lastSwitchesSeen[mover] = Details.switches[switchPort].lastSwitchesSeen[mover + 1];

				Details.switches[switchPort].lastSwitchesSeen[RESET_ARRAY_SIZE - 1] =now/1000;

				DEBUG(DEBUG_INFO, Serial.printf("lastSwitchesSeen (ms) "));

				for (int each = 0; each < RESET_ARRAY_SIZE; each++)
				{
					DEBUG(DEBUG_INFO, Serial.printf("%lu ", Details.switches[switchPort].lastSwitchesSeen[each]));
				}

				DEBUG(DEBUG_INFO, Serial.printf("\n\r"));
#endif

				// having CAUSED the interrupt, reflect its STATE in the DoRelay call
				DoSwitch(switchPort, (causeAndState & (1 << switchPort)) ? true : false, false);

				Details.switches[switchPort].switchCount++;

#ifdef _RESET_VIA_QUICK_SWITCH

				// remember the last RESET_ARRAY_SIZE - i'm assuming we won't wrap
				// only reset if we're currently STA
				if (currentMode == wifiMode::modeSTA)
				{
					if (Details.switches[switchPort].lastSwitchesSeen[RESET_ARRAY_SIZE - 1] - Details.switches[switchPort].lastSwitchesSeen[0] < (Details.resetWindowms))
					{
						DEBUG(DEBUG_IMPORTANT, Serial.println("RESETTING WIFI!\n\r"));
						resetWIFI = true;
					}
				}
#endif
			}
#ifndef _IGNORE_BOUNCE_LOGIC
			else
			{
				DEBUG(DEBUG_INFO, Serial.printf("bounce ignored\n\r"));
			}
			Details.switches[switchPort].last_seen_bounce = now;
#endif
		}
	}

	DEBUG(DEBUG_VERBOSE, Serial.println("OnSwitchISR out"));

}

// honour current switch state
void RevertAllSwitch()
{

#ifdef _SIMPLE_ONE_SWITCH

	// read the current switch state, and reflect that in the relay state
	digitalWrite(GPIO_RELAY, digitalRead(GPIO_SWITCH)==HIGH);


#else
	// get the switch state
	for (int port = 0; port < NUM_SOCKETS; port++)
	{
		DoSwitch((port),
			mcp.readSwitch(port),
			false);

	}
#endif
}

// override switch state
void DoAllSwitch(bool state, bool force)
{
	DEBUG(DEBUG_INFO, Serial.printf("DoAllSwitch: %s %s\r\n", state ? "ON" : "off", force ? "FORCE" : ""));

#ifdef _SIMPLE_ONE_SWITCH

	digitalWrite(GPIO_RELAY, state?HIGH:LOW);

#else

	for (int Switch = 0; Switch < NUM_SOCKETS; Switch++)
	{
		// MapSwitchToRelay is redundant given we're doing them all
		DoSwitch((Switch), state, force);
	}
#endif
}

// if forceSwitchToReflect change polarity of input switch if necessary to reflect this request
void DoSwitch(unsigned portNumber, bool on, bool forceSwitchToReflect)
{
	if (portNumber > 7 || portNumber < 0)
	{
		DEBUG(DEBUG_ERROR, Serial.printf("DoSwitch called out of bounds %u\n\r", portNumber));
		return;
	}

	DEBUG(DEBUG_IMPORTANT, Serial.printf("DoSwitch: relay %d %s %s\r\n", portNumber, on ? "ON" : "off", forceSwitchToReflect ? "FORCE" : ""));

#ifdef _SIMPLE_ONE_SWITCH

	digitalWrite(GPIO_RELAY, on ? HIGH : LOW);

#else
	DoRelay(MapSwitchToRelay(portNumber),on);
	if (forceSwitchToReflect)
	{
		mcp.SetSwitch(portNumber, on);
	}
#endif
}

#ifndef _SIMPLE_ONE_SWITCH

// do, portNumber is 0 thru 7
void DoRelay(unsigned portNumber, bool on)
{
	if (portNumber > 7 || portNumber < 0)
	{
		DEBUG(DEBUG_ERROR, Serial.printf("DoRelay called out of bounds %u\n\r", portNumber));
		return;
	}


	DEBUG(DEBUG_IMPORTANT, Serial.printf("DoRelay: relay %d %s\r\n", portNumber, on?"ON":"off"));

	mcp.SetRelay(portNumber, on);

}

#endif



void WriteJSONconfig()
{
	DEBUG(DEBUG_INFO, Serial.println("WriteJSONconfig"));

	// try to create it
	fs::File json = SPIFFS.open("/config.json", "w");

	if (!json)
	{
		DEBUG(DEBUG_ERROR, Serial.println("failed to create json"));
		return;
	}

	//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
	jsonBuffer.clear();

	JsonObject &root = jsonBuffer.createObject();

	root["debounceThresholdms"] = Details.debounceThresholdms;
	root["resetWindowms"] = Details.resetWindowms;

	JsonObject &wifi = root.createNestedObject("wifi");

	wifi["configured"] = Details.wifi.configured;
	if (Details.wifi.configured)
	{
		wifi["password"] = Details.wifi.password;
		wifi["ssid"] = Details.wifi.ssid;

		if (!Details.wifi.dhcp)
		{
			JsonObject &staticDetails = wifi.createNestedObject("network");
			staticDetails["ip"] = Details.wifi.ip.toString();
			staticDetails["gateway"] = Details.wifi.gateway.toString();
			staticDetails["mask"] = Details.wifi.netmask.toString();
		}

	}

#ifndef _SIMPLE_ONE_SWITCH
	AddMapToJSON(root, NUM_SOCKETS);
#endif

	DEBUG(DEBUG_VERBOSE, Serial.printf("jsonBuffer.size used : %d\n\r", jsonBuffer.size()));

	///////////////////// written here

	String jsonText;
	root.prettyPrintTo(jsonText);

	DEBUG(DEBUG_VERBOSE, Serial.printf("JSON : -- %s --\n\r", jsonText.c_str()));

	json.write((byte*)jsonText.c_str(), jsonText.length());

	DEBUG(DEBUG_VERBOSE, Serial.println("JSON : written"));

	json.close();

	DEBUG(DEBUG_VERBOSE, Serial.println("JSON : closed"));

}


void ReadJSONconfig()
{
	DEBUG(DEBUG_INFO, Serial.println("ReadJSON"));

#ifdef _ERASE_JSON_CONFIG
	DEBUG(DEBUG_IMPORTANT, Serial.println("erasing JSON file"));
	SPIFFS.remove(_JSON_CONFIG_FILE);
#endif

	if (!SPIFFS.exists(_JSON_CONFIG_FILE))
	{
		DEBUG(DEBUG_IMPORTANT, Serial.printf("'%s' does not exist\n\r", _JSON_CONFIG_FILE));
		// file does not exist
		WriteJSONconfig();

		return;

	}
	// first - see if the file is there
	fs::File json = SPIFFS.open(_JSON_CONFIG_FILE, "r");

	String jsonString = json.readString();

	json.close();

	DEBUG(DEBUG_INFO, Serial.printf("JSON: (%d) -- %s --\n\r",jsonString.length(), jsonString.c_str()));

	//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
	jsonBuffer.clear();
	JsonObject& root = jsonBuffer.parseObject(jsonString);

	if (!root.success())
	{
		DEBUG(DEBUG_ERROR, Serial.println("JSON parse failed"));

		// kill it - and write it again
		SPIFFS.remove(_JSON_CONFIG_FILE);

		DEBUG(DEBUG_VERBOSE, Serial.println("JSON file deleted"));

		WriteJSONconfig();

		return;

	}
	else
	{
		DEBUG(DEBUG_VERBOSE, Serial.println("JSON parsed"));
	}

	Details.debounceThresholdms = root["debounceThresholdms"];
	Details.resetWindowms = root["resetWindowms"];

	JsonObject &wifi = root["wifi"];

	Details.wifi.configured = wifi["configured"];
	if (Details.wifi.configured)
	{
		Details.wifi.password = (const char*)(wifi["password"]);
		Details.wifi.ssid = (const char*)(wifi["ssid"]);

		JsonObject &staticDetails = wifi["network"];

		if(staticDetails.success())
		{
			Details.wifi.dhcp = false;

			if(Details.wifi.ip.fromString((const char*)staticDetails["ip"]) &&
				Details.wifi.gateway.fromString((const char*)staticDetails["gateway"]) &&
				Details.wifi.netmask.fromString((const char*)staticDetails["mask"]))
			{
				Details.wifi.dhcp = false;
			}
			else
			{
				DEBUG(DEBUG_ERROR, Serial.println("staticDetails parse failed, reverting to DHCP"));
				Details.wifi.dhcp = true;
			}
		}
		else
		{
			Details.wifi.dhcp = true;
		}

	}
	else
	{
		Details.wifi.password = String();
		Details.wifi.ssid = String();
	}

#ifndef _SIMPLE_ONE_SWITCH
	// add the switch map
	JsonArray &switchMap = root["switchMap"];
	if (switchMap.success())
	{
		for (unsigned each = 0; each < NUM_SOCKETS; each++)
		{
			JsonObject &theSwitch= switchMap[each]["switch"];
			if(theSwitch.success())
			{
				Details.switches[each].relay = theSwitch["relay"];
				Details.switches[each].name = (const char*)theSwitch["name"];
			}
			else
			{
				DEBUG(DEBUG_IMPORTANT, Serial.printf("switchMap switch %d not found\n\r",each));
			}
		}
	}
#endif

}




void BeginWebServer()
{
	server.begin();
	DEBUG(DEBUG_INFO, Serial.println("HTTP server started"));
}

// disjoin and rejoin, optionally force a STA attempt
wifiMode ConnectWifi(wifiMode intent)
{
	busyDoingSomethingIgnoreSwitch = true;

	WiFi.persistent(false);

	DEBUG(DEBUG_INFO, Serial.printf("ConnectWifi from %d to %d\n\r", currentMode, intent));

	// turn off wifi
	switch (currentMode)
	{
	case wifiMode::modeOff:
		break;
	case wifiMode::modeAP:
		WiFi.softAPdisconnect();
		break;
	case wifiMode::modeSTA:
	case wifiMode::modeSTAspeculative:
		WiFi.setAutoReconnect(false);
		WiFi.disconnect();
		break;
	case wifiMode::modeUnknown:
		break;
	}

	delay(1000);

	DEBUG(DEBUG_INFO, Serial.println("wifi disconnected"));

	if (intent == wifiMode::modeOff)
	{
		// turn wifi off
		WiFi.mode(WIFI_OFF);

	}

	if (intent == wifiMode::modeSTA || intent==wifiMode::modeSTAspeculative)
	{

		// turn bonjour off??
		DEBUG(DEBUG_VERBOSE, Serial.print("Attempting connect to "));
		DEBUG(DEBUG_VERBOSE, Serial.println(Details.wifi.ssid));

		WiFi.mode(WIFI_STA);

		WiFi.begin(Details.wifi.ssid.c_str(), Details.wifi.password.c_str());

		// Wait for connection
		for(int attempts=0;attempts<15;attempts++)
		{
			if (WiFi.status() != WL_CONNECTED) 
			{
				delay(1000);
				DEBUG(DEBUG_VERBOSE, Serial.print("."));
			}
			else
				break;
		}

		if (WiFi.status() == WL_CONNECTED)
		{

			DEBUG(DEBUG_INFO, Serial.println(""));
			DEBUG(DEBUG_INFO, Serial.printf("Connected to %s\n\r", Details.wifi.ssid.c_str()));
			DEBUG(DEBUG_INFO, Serial.printf("IP address: %s\n\r", WiFi.localIP().toString().c_str()));

			WiFi.setAutoReconnect(true);

			currentMode = wifiMode::modeSTA;

		}
		else
		{

			DEBUG(DEBUG_ERROR, Serial.println("FAILED to connect"));

			// depending on intent ...
			if (intent == wifiMode::modeSTAspeculative)
			{
				// we're trying this for the first time, we failed, fall back to AP
				return ConnectWifi(wifiMode::modeAP);
			}
		}
	}

	if (intent == wifiMode::modeAP)
	{
		// defaults to 192.168.4.1
		DEBUG(DEBUG_INFO, Serial.println("Attempting to start AP"));

		// we were unable to connect, so start our own access point
		WiFi.mode(WIFI_AP);
		WiFi.softAP(esphostname.c_str());

		DEBUG(DEBUG_IMPORTANT, Serial.printf("Started AP %s\n\r", WiFi.softAPIP().toString().c_str()));
	
		currentMode = wifiMode::modeAP;


	}

	BeginMDNSServer();

	BeginWebServer();

	busyDoingSomethingIgnoreSwitch = false;

	return currentMode;
}

// if we see more than x switches in y time, we reset the flash and enter AP mode (so we can be joined to another wifi network)

void BeginMDNSServer()
{
	if (mdns.begin(esphostname.c_str()))
	{
		mdns.addService("http", "tcp", 80);
		DEBUG(DEBUG_IMPORTANT, Serial.printf("MDNS responder started http://%s.local/\n\r", esphostname.c_str()));
	}
	else
	{
		DEBUG(DEBUG_ERROR, Serial.println("MDNS responder failed"));
	}

}


//void ResetWIFI()
//{
//	DEBUG(DEBUG_IMPORTANT, Serial.println("Resetting WIFI"));
//
//	wifiMode now = currentMode;
//
//	ConnectWifi(wifiMode::modeOff);
//
//	ConnectWifi(now);
//}

void RebootMe()
{
	// hangs the esp
	//WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while (1)wdt_reset();
	//ESP.restart();
}

#ifdef _RESET_VIA_QUICK_SWITCH

void ResetMe()
{
	DEBUG(DEBUG_IMPORTANT, Serial.println("Resetting"));

	resetWIFI = false;
	// clear the credentials
	Details.wifi.configured = false;
	Details.wifi.password = String();
	Details.wifi.ssid = String();
	WriteJSONconfig();
	// and reconnect as an AP
	ConnectWifi(wifiMode::modeAP);

}

#endif



void setup(void) 
{

#ifdef _USING_OLED
	oled.init();                      // Initialze SSD1306 OLED display
	oled.setFont(font8x8);            // Set font type (default 8x8)
	oled.clearDisplay();              // Clear screen
	oled.setTextXY(0, 0);              // Set cursor position, start of line 0
	oled.putString(esphostname);
	oled.putString(".local");
#endif




	// reset the bounce thresh-holds
	for (int eachSwitch = 0; eachSwitch < NUM_SOCKETS; eachSwitch++)
	{
#ifdef _RESET_VIA_QUICK_SWITCH
		// clean up the switch times
		memset(&Details.switches[eachSwitch].lastSwitchesSeen, 0, sizeof(Details.switches[eachSwitch].lastSwitchesSeen));
#endif

		Details.switches[eachSwitch].switchCount = 0;
#ifndef _IGNORE_BOUNCE_LOGIC
		Details.switches[eachSwitch].last_seen_bounce = 0;
#endif
	}

	// mandatory "let it settle" delay
	Serial.begin(115200);

	DEBUG(DEBUG_INFO, Serial.println("setup() running"));


	SPIFFS.begin();

	ReadJSONconfig();

	DEBUG(DEBUG_VERBOSE, Serial.println("starting"));
	DEBUG(DEBUG_IMPORTANT, Serial.println(esphostname));
	DEBUG(DEBUG_VERBOSE, Serial.printf("bounce %lu\n\r",Details.debounceThresholdms));
	DEBUG(DEBUG_VERBOSE, Serial.printf("reset %lu\n\r",Details.resetWindowms));

	enum wifiMode intent = wifiMode::modeUnknown;

	if (Details.wifi.configured)
	{
		DEBUG(DEBUG_INFO, Serial.println("credentials found"));
		DEBUG(DEBUG_VERBOSE, Serial.println(Details.wifi.ssid));
		DEBUG(DEBUG_VERBOSE, Serial.println(Details.wifi.password));
		intent = wifiMode::modeSTA;
	}
	else
	{
		DEBUG(DEBUG_WARN, Serial.println("WiFi not configured"));
		intent = wifiMode::modeAP;

	}


	// set callbacks for wifi
	onConnect=WiFi.onStationModeConnected([](const WiFiEventStationModeConnected&c) {
	
		DEBUG(DEBUG_IMPORTANT, Serial.printf("EVENT wifi connected %s\n\r", c.ssid.c_str()));

	});

	onIPgranted=WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
		IPAddress copy = event.ip;
		DEBUG(DEBUG_IMPORTANT, Serial.printf("EVENT IP granted %s\n\r", copy.toString().c_str()));

		if (!Details.wifi.dhcp)
		{
			if (WiFi.config(Details.wifi.ip, Details.wifi.gateway, Details.wifi.netmask))
			{
				DEBUG(DEBUG_IMPORTANT, Serial.printf("EVENT IP FORCED %s\n\r", WiFi.localIP().toString().c_str()));
			}
			else
				DEBUG(DEBUG_IMPORTANT, Serial.println("EVENT IP FORCED FAILED"));

		}

		DEBUG(DEBUG_IMPORTANT, Serial.printf("EVENT GATEWAY %s\n\r", WiFi.gatewayIP().toString().c_str()));

	});

	onDisconnect=WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &c) {
	
		DEBUG(DEBUG_WARN, Serial.println("EVENT disconnected "));

	});

#ifdef _SIMPLE_ONE_SWITCH

	// set the relay pin to output
	pinMode(GPIO_RELAY, OUTPUT);

	// and the switch pin to input
	pinMode(GPIO_SWITCH, INPUT_PULLUP);

	attachInterrupt(GPIO_SWITCH, OnSwitchISR, CHANGE);

#else
	// initialise the MCP
	DEBUG(DEBUG_VERBOSE, Serial.println("Initialising MCP"));
	mcp.Initialise();


	// preparing GPIOs
	pinMode(inputSwitchPin, INPUT_PULLUP);
	// the MCP interrupt is configured to fire on change, and goes LOW when fired
	attachInterrupt(inputSwitchPin, OnSwitchISR, ONLOW);
#endif

	// default off, and don't force switches
	DoAllSwitch(false,false);

	// try to connect to the wifi
	ConnectWifi(intent);

	// honour current switch state
	RevertAllSwitch();

	// set up the callback handlers for the webserver
	InstallWebServerHandlers();

	

}

// set up all the handlers for the web server
void InstallWebServerHandlers()
{
	DEBUG(DEBUG_VERBOSE, Serial.println("InstallWebServerHandlers IN"));

	// set up the json handlers
	// POST
	// all ON/OFF 
	// switch ON/OFF
	// revert

	server.on("/revert", []() {

		RevertAllSwitch();

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();
	});

	server.on("/all", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/all"));

		for (uint8_t i = 0; i < server.args(); i++)
		{
			if (server.argName(i) == "action")
			{
				DoAllSwitch(server.arg(i) == "on" ? true : false, true);
			}
		}

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();
	});

#ifndef _SIMPLE_ONE_SWITCH

	server.on("/toggle", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/toggle"));

		delay(_WEB_TAR_PIT_DELAY);

		// must be an arg
		if (!server.args())
		{
			return;
		}

		if (server.argName(0) == "relay")
		{
			if (mcp.ToggleRelay(server.arg(0).toInt()))
			{
				server.send(200, "text/html", "<html></html>");
			}
		}

	});
#endif

	server.on("/button", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/button"));

		// these have to be in port/action pairs
		if (server.args() % 2)
		{
			return;
		}


		for (uint8_t i = 0; i < server.args(); i += 2)
		{
			int port = -1; bool action = false;
			if (server.argName(i) == "port" && server.argName(i + 1) == "action")
			{
				port = server.arg(i).toInt();
				server.arg(i + 1).toLowerCase();
				action = server.arg(i + 1) == "on" ? true : false;
			}
			else if (server.argName(i) == "action" && server.argName(i + 1) == "port")
			{
				port = server.arg(i + 1).toInt();
				server.arg(i).toLowerCase();
				action = server.arg(i) == "on" ? true : false;
			}

			if (port != -1)
				DoSwitch(port, action, true);
		}

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();

	});

	server.on("/resetCounts", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/resetCounts"));

		delay(_WEB_TAR_PIT_DELAY);

		for (int eachSwitch = 0; eachSwitch < NUM_SOCKETS; eachSwitch++)
		{
			Details.switches[eachSwitch].switchCount = 0;
		}

		server.send(200,"text/html","<html/>");

	});

	server.on("/reboot", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/reboot"));

		RebootMe();

	});


	server.on("/resetWIFI", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/resetWIFI"));

		ResetMe();

	});




	server.on("/", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/"));

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();

	});


	// posted config
	server.on("/json/config", HTTP_POST, []() {

		DEBUG(DEBUG_INFO, Serial.println("json config posted"));
		DEBUG(DEBUG_INFO, Serial.println(server.arg("plain")));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(server.arg("plain"));

		long bounce = root["bouncems"];
		long reset = root["resetms"];

		// sanity check these values!

		Details.debounceThresholdms = bounce;
		Details.resetWindowms = reset;

		// extract the details
		WriteJSONconfig();
		delay(_WEB_TAR_PIT_DELAY);

		server.send(200, "text/html", "<html></html>");

		});


	server.on("/json/wifi", HTTP_POST, []() {

		DEBUG(DEBUG_INFO, Serial.println("json wifi posted"));
		DEBUG(DEBUG_INFO, Serial.println(server.arg("plain")));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(server.arg("plain"));

		String ssid = root["ssid"];
		String pwd = root["pwd"];

		// sanity check these values

		Details.wifi.ssid = ssid;
		Details.wifi.password = pwd;

		// dhcp or static?
		if (root["dhcp"] == 1)
		{
			Details.wifi.dhcp = true;
		}
		else
		{
			Details.wifi.dhcp = false;
			Details.wifi.ip.fromString( (const char*)root["ip"] );
			Details.wifi.gateway.fromString((const char*)root["gateway"]);
			Details.wifi.netmask.fromString((const char*)root["netmask"]);
		}

		// force attempt
		// if we fail we fall back to AP
		if (ConnectWifi(wifiMode::modeSTAspeculative) == wifiMode::modeSTA)
		{
			Details.wifi.configured = true;
		}
		else
		{
			Details.wifi.configured = false;
		}

		// and update json
		WriteJSONconfig();

		//delay(_WEB_TAR_PIT_DELAY);
		//server.send(200, "text/html", "<html></html>");


	});

	// GET

	server.on("/json/state", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json state called"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = esphostname;
		root["switchCount"] = NUM_SOCKETS;

		JsonArray &switchState = root.createNestedArray("switchState");
		for (unsigned each = 0; each < NUM_SOCKETS; each++)
		{
			JsonObject &switchRelay = switchState.createNestedObject();
			switchRelay["switch"] = each;
			switchRelay["name"] = Details.switches[each].name;
			switchRelay["relay"] = Details.switches[each].relay;
			switchRelay["state"] = 
#ifdef _SIMPLE_ONE_SWITCH
				// reflect the relay, not the switch
				digitalRead(GPIO_RELAY)==HIGH ? 1 : 0;
#else
				mcp.readSwitch(each) ? 1 : 0;
#endif
			switchRelay["stateChanges"] = Details.switches[each].switchCount;
		}

		String jsonText;
		root.prettyPrintTo(jsonText);

		DEBUG(DEBUG_VERBOSE, Serial.println(jsonText));

		server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		server.send(200, "application/json", jsonText);
	});

	server.on("/json/maxSwitchCount", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json maxSwitchCount called"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = esphostname;
		root["switchCount"] = NUM_SOCKETS;

		unsigned maxSwitch = -1, maxSwitchCount = 0;

		for (unsigned each = 0; each < NUM_SOCKETS; each++)
		{
			if (Details.switches[each].switchCount > maxSwitchCount)
			{
				maxSwitchCount = Details.switches[each].switchCount;
				maxSwitch = each;
			}

		}

		{
			root["maxSwitch"] = maxSwitch;
			root["maxSwitchCount"] = maxSwitchCount;

			DEBUG(DEBUG_INFO, Serial.printf("maxSwitch %u count %u \n\r",maxSwitch, maxSwitch));

		}

		String jsonText;
		root.prettyPrintTo(jsonText);

		DEBUG(DEBUG_VERBOSE, Serial.println(jsonText));

		server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		server.send(200, "application/json", jsonText);
	});



	server.on("/json/config", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json config called"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = esphostname;
		root["bouncems"] = Details.debounceThresholdms;
		root["resetms"] = Details.resetWindowms;

		String jsonText;
		root.prettyPrintTo(jsonText);

		DEBUG(DEBUG_VERBOSE, Serial.println(jsonText));

		server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		server.send(200, "application/json", jsonText);
	});

	server.on("/json/wifi", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json wifi called"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();
		root["name"] = esphostname;

		// let's get all wifis we can see
		int found=WiFi.scanNetworks();

		std::vector<std::pair<String, int>> allWifis;

		JsonArray &wifis = root.createNestedArray("wifi");

		for (int each = 0; each < found; each++)
		{
			allWifis.push_back(std::pair<String, int>(WiFi.SSID(each), WiFi.RSSI(each)));
			std::sort(allWifis.begin(), allWifis.end(), [](const std::pair<String, int> &a, const std::pair<String, int> &b) { return a.second > b.second; });

		}

		int maxFound = found < 10 ? found : 10;

		for (int each = 0; each < maxFound ; each++)
		{
			JsonObject &wifi = wifis.createNestedObject();
			wifi["ssid"] = allWifis[each].first;
			wifi["sig"] = allWifis[each].second;

			DEBUG(DEBUG_INFO, Serial.printf("%d '%s' %d \n\r",each+1, allWifis[each].first.c_str(), allWifis[each].second));

		}
		

		String jsonText;
		root.prettyPrintTo(jsonText);

		DEBUG(DEBUG_VERBOSE, Serial.println(jsonText));

		// do not cache
		server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		server.send(200, "text/json", jsonText);
	});

	// serve up everthing in SPIFFS
	SPIFFS.openDir("/");

	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {
		String file = dir.fileName();

		// cache it for an hour
		server.serveStatic(file.c_str(), SPIFFS, file.c_str(),"Cache-Control: public, max-age=3600");

		DEBUG(DEBUG_VERBOSE, Serial.printf("Serving %s\n\r", file.c_str()));

	}

	DEBUG(DEBUG_VERBOSE, Serial.println("InstallWebServerHandlers OUT"));

}

void SendServerPage()
{
	// given the current state of the device, send the appropriate page back
	File f;
	switch (currentMode)
	{
	case wifiMode::modeAP:
		f = SPIFFS.open("/APmode.htm", "r");
		break;
	case wifiMode::modeSTA:
		f = SPIFFS.open("/STAmode.htm", "r");
		break;
	case wifiMode::modeUnknown:
	default:
		f = SPIFFS.open("/Error.htm", "r");
		break;

	}

	server.streamFile(f, "text/html");
	f.close();

}

#ifndef _SIMPLE_ONE_SWITCH
void AddMapToJSON(JsonObject &root, unsigned numSockets)
{

	DEBUG(DEBUG_VERBOSE, Serial.printf("AddMapToJSON %d\n\r", numSockets));

	root["switchCount"] = numSockets;

	JsonArray &switchMap = root.createNestedArray("switchMap");

	for (unsigned each = 0; each < numSockets; each++)
	{
		JsonObject &theSwitch = switchMap.createNestedObject();
		theSwitch["switch"] = each;
		theSwitch["relay"] = Details.switches[each].relay;
		theSwitch["name"] = Details.switches[each].name.c_str();

	}

}

#endif

#ifdef _TEST_WFI_STATE
unsigned long lastTested = 0;
#define _TEST_WIFI_MILLIS	(15*60*1000)
#endif

void loop(void) 
{
#ifdef _RESET_VIA_QUICK_SWITCH
	if (resetWIFI)
		ResetMe();
#endif

	server.handleClient();

#ifdef _TEST_WFI_STATE

	unsigned long now = micros() / 1000;

	if (!lastTested || ((now - lastTested) > _TEST_WIFI_MILLIS))
	{
		WiFiMode_t currentState = WiFi.getMode();

		DEBUG(DEBUG_VERBOSE, Serial.printf("================ WIFI %d\n\r", currentState));

		WiFi.printDiag(Serial);

		lastTested = now;

	}

#endif

}