#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

#include "mcp23017.h"

#include "debug_defines.h"

#include <ArduinoJson.h>
#include <FS.h>


//#define _ERASE_JSON_CONFIG
#define _JSON_CONFIG_FILE "/config.json"

#define JSON_STATIC_BUFSIZE	2048

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

// first board that worked, althought the pins were swapped around on the output
#define _BOARD_VER_1_1




extern "C" {

#include <user_interface.h>		// for system_get_chip_id();

}





// we run MDNS so we can be found by "esp8266_<last 3 bytes of MAC address>.local" by the RPI
MDNSResponder mdns;

// number of relays & switches
#define NUM_SOCKETS	6

// millis timeouts
#define QUICK_SWITCH_TIMEOUT_DEFAULT	6000
#define BOUNCE_TIMEOUT_DEFAULT			100

#define _RESET_VIA_QUICK_SWITCH
//#define _IGNORE_BOUNCE_LOGIC	


#ifdef _RESET_VIA_QUICK_SWITCH
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
		"","",false
	},
	BOUNCE_TIMEOUT_DEFAULT,
	QUICK_SWITCH_TIMEOUT_DEFAULT,

#ifdef _BOARD_VER_1_1
	{
		{ "Switch 0", 0 },
		{ "Switch 1", 5 },
		{ "Switch 2", 4 },
		{ "Switch 3", 3 },
		{ "Switch 4", 2 },
		{ "Switch 5", 1 }
	}
#else
	{
		{ "Switch 0", 0 },
		{ "Switch 1", 1 },
		{ "Switch 2", 2 },
		{ "Switch 3", 3 },
		{ "Switch 4", 4 },
		{ "Switch 5", 5 }
	}
#endif

};

// needs to be persisted or the event is unsubscribed
WiFiEventHandler onConnect, onDisconnect, onIPgranted;


volatile bool busyDoingSomethingIgnoreSwitch = false;

String esphostname = "esp8266_";


enum wifiMode { modeOff, modeAP, modeSTA, modeSTAspeculative, modeUnknown } ;
wifiMode currentMode = modeUnknown;

wifiMode ConnectWifi(wifiMode intent);


ESP8266WebServer server(80);

// light manual trigger IN
int inputSwitchPin = 14; // D5
int resetMCPpin = 16;// D0;
int powerRelayBoardNPN = 0; // d3

//mcp23017 mcp(4, 5, resetMCPpin);
mcp23017AndRelay mcp(4, 5, resetMCPpin, powerRelayBoardNPN);



// how long we slow the web hots down for (millis)
#define _WEB_TAR_PIT_DELAY 200




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





