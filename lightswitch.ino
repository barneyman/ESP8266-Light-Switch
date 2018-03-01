
#include <WiFiClient.h>


#define _SONOFF_BASIC
//#define _WEMOS_RELAY_SHIELD


#if defined(_WEMOS_RELAY_SHIELD) && defined(_SONOFF_BASIC)
#error "Cannot have BOTH types of modules defined!"
#endif

#if defined(_WEMOS_RELAY_SHIELD) || defined(_SONOFF_BASIC)

#define _SIMPLE_ONE_SWITCH
#define NUM_SOCKETS	1

#else
#define NUM_SOCKETS	6

#endif

#if defined (_WEMOS_RELAY_SHIELD)

#define GPIO_RELAY	D1
#define GPIO_LED		BUILTIN_LED
// whatever you want it to be!
#define GPIO_SWITCH D2

#elif defined (_SONOFF_BASIC)

// IMPORTANT
// https://github.com/arendst/Sonoff-Tasmota/wiki/Arduino-IDE
// Generic ESP8266 (beware, some 8255s out there!)
// Flashmode DOUT
// 1M 128k SPIFFS

// a number of exceptions in 2.4.0 & LWIP2 - currently only works reliably with 2.3.0 and LWIP1.4

#define GPIO_RELAY		12	// GPIO12
#define GPIO_LED			13
#define GPIO_SWITCH		0	// GPIO0 is existing button, GPIO14/D5 for the one on the header
#define GPIO_SWITCH2		14
	
#endif






#include "mcp23017.h"



#include <ArduinoJson.h>
#include <FS.h>

#include <vector>
#include <algorithm> 

#ifndef _SONOFF_BASIC
// sonoff isn't big enough for a decent SPIFFs
#include <ESP8266httpUpdate.h>
#endif

#ifdef _SONOFF_BASIC
#define _VERSION_ROOT	"lightS_"
#elif defined (_WEMOS_RELAY_SHIELD)
#define _VERSION_ROOT	"lightW_"
#else
#define _VERSION_ROOT	"light6_"
#endif


#define _MYVERSION			_VERSION_ROOT "1.1"

//#define _ERASE_JSON_CONFIG
#define _JSON_CONFIG_FILE "/config.json"

#define JSON_STATIC_BUFSIZE	2048
StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;

#ifdef _DEBUG
#define _TEST_WFI_STATE	
#endif





// first board that worked, although the pins were swapped around on the output
#define _BOARD_VER_1_1


// my libs
#include <myWifi.h>

#ifdef _SONOFF_BASIC
myWifiClass wifiInstance("sonoff_");
#elif defined(_WEMOS_RELAY_SHIELD)
myWifiClass wifiInstance("wemos_");
#else
myWifiClass wifiInstance("6switch_");
#endif







// millis timeouts
#define QUICK_SWITCH_TIMEOUT_DEFAULT	6000
#define BOUNCE_TIMEOUT_DEFAULT			100

#define _RESET_VIA_QUICK_SWITCH


#ifdef _RESET_VIA_QUICK_SWITCH
// how many transitions we have to see (on -> off and v-v) within Details.resetWindowms before we assume a resey has been asked for
#define RESET_ARRAY_SIZE 12
bool resetWIFI = false;
#endif





enum	typeOfSwitch { stNone, stMomentary, stToggle };
enum	switchState { swUnknown, swOn, swOff };

struct 
{
	bool configDirty;

	// wifi deets
	myWifiClass::wifiDetails wifi;


	// how long to wait for the light switch to settle
	unsigned long debounceThresholdmsToggle, debounceThresholdmsMomentary;

	// 6 switches in this time force an AP reset
	unsigned long resetWindowms;

	// the switches are named
	struct {

		// persisted
		String name;
		unsigned relay;
		enum switchState lastState;

