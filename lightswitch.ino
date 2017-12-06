#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

#include "mcp23017.h"

#include "debug_defines.h"


// use eiher the flash or SIFFS
// if you use FLASH, you'll have to build/upload with _INITIALISE_EEPROM set once, then upload WITHOUT it set (or the eeprom will be erased every boot = BAD)
// if you use SIFFS you'll have to upload the config.json file in the data directory

#define _USE_JSON_NOT_EEPROM

#ifdef _USE_JSON_NOT_EEPROM
#include <ArduinoJson.h>
#include <FS.h>
//#define _ERASE_JSON_CONFIG
#define _JSON_CONFIG_FILE "/config.json"
#else
// run it ONCE with this flag set, just to write sane values into the EEPROM
//#define _INITIALISE_EEPROM
#include <EEPROM.h>
#endif


#define _USING_OLED

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


struct
{
	// store network credentials in eeprom
	struct
	{
#ifdef _USE_JSON_NOT_EEPROM
		String ssid;
		String password;
#else
		char ssid[128];
		char password[128];
#endif
		bool configured;

	} wifi;

	// how long to wait for the light switch to settle
	unsigned long debounceThresholdms;

	// 6 switches in this time force an AP reset
	unsigned long resetWindowms;

} Details;

// needs to be persisted or the event is unsubscribed
WiFiEventHandler onConnect, onDisconnect, onIPgranted;


volatile bool busyDoingSomethingIgnoreSwitch = false;

String esphostname = "esp8266_";


enum wifiMode { modeAP, modeSTA, modeUnknown } ;
wifiMode currentMode = modeUnknown;


void ConnectWifi(wifiMode intent);


ESP8266WebServer server(80);

// light manual trigger IN
int inputSwitchPin = 14; // D5
int resetMCPpin = 16;// D0;
int powerRelayBoardNPN = 0; // d3

//mcp23017 mcp(4, 5, resetMCPpin);
mcp23017AndRelay mcp(4, 5, resetMCPpin, powerRelayBoardNPN);


// number of relays & switches
#define NUM_SOCKETS	6

// how long we slow the web hots down for (millis)
#define _WEB_TAR_PIT_DELAY 200

#define _RESET_VIA_QUICK_SWITCH
//#define _IGNORE_BOUNCE_LOGIC	

#ifndef _IGNORE_BOUNCE_LOGIC
	unsigned long last_micros[NUM_SOCKETS];
#endif

#ifdef _RESET_VIA_QUICK_SWITCH
	#define RESET_ARRAY_SIZE 12
	unsigned long lastSwitchesSeen[NUM_SOCKETS][RESET_ARRAY_SIZE];
	bool resetWIFI = false;
#endif

	// millis timeouts
#define QUICK_SWITCH_TIMEOUT_DEFAULT	6000
#define BOUNCE_TIMEOUT_DEFAULT			100

	unsigned switchToRelayMap[NUM_SOCKETS] =
	{
#ifdef _BOARD_VER_1_1
		0,5,4,3,2,1
#else
		0,1,2,3,4,5
#endif
	};