void OnSwitchISR()
{
	// if we're up to our neck in something else (normally WIFI negotiation) ignore this
	if (busyDoingSomethingIgnoreSwitch)
	{
		DEBUG(DEBUG_WARN, Serial.println("	OnSwitchISR redundant"));
		// ask what changed, clear interrupt
		mcp.InterruptCauseAndCurrentState(true);
		return;
	}

	DEBUG(DEBUG_VERBOSE, Serial.println("	OnSwitchISR in"));


	// ask what changed, clear interrupt
	int causeAndState =
		mcp.InterruptCauseAndCurrentState(false);

	for (unsigned switchPort = 0; switchPort < NUM_SOCKETS; switchPort++)
	{
		DEBUG(DEBUG_VERBOSE,Serial.printf("Checking port %d\r\n", switchPort));

		// +8 to get HIBYTE to see if this port CAUSED the interrupt
		if (causeAndState & (1 << (switchPort + 8)))
		{

#if defined(_RESET_VIA_QUICK_SWITCH) || !defined(_IGNORE_BOUNCE_LOGIC)
			unsigned long now = micros(), interval=0;
#endif

#ifndef _IGNORE_BOUNCE_LOGIC
			// gate against messy tactile/physical switches
			interval = now - Details.switches[switchPort].last_seen_bounce;

			DEBUG(DEBUG_WARN, Serial.printf("%lu ms ", interval / 1000UL));

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

				// remember the last 6 - i'm assuming we won't wrap
				// only reset if we're currently STA
				if (currentMode == wifiMode::modeSTA)
				{
					if (Details.switches[switchPort].lastSwitchesSeen[RESET_ARRAY_SIZE - 1] - Details.switches[switchPort].lastSwitchesSeen[0] < (Details.resetWindowms))
					{
						DEBUG(DEBUG_WARN, Serial.println("RESETTING WIFI!\n\r"));
						resetWIFI = true;
					}
				}
#endif
			}
#ifndef _IGNORE_BOUNCE_LOGIC
			else
			{
				DEBUG(DEBUG_WARN, Serial.printf("bounce ignored\n\r"));
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
	// get the switch state
	for (int port = 0; port < NUM_SOCKETS; port++)
	{
		DoSwitch((port),
			mcp.readSwitch(port),
			false);

	}
}

// override switch state
void DoAllSwitch(bool state, bool force)
{
	DEBUG(DEBUG_IMPORTANT, Serial.printf("DoAllSwitch: %s %s\r\n", state ? "ON" : "off", force ? "FORCE" : ""));

	for (int Switch = 0; Switch < NUM_SOCKETS; Switch++)
	{
		// MapSwitchToRelay is redundant given we're doing them all
		DoSwitch((Switch), state, force);
	}
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

	DoRelay(MapSwitchToRelay(portNumber),on);
	if (forceSwitchToReflect)
	{
		mcp.SetSwitch(portNumber, on);
	}

}

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

	StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;

	JsonObject &root = jsonBuffer.createObject();

	root["debounceThresholdms"] = Details.debounceThresholdms;
	root["resetWindowms"] = Details.resetWindowms;

	JsonObject &wifi = root.createNestedObject("wifi");

	wifi["configured"] = Details.wifi.configured;
	if (Details.wifi.configured)
	{
		wifi["password"] = Details.wifi.password;
		wifi["ssid"] = Details.wifi.ssid;
	}

	AddMapToJSON(root, NUM_SOCKETS);


	///////////////////// written here

	String jsonText;
	root.prettyPrintTo(jsonText);

	DEBUG(DEBUG_VERBOSE, Serial.printf("JSON : %s\n\r", jsonText.c_str()));

	json.write((byte*)jsonText.c_str(), jsonText.length());

	json.close();
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
		DEBUG(DEBUG_IMPORTANT, Serial.println("Config.json does not exist"));
		// file does not exist
		WriteJSONconfig();

		return;

	}
	// first - see if the file is there
	fs::File json = SPIFFS.open(_JSON_CONFIG_FILE, "r");

	String jsonString = json.readString();

	json.close();

	DEBUG(DEBUG_IMPORTANT, Serial.printf("JSON: %s\n\r", jsonString.c_str()));

	StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;

	JsonObject& root = jsonBuffer.parseObject(jsonString);

	if (!root.success())
	{
		DEBUG(DEBUG_ERROR, Serial.println("JSON parse failed"));
	}

	Details.debounceThresholdms = root["debounceThresholdms"];
	Details.resetWindowms = root["resetWindowms"];

	JsonObject &wifi = root["wifi"];

	Details.wifi.configured = wifi["configured"];
	if (Details.wifi.configured)
	{
		Details.wifi.password = (const char*)(wifi["password"]);
		Details.wifi.ssid = (const char*)(wifi["ssid"]);
	}
	else
	{
		Details.wifi.password = String();
		Details.wifi.ssid = String();
	}

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

	DEBUG(DEBUG_INFO, Serial.println("ConnectWifi"));

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

	DEBUG(DEBUG_WARN, Serial.println("wifi disconnected"));

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

			DEBUG(DEBUG_IMPORTANT, Serial.println("FAILED to connect"));

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


void ResetWIFI()
{
	DEBUG(DEBUG_IMPORTANT, Serial.println("Resetting WIFI"));

	wifiMode now = currentMode;

	ConnectWifi(wifiMode::modeOff);

	ConnectWifi(now);
}

void RebootMe()
{
	// hangs the esp
	//WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while (1)wdt_reset();
	//ESP.restart();
}

#ifdef _RESET_VIA_QUICK_SWITCH

void ResetMe()
{
	DEBUG(DEBUG_WARN, Serial.println("Resetting"));

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
	char idstr[20];
	sprintf(idstr,"%0x", system_get_chip_id());
	esphostname += idstr;


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
	delay(1000);
	Serial.begin(115200);


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
		DEBUG(DEBUG_WARN, Serial.println("no stored credentials"));
		intent = wifiMode::modeAP;

	}


	// set callbacks for wifi
	onConnect=WiFi.onStationModeConnected([](const WiFiEventStationModeConnected&c) {
	
		DEBUG(DEBUG_IMPORTANT, Serial.printf("EVENT wifi connected %s\n\r", c.ssid.c_str()));

	});

	onIPgranted=WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
		IPAddress copy = event.ip;
		DEBUG(DEBUG_IMPORTANT, Serial.printf("EVENT IP granted %s\n\r", copy.toString().c_str()));

// my router doesn't understand *LAN* routes
#define _HARD_CODED_IP_ADDRESS
#ifdef _HARD_CODED_IP_ADDRESS
		if(WiFi.config(IPAddress(192, 168, 42, 18), IPAddress(192, 168, 42, 250), IPAddress(255, 255, 255, 0)))
		{
			DEBUG(DEBUG_IMPORTANT, Serial.printf("EVENT IP FORCED %s\n\r", WiFi.localIP().toString().c_str()));
		}
		else
			DEBUG(DEBUG_IMPORTANT, Serial.println("EVENT IP FORCED FAILED"));
#endif
		DEBUG(DEBUG_IMPORTANT, Serial.printf("EVENT GATEWAY %s\n\r", WiFi.gatewayIP().toString().c_str()));

	});

	onDisconnect=WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &c) {
	
		DEBUG(DEBUG_WARN, Serial.println("EVENT disconnected "));

	});


	// initialise the MCP
	DEBUG(DEBUG_VERBOSE, Serial.println("Initialising MCP"));
	mcp.Initialise();


	// preparing GPIOs
	pinMode(inputSwitchPin, INPUT_PULLUP);
	// the MCP interrupt is configured to fire on change, and goes LOW when fired
	DEBUG(DEBUG_VERBOSE, Serial.println("Attach Interrupt"));
	attachInterrupt(inputSwitchPin, OnSwitchISR, ONLOW);


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


	server.on("/toggle", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/toggle"));

		delay(_WEB_TAR_PIT_DELAY);

		// these have to be in port/action pairs
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




	server.on("/", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/"));

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();

	});


	// posted config
	server.on("/json/config", HTTP_POST, []() {

		DEBUG(DEBUG_INFO, Serial.println("json config posted"));
		DEBUG(DEBUG_INFO, Serial.println(server.arg("plain")));

		StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
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

		StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(server.arg("plain"));

		String ssid = root["ssid"];
		String pwd = root["pwd"];

		// sanity check these values

		Details.wifi.ssid = ssid;
		Details.wifi.password = pwd;


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

		delay(_WEB_TAR_PIT_DELAY);

		server.send(200, "text/html", "<html></html>");


	});

	// GET

	server.on("/json/state", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json state called"));

		StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;

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
			switchRelay["state"] = mcp.readSwitch(each) ? 1 : 0;
			switchRelay["stateChanges"] = Details.switches[each].switchCount;
		}

		String jsonText;
		root.prettyPrintTo(jsonText);

		server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		server.send(200, "application/json", jsonText);
	});

	server.on("/json/config", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json config called"));

		StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = esphostname;
		root["bouncems"] = Details.debounceThresholdms;
		root["resetms"] = Details.resetWindowms;

		String jsonText;
		root.prettyPrintTo(jsonText);

		server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		server.send(200, "application/json", jsonText);
	});

	server.on("/json/wifi", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json wifi called"));

		StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;

		JsonObject &root = jsonBuffer.createObject();
		root["name"] = esphostname;

		// let's get all wifis we can see
		int found=WiFi.scanNetworks();

		JsonArray &wifis = root.createNestedArray("wifi");

		for (int each = 0; each < found; each++)
		{
			JsonObject &wifi = wifis.createNestedObject();
			wifi["ssid"] = WiFi.SSID(each);
			wifi["sig"] = WiFi.RSSI(each);

		}

		String jsonText;
		root.prettyPrintTo(jsonText);
		
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

void AddMapToJSON(JsonObject &root, unsigned numSockets)
{
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

#define _TEST_WFI_STATE	

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