		// not persisted
		enum switchState preferredDefault;
		enum  typeOfSwitch switchType;
#ifdef GPIO_SWITCH2
		enum  typeOfSwitch altSwitchType;
#endif
		// when we saw this switch change state - used to debounce the switch
		unsigned long last_seen_bounce;

#ifdef _RESET_VIA_QUICK_SWITCH
		unsigned long lastSwitchesSeen[RESET_ARRAY_SIZE];
#endif

		// how many times we've see this PHYSICAL switch .. er ... switch
		unsigned long switchCount;

	} switches[NUM_SOCKETS];

} Details = {

	false, 

	{
		"","",false, true
	},
	BOUNCE_TIMEOUT_DEFAULT, BOUNCE_TIMEOUT_DEFAULT*3,

	QUICK_SWITCH_TIMEOUT_DEFAULT,

#ifdef 	_SONOFF_BASIC
	{
#ifdef GPIO_SWITCH2
		{ "Sonoff",0,swUnknown, swOff, stMomentary,stToggle, 0, 0 }
#else
		{ "Sonoff",0, swUnknown, swOff, stMomentary, 0,0 }
#endif
	}

#elif defined _SIMPLE_ONE_SWITCH

	{
		{ "Device A",0, swUnknown,swOff, stMomentary, 0,0 }
	}
#else

#ifdef _BOARD_VER_1_1
	{
		{ "Device A", 0, swUnknown,swOff, stToggle, 0,0 },
		{ "Device B", 5, swUnknown,swOff, stToggle, 0,0 },
		{ "Device C", 4, swUnknown,swOff, stToggle, 0,0 },
		{ "Device D", 3, swUnknown,swOff, stToggle, 0,0 },
		{ "Device E", 2, swUnknown,swOff, stToggle, 0,0 },
		{ "Device F", 1, swUnknown,swOff, stToggle, 0,0 }
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

#ifdef GPIO_SWITCH2
// ICACHE_RAM_ATTR  makes it ISR safe
void ICACHE_RAM_ATTR OnSwitchISR2()
{
	// if we're up to our neck in something else (normally WIFI negotiation) ignore this
	if (wifiInstance.busyDoingSomethingIgnoreSwitch)
	{
		DEBUG(DEBUG_INFO, Serial.println("	OnSwitchISR_2 redundant"));
		return;
	}

	DEBUG(DEBUG_VERBOSE, Serial.println("	OnSwitchISR_2 in"));


	// ask what changed, clear interrupt
	int causeAndState =
	(Details.switches[0].altSwitchType == stMomentary ?
		// fake the cause and reflect INVERSE state of relay - because MOMENTARY
		(1 << 8) | (digitalRead(GPIO_RELAY) == HIGH ? 0 : 1) :
		// handle the toggle as a toggle
		(1 << 8) | (digitalRead(GPIO_RELAY) == HIGH ? 0 : 1));

	HandleCauseAndState(causeAndState);

	DEBUG(DEBUG_VERBOSE, Serial.println("OnSwitchISR2 out"));
}
#endif

void ICACHE_RAM_ATTR OnSwitchISR()
{
	// if we're up to our neck in something else (normally WIFI negotiation) ignore this
	if (wifiInstance.busyDoingSomethingIgnoreSwitch)
	{
		DEBUG(DEBUG_INFO, Serial.println("	OnSwitchISR redundant"));

#ifndef _SIMPLE_ONE_SWITCH
		// ask what changed, clear interrupt, so we don't leave the INTerrupt hanging
		mcp.InterruptCauseAndCurrentState(true);
#endif
		return;
	}

	DEBUG(DEBUG_VERBOSE, Serial.println("	OnSwitchISR in"));


	// ask what changed, clear interrupt
	int causeAndState =
#ifdef _SIMPLE_ONE_SWITCH
		(Details.switches[0].switchType == stMomentary ?
		// fake the cause and reflect INVERSE state of relay - because MOMENTARY
		(1 << 8) | (digitalRead(GPIO_RELAY) == HIGH ? 0 : 1) :
		// fake the cause and rstate of switch - because TOGGLE
		(1 << 8) | (digitalRead(GPIO_SWITCH) == HIGH ? 1 : 0));

#else
		mcp.InterruptCauseAndCurrentState(false);
#endif

	HandleCauseAndState(causeAndState);

	DEBUG(DEBUG_VERBOSE, Serial.println("OnSwitchISR out"));

}

void ICACHE_RAM_ATTR HandleCauseAndState(int causeAndState)
{
	for (unsigned switchPort = 0; switchPort < NUM_SOCKETS; switchPort++)
	{
		DEBUG(DEBUG_VERBOSE, Serial.printf("Checking port %d [%04x]\r\n", switchPort, causeAndState));

		// +8 to get HIBYTE to see if this port CAUSED the interrupt
		if (causeAndState & (1 << (switchPort + 8)))
		{

#if defined(_RESET_VIA_QUICK_SWITCH) 
			unsigned long now = micros(), interval = 0;
#endif

			// gate against messy tactile/physical switches
			interval = now - Details.switches[switchPort].last_seen_bounce;

			DEBUG(DEBUG_VERBOSE, Serial.printf("%lu ms ", interval / 1000UL));

			// if it's been longer than the bounce threshold since we saw this button, honour it
			unsigned long bounceToHonour = Details.switches[switchPort].switchType == stMomentary ? Details.debounceThresholdmsMomentary : Details.debounceThresholdmsToggle;
			if (interval >= (unsigned long)(bounceToHonour * 1000))
			{
#ifdef _RESET_VIA_QUICK_SWITCH
				// move the last seens along
				for (unsigned mover = 0; mover < RESET_ARRAY_SIZE - 1; mover++)
					Details.switches[switchPort].lastSwitchesSeen[mover] = Details.switches[switchPort].lastSwitchesSeen[mover + 1];

				Details.switches[switchPort].lastSwitchesSeen[RESET_ARRAY_SIZE - 1] = now / 1000;

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
				if (wifiInstance.currentMode == myWifiClass::wifiMode::modeSTA)
				{
					if (Details.switches[switchPort].lastSwitchesSeen[RESET_ARRAY_SIZE - 1] - Details.switches[switchPort].lastSwitchesSeen[0] < (Details.resetWindowms))
					{
						DEBUG(DEBUG_IMPORTANT, Serial.println("RESETTING WIFI!\n\r"));
						resetWIFI = true;
					}
				}
#endif
			}
			else
			{
				DEBUG(DEBUG_INFO, Serial.printf("bounce ignored\n\r"));
			}
			Details.switches[switchPort].last_seen_bounce = now;
		}
	}
}

// honour current switch state
void RevertAllSwitch()
{

#ifdef _SIMPLE_ONE_SWITCH

	// read the current switch state, and reflect that in the relay state
	enum switchState requestState = swUnknown;
	if (Details.switches[0].lastState == swUnknown)
	{
		// try to find it
		if (Details.switches[0].switchType == stToggle)
		{
			// found a toggle, believe it
			requestState = (digitalRead(GPIO_SWITCH) == HIGH)?swOn:swOff;
		}
#ifdef GPIO_SWITCH2
		else if (Details.switches[0].altSwitchType == stToggle)
		{
			// found a toggle, believe it
			requestState = (digitalRead(GPIO_SWITCH2) == HIGH) ? swOn : swOff;
		}
#endif
		else
		{
			requestState = Details.switches[0].preferredDefault;
		}
	}
	else
	{
		requestState = Details.switches[0].lastState;
	}
	DoSwitch(0, requestState == swOn ? true : false, false);


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
	DoSwitch(0, state, force);
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



#ifdef _SONOFF_BASIC
	// LED is inverted on the sonoff
	digitalWrite(GPIO_LED, on ? LOW : HIGH);
#else
	digitalWrite(GPIO_LED, on ? HIGH : LOW);
#endif

#else
	DoRelay(MapSwitchToRelay(portNumber), on);
	if (forceSwitchToReflect)
	{
		mcp.SetSwitch(portNumber, on);
	}
#endif

	enum switchState newState = on ? swOn : swOff;;
	// reflect in state
	if (newState != Details.switches[portNumber].lastState)
	{
		Details.switches[portNumber].lastState = newState;
		Details.configDirty = true;
	}
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

	root["debounceThresholdmsMomentary"] = Details.debounceThresholdmsMomentary;	
	root["debounceThresholdmsToggle"] = Details.debounceThresholdmsToggle;

	root["resetWindowms"] = Details.resetWindowms;

	wifiInstance.WriteDetailsToJSON(root, Details.wifi);

	AddMapToJSON(root, NUM_SOCKETS);

	DEBUG(DEBUG_VERBOSE, Serial.printf("jsonBuffer.size used : %d\n\r", jsonBuffer.size()));

	///////////////////// written here

	String jsonText;
	root.prettyPrintTo(jsonText);

	DEBUG(DEBUG_VERBOSE, Serial.printf("JSON : -- %s --\n\r", jsonText.c_str()));

	json.write((byte*)jsonText.c_str(), jsonText.length());

	DEBUG(DEBUG_VERBOSE, Serial.println("JSON : written"));

	json.close();

	DEBUG(DEBUG_VERBOSE, Serial.println("JSON : closed"));

	Details.configDirty = false;
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

	Details.configDirty = false;

	Details.debounceThresholdmsMomentary= root["debounceThresholdmsMomentary"];
	Details.debounceThresholdmsToggle=root["debounceThresholdmsToggle"];
	Details.resetWindowms= root["resetWindowms"];

	wifiInstance.ReadDetailsFromJSON(root, Details.wifi);

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
				Details.switches[each].lastState = (enum switchState)(int)theSwitch["lastState"];
			}
			else
			{
				DEBUG(DEBUG_IMPORTANT, Serial.printf("switchMap switch %d not found\n\r",each));
			}
		}
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
	wifiInstance.ConnectWifi(myWifiClass::wifiMode::modeAP,Details.wifi);

}