unsigned MapSwitchToRelay(unsigned switchNumber)
{
	unsigned relayNumber = switchNumber;

	if (relayNumber > NUM_SOCKETS || relayNumber < 0)
	{
		DEBUG(DEBUG_ERROR, Serial.printf("MapSwitchToRelay called out of bounds %u\n\r", relayNumber));
	}
	else
	{
		relayNumber=switchToRelayMap[switchNumber];
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
			interval = now - last_micros[switchPort];

			DEBUG(DEBUG_WARN, Serial.printf("%lu ms ", interval / 1000UL));

			if (interval >= (unsigned long)(Details.debounceThresholdms * 1000))
#endif
			{
#ifdef _RESET_VIA_QUICK_SWITCH
				// move the last seens along
				memmove(&lastSwitchesSeen[switchPort][0], &lastSwitchesSeen[switchPort][1], sizeof(lastSwitchesSeen[switchPort][1])*RESET_ARRAY_SIZE - 1);
				lastSwitchesSeen[switchPort][RESET_ARRAY_SIZE - 1] =now/1000;

				DEBUG(DEBUG_INFO, Serial.printf("lastSwitchesSeen "));

				for (int each = 0; each < NUM_SOCKETS; each++)
				{
					DEBUG(DEBUG_INFO, Serial.printf("%lu ", lastSwitchesSeen[switchPort][each]));
				}

				DEBUG(DEBUG_INFO, Serial.printf("\n\r"));
#endif

				// having CAUSED the interrupt, reflect its STATE in the DoRelay call
				DoSwitch(switchPort, (causeAndState & (1 << switchPort)) ? true : false, false);



#ifdef _RESET_VIA_QUICK_SWITCH

				// remember the last 6 - i'm assuming we won't wrap
				// only reset if we're currently STA
				if (currentMode == wifiMode::modeSTA)
				{
					if (lastSwitchesSeen[switchPort][RESET_ARRAY_SIZE - 1] - lastSwitchesSeen[switchPort][0] < (Details.resetWindowms))
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
			last_micros[switchPort] = now;
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

#ifdef _USE_JSON_NOT_EEPROM

void WriteJSON(bool apset, const char *ssid, const char *pwd, long bounce, long reset, unsigned *map)
{
	DEBUG(DEBUG_INFO, Serial.println("WriteJSON"));

	Details.debounceThresholdms = bounce;
	Details.resetWindowms = reset;
	Details.wifi.configured = apset;

	Details.wifi.password = pwd?pwd:String();
	Details.wifi.ssid = ssid?ssid:String();

	// first - see if the file is there
	fs::File json = SPIFFS.open("/config.json", "w");

	if (!json)
	{
		DEBUG(DEBUG_ERROR, Serial.println("failed to create json"));
		return;
	}

	StaticJsonBuffer<1024> jsonBuffer;

	JsonObject &root=jsonBuffer.createObject();

	root["debounceThresholdms"]= Details.debounceThresholdms;
	root["resetWindowms"]=Details.resetWindowms;

	JsonObject &wifi = root.createNestedObject("wifi");

	wifi["configured"]=Details.wifi.configured;
	if (Details.wifi.configured)
	{
		wifi["password"]=Details.wifi.password;
		wifi["ssid"]=Details.wifi.ssid;
	}


	AddMapToJSON(root, NUM_SOCKETS, map);


	///////////////////// written here

	String jsonText;
	root.prettyPrintTo(jsonText);

	DEBUG(DEBUG_VERBOSE, Serial.printf("JSON : %s\n\r",jsonText.c_str()));

	json.write((byte*)jsonText.c_str(),jsonText.length());
	
	json.close();
}

void ReadJSON(unsigned *map)
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
		WriteJSON(false, NULL, NULL, BOUNCE_TIMEOUT_DEFAULT, QUICK_SWITCH_TIMEOUT_DEFAULT,map);

		return;

	}
	// first - see if the file is there
	fs::File json = SPIFFS.open(_JSON_CONFIG_FILE, "r");

	String jsonString=json.readString();

	json.close();

	DEBUG(DEBUG_IMPORTANT, Serial.printf("JSON: %s\n\r",jsonString.c_str()));

	StaticJsonBuffer<1024> jsonBuffer;

	JsonObject& root = jsonBuffer.parseObject(jsonString);
	
	if (!root.success())
	{
		DEBUG(DEBUG_ERROR, Serial.println("JSON parse failed"));
		WriteJSON(false, NULL, NULL, Details.debounceThresholdms, Details.resetWindowms,&switchToRelayMap[0]);
	}

	Details.debounceThresholdms=root["debounceThresholdms"];
	Details.resetWindowms=root["resetWindowms"];

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
			*map = switchMap[each];
			map++;
		}
	}


}


#else