#endif



void setup(void) 
{

	// reset the bounce thresh-holds
	for (int eachSwitch = 0; eachSwitch < NUM_SOCKETS; eachSwitch++)
	{
#ifdef _RESET_VIA_QUICK_SWITCH
		// clean up the switch times
		memset(&Details.switches[eachSwitch].lastSwitchesSeen, 0, sizeof(Details.switches[eachSwitch].lastSwitchesSeen));
#endif

		Details.switches[eachSwitch].switchCount = 0;
		Details.switches[eachSwitch].last_seen_bounce = 0;
	}

	// mandatory "let it settle" delay
	Serial.begin(115200);

	DEBUG(DEBUG_INFO, Serial.println("setup() running"));


	SPIFFS.begin();

	ReadJSONconfig();

	DEBUG(DEBUG_VERBOSE, Serial.println("starting"));
	DEBUG(DEBUG_IMPORTANT, Serial.println(wifiInstance.m_hostName.c_str()));
	DEBUG(DEBUG_VERBOSE, Serial.printf("bounceMomentary %lu\n\r",Details.debounceThresholdmsMomentary));
	DEBUG(DEBUG_VERBOSE, Serial.printf("bounceToggle %lu\n\r", Details.debounceThresholdmsToggle));
	DEBUG(DEBUG_VERBOSE, Serial.printf("reset %lu\n\r",Details.resetWindowms));

	enum myWifiClass::wifiMode intent = myWifiClass::wifiMode::modeUnknown;

	if (Details.wifi.configured)
	{
		DEBUG(DEBUG_INFO, Serial.println("credentials found"));
		DEBUG(DEBUG_VERBOSE, Serial.println(Details.wifi.ssid));
		DEBUG(DEBUG_VERBOSE, Serial.println(Details.wifi.password));
		intent = myWifiClass::wifiMode::modeSTA;
	}
	else
	{
		DEBUG(DEBUG_WARN, Serial.println("WiFi not configured"));
		intent = myWifiClass::wifiMode::modeAP;

	}



#ifdef _SIMPLE_ONE_SWITCH

	// set the relay pin to output
	pinMode(GPIO_RELAY, OUTPUT);
	pinMode(GPIO_LED, OUTPUT);

	// and the switch pin to input - pullup
	pinMode(GPIO_SWITCH, INPUT_PULLUP);
	// for momentary switches we just look for low
	attachInterrupt(GPIO_SWITCH, OnSwitchISR, Details.switches[0].switchType==stMomentary?ONLOW:CHANGE);

#ifdef GPIO_SWITCH2
	// and the switch pin to input - pullup
	pinMode(GPIO_SWITCH2, INPUT_PULLUP);
	// for toggle switches we just look for change
	attachInterrupt(GPIO_SWITCH2, OnSwitchISR2, Details.switches[0].altSwitchType == stMomentary ? ONLOW : CHANGE);
#endif

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
	wifiInstance.ConnectWifi(intent, Details.wifi);

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

	wifiInstance.server.on("/revert", []() {

		RevertAllSwitch();

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();
	});

	wifiInstance.server.on("/all", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/all"));

		for (uint8_t i = 0; i < wifiInstance.server.args(); i++)
		{
			if (wifiInstance.server.argName(i) == "action")
			{
				DoAllSwitch(wifiInstance.server.arg(i) == "on" ? true : false, true);
			}
		}

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();
	});

#ifndef _SIMPLE_ONE_SWITCH

	wifiInstance.server.on("/json/upgrade", HTTP_POST, []() {

		DEBUG(DEBUG_INFO, Serial.println("json upgrade posted"));
		DEBUG(DEBUG_INFO, Serial.println(wifiInstance.server.arg("plain")));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(wifiInstance.server.arg("plain"));

		String host = root["host"];
		int port = root["port"];
		String url= root["url"];

		delay(_WEB_TAR_PIT_DELAY);

		enum HTTPUpdateResult result = ESPhttpUpdate.update(host, port, url, _MYVERSION);

		switch (result)
		{
		case HTTP_UPDATE_FAILED:
			DEBUG(DEBUG_ERROR, Serial.println("updated FAILED"));
			break;
		case HTTP_UPDATE_NO_UPDATES:
			DEBUG(DEBUG_IMPORTANT, Serial.println("no updates"));
			break;
		case HTTP_UPDATE_OK:
			DEBUG(DEBUG_IMPORTANT, Serial.println("update succeeded"));
			break;
		}


		wifiInstance.server.send(200, "text/html", "<html></html>");

	});


	wifiInstance.server.on("/toggle", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/toggle"));

		delay(_WEB_TAR_PIT_DELAY);

		// must be an arg
		if (!wifiInstance.server.args())
		{
			return;
		}

		if (wifiInstance.server.argName(0) == "relay")
		{
			if (mcp.ToggleRelay(wifiInstance.server.arg(0).toInt()))
			{
				wifiInstance.server.send(200, "text/html", "<html></html>");
			}
		}

	});