void WriteEeprom(bool apset,const char *ssid,const char *pwd, long bounce, long reset)
{
	DEBUG(DEBUG_VERBOSE, Serial.println("Writing EEPROM"));
	DEBUG(DEBUG_VERBOSE, Serial.println(bounce));
	DEBUG(DEBUG_VERBOSE, Serial.println(reset));

	Details.wifi.configured = apset;
	if(apset)
	{
		strcpy(Details.wifi.ssid, ssid);
		strcpy(Details.wifi.password, pwd);
	}
	else
	{
		memset(Details.wifi.ssid, 0, sizeof(Details.wifi.ssid));
		memset(Details.wifi.password, 0, sizeof(Details.wifi.password));
	}

	Details.debounceThresholdms = bounce;
	Details.resetWindowms = reset;

	EEPROM.put(0, Details);
	EEPROM.commit();

}

#endif


void BeginWebServer()
{
	server.begin();
	DEBUG(DEBUG_INFO, Serial.println("HTTP server started"));
}

// disjoin and rejoin, optionally force a STA attempt
void ConnectWifi(wifiMode intent)
{
	busyDoingSomethingIgnoreSwitch = true;

	WiFi.persistent(false);

	DEBUG(DEBUG_INFO, Serial.println("ConnectWifi"));

	// turn off wifi
	switch (currentMode)
	{
	case wifiMode::modeAP:
		WiFi.softAPdisconnect();
		break;
	case wifiMode::modeSTA:
		WiFi.setAutoReconnect(false);
		WiFi.disconnect();
		break;
	case wifiMode::modeUnknown:
		break;
	}

	delay(1000);

	DEBUG(DEBUG_WARN, Serial.println("wifi disconnected"));


	if (intent == wifiMode::modeSTA)
	{
		currentMode = wifiMode::modeSTA;

		// turn bonjour off??
		DEBUG(DEBUG_VERBOSE, Serial.print("Attempting connect to "));
		DEBUG(DEBUG_VERBOSE, Serial.println(Details.wifi.ssid));

		WiFi.mode(WIFI_STA);



#ifdef _USE_JSON_NOT_EEPROM
		WiFi.begin(Details.wifi.ssid.c_str(), Details.wifi.password.c_str());
#else
		WiFi.begin(Details.wifi.ssid, Details.wifi.password);
#endif

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

#ifdef _RESET_VIA_QUICK_SWITCH
void ResetMe()
{
	DEBUG(DEBUG_WARN, Serial.println("Resetting"));

	resetWIFI = false;
	// clear the credentials
#ifdef _USE_JSON_NOT_EEPROM
	WriteJSON(false, NULL, NULL, Details.debounceThresholdms, Details.resetWindowms, &switchToRelayMap[0]);
#else
	WriteEeprom(false, NULL, NULL, Details.debounceThresholdms, Details.resetWindowms);
#endif
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



#ifdef _RESET_VIA_QUICK_SWITCH
	// clean up the switch times
	memset(&lastSwitchesSeen, 0, sizeof(lastSwitchesSeen));
#endif

	// reset the bounce thresh-holds
#ifndef _IGNORE_BOUNCE_LOGIC
	for (int eachSwitch = 0; eachSwitch < NUM_SOCKETS; eachSwitch++)
	{
		last_micros[eachSwitch] = 0;
	}
#endif

	// mandatory "let it settle" delay
	delay(1000);
	Serial.begin(115200);

#ifdef _USE_JSON_NOT_EEPROM

	SPIFFS.begin();

	ReadJSON(&switchToRelayMap[0]);

#else
	// start eeprom library
	EEPROM.begin(512);

#ifdef _INITIALISE_EEPROM
	WriteEeprom(false, NULL, NULL, BOUNCE_TIMEOUT_DEFAULT, QUICK_SWITCH_TIMEOUT_DEFAULT);
#endif

	// try reading the eeprom
	// the marker is to spot virgin EEPROM (can i vape it on a build?)
	EEPROM.get(0, Details);

#endif

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
		//Serial.println(c.ssid);

	});

	onIPgranted=WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
		IPAddress copy = event.ip;
		DEBUG(DEBUG_IMPORTANT, Serial.printf("EVENT IP granted %s\n\r", copy.toString().c_str()));

// my router doesn't understand *LAN* routes
#define _HARD_CODED_IP_ADDRESS
#ifdef _HARD_CODED_IP_ADDRESS
		if(WiFi.config(IPAddress(192, 168, 42, 17), IPAddress(192, 168, 42, 250), IPAddress(255, 255, 255, 0)))
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
		//Serial.println(c.ssid);

	});


	// initialise the MCP
	mcp.Initialise();


	// preparing GPIOs
	pinMode(inputSwitchPin, INPUT_PULLUP);
	// the MCP interrupt is configured to fire on change, and goes LOW when fired
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



	server.on("/button", []() {

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

	server.on("/", []() {

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();

	});


	// posted config
	server.on("/json/config", HTTP_POST, []() {

		DEBUG(DEBUG_INFO, Serial.println("json config posted"));
		DEBUG(DEBUG_INFO, Serial.println(server.arg("plain")));

		StaticJsonBuffer<1024> jsonBuffer;
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(server.arg("plain"));

		String ssid = root["ssid"];
		String pwd = root["pwd"];
		long bounce = root["bouncems"];
		long reset = root["resetms"];

		// extract the details
#ifdef _USE_JSON_NOT_EEPROM
		WriteJSON(true, ssid.c_str(), pwd.c_str(), bounce, reset, &switchToRelayMap[0]);
#else
		WriteEeprom(true, ssid.c_str(), pwd.c_str(), bounce, reset);
#endif
		delay(_WEB_TAR_PIT_DELAY);

		// force attempt
		ConnectWifi(wifiMode::modeSTA);
		delay(1000);

		server.send(200, "text/html", "<html></html>");


		});

	// GET
	server.on("/json/map", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json map called"));

		StaticJsonBuffer<1024> jsonBuffer;

		JsonObject &root = jsonBuffer.createObject();

		AddMapToJSON(root, NUM_SOCKETS, &switchToRelayMap[0]);

		String jsonText;
		root.prettyPrintTo(jsonText);

		server.send(200, "application/json", jsonText);
	});


	server.on("/json/state", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json state called"));

		StaticJsonBuffer<1024> jsonBuffer;

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = esphostname;
		root["switchCount"] = NUM_SOCKETS;

		JsonArray &switchState = root.createNestedArray("switchState");
		for (unsigned each = 0; each < NUM_SOCKETS; each++)
		{
			JsonObject &switchRelay = jsonBuffer.createObject();
			switchRelay["switch"] = each;
			switchRelay["relay"] = MapSwitchToRelay(each);
			switchRelay["state"] = mcp.readSwitch(each) ? 1 : 0;

			switchState.add(switchRelay);
		}

		String jsonText;
		root.prettyPrintTo(jsonText);

		server.send(200, "application/json", jsonText);
	});

	server.on("/json/config", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json config called"));

		StaticJsonBuffer<1024> jsonBuffer;

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = esphostname;
		root["bouncems"] = Details.debounceThresholdms;
		root["resetms"] = Details.resetWindowms;

		String jsonText;
		root.prettyPrintTo(jsonText);

		server.send(200, "text/json", jsonText);
	});


	server.on("/json/file", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_IMPORTANT, Serial.println("json file called"));

		if (SPIFFS.exists(_JSON_CONFIG_FILE))
		{
			fs::File json = SPIFFS.open(_JSON_CONFIG_FILE, "r");

			String jsonString = json.readString();

			server.send(200, "application/json", jsonString);

			json.close();
		}
	});

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

void AddMapToJSON(JsonObject &root, unsigned numSockets, unsigned *map)
{
	root["switchCount"] = numSockets;

	JsonArray &switchMap = root.createNestedArray("switchMap");
	for (unsigned each = 0; each < numSockets; each++)
	{
		switchMap.add(*map);
		map++;
	}

}


void loop(void) 
{
#ifdef _RESET_VIA_QUICK_SWITCH
	if (resetWIFI)
		ResetMe();
#endif

	
	server.handleClient();
}