#endif

	wifiInstance.server.on("/button", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/button"));

		// these have to be in port/action pairs
		if (wifiInstance.server.args() % 2)
		{
			return;
		}


		for (uint8_t i = 0; i < wifiInstance.server.args(); i += 2)
		{
			int port = -1; bool action = false;
			if (wifiInstance.server.argName(i) == "port" && wifiInstance.server.argName(i + 1) == "action")
			{
				port = wifiInstance.server.arg(i).toInt();
				wifiInstance.server.arg(i + 1).toLowerCase();
				action = wifiInstance.server.arg(i + 1) == "on" ? true : false;
			}
			else if (wifiInstance.server.argName(i) == "action" && wifiInstance.server.argName(i + 1) == "port")
			{
				port = wifiInstance.server.arg(i + 1).toInt();
				wifiInstance.server.arg(i).toLowerCase();
				action = wifiInstance.server.arg(i) == "on" ? true : false;
			}

			if (port != -1)
				DoSwitch(port, action, true);
		}

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();

	});

	wifiInstance.server.on("/resetCounts", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/resetCounts"));

		delay(_WEB_TAR_PIT_DELAY);

		for (int eachSwitch = 0; eachSwitch < NUM_SOCKETS; eachSwitch++)
		{
			Details.switches[eachSwitch].switchCount = 0;
		}

		wifiInstance.server.send(200,"text/html","<html/>");

	});

	wifiInstance.server.on("/reboot", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/reboot"));

		RebootMe();

	});


	wifiInstance.server.on("/resetWIFI", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/resetWIFI"));

		ResetMe();

	});

	wifiInstance.server.on("/stopAP", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/stopAP"));

		if (wifiInstance.currentMode == myWifiClass::wifiMode::modeSTAandAP)
		{
			wifiInstance.ConnectWifi(myWifiClass::wifiMode::modeSTA, Details.wifi);
			wifiInstance.server.send(200, "text/html", "<html/>");
		}
		else
		{
			wifiInstance.server.send(500, "text/html", "<html/>");
		}

	});



	wifiInstance.server.on("/", []() {

		DEBUG(DEBUG_VERBOSE, Serial.println("/"));

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();

	});


	// posted config
	wifiInstance.server.on("/json/config", HTTP_POST, []() {

		DEBUG(DEBUG_INFO, Serial.println("json config posted"));
		DEBUG(DEBUG_INFO, Serial.println(wifiInstance.server.arg("plain")));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(wifiInstance.server.arg("plain"));

		long bounceMomentary = root["bouncemsMomentary"];
		long bounceToggle = root["bouncemsToggle"];

		long reset = root["resetms"];

		// sanity check these values!

		Details.debounceThresholdmsToggle = bounceToggle;
		Details.debounceThresholdmsMomentary = bounceMomentary;

		Details.resetWindowms = reset;

		// extract the details
		WriteJSONconfig();
		delay(_WEB_TAR_PIT_DELAY);

		wifiInstance.server.send(200, "text/html", "<html></html>");

		});


	wifiInstance.server.on("/json/wifi", HTTP_POST, []() {

		DEBUG(DEBUG_INFO, Serial.println("json wifi posted"));
		DEBUG(DEBUG_INFO, Serial.println(wifiInstance.server.arg("plain")));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(wifiInstance.server.arg("plain"));

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
		// if we succeedefd, send back success, then change to STA
		if (wifiInstance.ConnectWifi(myWifiClass::wifiMode::modeSTAspeculative, Details.wifi) == myWifiClass::wifiMode::modeSTAandAP)
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
		//wifiInstance.server.send(200, "text/html", "<html></html>");


	});

	// GET

	wifiInstance.server.on("/json/state", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json state called"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = wifiInstance.m_hostName.c_str();
		root["switchCount"] = NUM_SOCKETS;

		JsonArray &switchState = root.createNestedArray("switchState");
		for (unsigned each = 0; each < NUM_SOCKETS; each++)
		{
			JsonObject &switchRelay = switchState.createNestedObject();
			switchRelay["switch"] = each;
			switchRelay["type"] = Details.switches[each].switchType==stMomentary?"Momentary":"Toggle";
			switchRelay["name"] = Details.switches[each].name;
			switchRelay["relay"] = Details.switches[each].relay;
			switchRelay["state"] = 
#ifdef _SIMPLE_ONE_SWITCH
				// reflect the relay, not the switch
				Details.switches[0].lastState== swOn?1:0;
#else
				mcp.readSwitch(each) ? 1 : 0;
#endif
			switchRelay["stateChanges"] = Details.switches[each].switchCount;
		}

		String jsonText;
		root.prettyPrintTo(jsonText);

		DEBUG(DEBUG_VERBOSE, Serial.println(jsonText));

		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
	});

	wifiInstance.server.on("/json/maxSwitchCount", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json maxSwitchCount called"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = wifiInstance.m_hostName.c_str();
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

		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
	});



	wifiInstance.server.on("/json/config", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json config called"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = wifiInstance.m_hostName.c_str();
		root["version"] = _MYVERSION;
		root["bouncemsMomentary"] = Details.debounceThresholdmsMomentary;
		root["bouncemsToggle"] = Details.debounceThresholdmsToggle;
		root["resetms"] = Details.resetWindowms;

		String jsonText;
		root.prettyPrintTo(jsonText);

		DEBUG(DEBUG_VERBOSE, Serial.println(jsonText));

		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
	});

	wifiInstance.server.on("/json/wificonfig", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json wificonfig called"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = wifiInstance.m_hostName.c_str();
		root["ssid"] = wifiInstance.SSID();
		root["ip"] = wifiInstance.localIP().toString();

		String jsonText;
		root.prettyPrintTo(jsonText);

		DEBUG(DEBUG_VERBOSE, Serial.println(jsonText));

		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
	});


	wifiInstance.server.on("/json/wifi", HTTP_GET, []() {
		// give them back the port / switch map

		DEBUG(DEBUG_INFO, Serial.println("json wifi called"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();
		root["name"] = wifiInstance.m_hostName.c_str();

		// let's get all wifis we can see
		std::vector<std::pair<String, int>> allWifis;
		int found=wifiInstance.ScanNetworks(allWifis);

		JsonArray &wifis = root.createNestedArray("wifi");

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
		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "text/json", jsonText);
	});

	// serve up everthing in SPIFFS
	SPIFFS.openDir("/");

	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {
		String file = dir.fileName();

		// cache it for an hour
		wifiInstance.server.serveStatic(file.c_str(), SPIFFS, file.c_str(),"Cache-Control: public, max-age=3600");

		DEBUG(DEBUG_VERBOSE, Serial.printf("Serving %s\n\r", file.c_str()));

	}

	DEBUG(DEBUG_VERBOSE, Serial.println("InstallWebServerHandlers OUT"));

}

void SendServerPage()
{
	// given the current state of the device, send the appropriate page back
	File f;
	switch (wifiInstance.currentMode)
	{
	case myWifiClass::wifiMode::modeSTAandAP:
		f = SPIFFS.open("/STAAPmode.htm", "r");
		break;
	case myWifiClass::wifiMode::modeAP:
		f = SPIFFS.open("/APmode.htm", "r");
		break;
	case myWifiClass::wifiMode::modeSTA:
		f = SPIFFS.open("/STAmode.htm", "r");
		break;
	case myWifiClass::wifiMode::modeUnknown:
	default:
		f = SPIFFS.open("/Error.htm", "r");
		break;

	}

	wifiInstance.server.streamFile(f, "text/html");
	f.close();

}


void AddMapToJSON(JsonObject &root, unsigned numSockets)
{

	DEBUG(DEBUG_VERBOSE, Serial.printf("AddMapToJSON %d\n\r", numSockets));

	root["switchCount"] = numSockets;

	JsonArray &switchMap = root.createNestedArray("switchMap");

	for (unsigned each = 0; each < numSockets; each++)
	{
		JsonObject &theSwitch = switchMap.createNestedObject();
		theSwitch["switch"] = each;
		theSwitch["name"] = Details.switches[each].name.c_str();
		theSwitch["relay"] = Details.switches[each].relay;
		theSwitch["lastState"] = (int)Details.switches[each].lastState;

	}

}



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

	if (Details.configDirty)
		WriteJSONconfig();

	wifiInstance.server.handleClient();

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