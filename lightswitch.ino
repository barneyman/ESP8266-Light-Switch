#include <OneWire.h>

#include <WiFiClient.h>
#include <debugLogger.h>

// define one of the following ... 
// nothing defined == 6 switch nodemcu
#define _SONOFF_BASIC					// the basic normal sonoff
//#define _SONOFF_BASIC_EXTRA_SWITCH		// one with the extra GPIO pin wired as a switch
//#define _AT_RGBSTRIP					// a strip of RGBs


// turn these OFF for the 6 switch one
// the sonoff units
// normal
// #define _SONOFF_BASIC
// the one in LS with an extra switch wire
// #define _SONOFF_BASIC_EXTRA_SWITCH
// the wemso D1 relay shield




#if !defined(_SONOFF_BASIC) && !defined(_SONOFF_BASIC_EXTRA_SWITCH) && !defined(_AT_RGBSTRIP)
#error "Something MUST be defined!"
#endif

#if defined(_SONOFF_BASIC) || defined(_SONOFF_BASIC_EXTRA_SWITCH)

// without this, we use a MUXer
#define _PHYSICAL_SWITCH_EXISTS
#define NUM_SOCKETS	1

// 1mb 128k spiffs gives you ~ 500k for bin file
#define _OTA_AVAILABLE

#elif defined(_AT_RGBSTRIP)

#define _OTA_AVAILABLE
#define NUM_SOCKETS	1



#endif

#if defined(_SONOFF_BASIC) || defined(_SONOFF_BASIC_EXTRA_SWITCH)


// IMPORTANT - for programming SONOFF Basics
// https://github.com/arendst/Sonoff-Tasmota/wiki/Arduino-IDE
// Generic ESP8266 (beware, some 8255s out there!)
// Flashmode DOUT
// FlashSize 1M 128k SPIFFS
// Upload Sketch AND Publish Server Files
// Connect the internal header up to an FTDI
// With it off, hold the button down and power it up, keep the button down for a second or two

// a number of exceptions in 2.4.0 & LWIP2 - currently only works reliably with 2.3.0 and LWIP1.4

// do not exceed 1Mb of spiffs in wemos (and maybe others) - you get assertions in DataSource.h


#define GPIO_RELAY		12	// GPIO12
#define GPIO_LED		13
#define GPIO_SWITCH		0	// GPIO0 is existing button, GPIO14/D5 for the one on the header

#ifdef _SONOFF_BASIC_EXTRA_SWITCH

#define GPIO_SWITCH2		14

#endif

#elif defined(_AT_RGBSTRIP)

#ifdef _DEBUG
#define _NUM_LEDS	15
#else
#define _NUM_LEDS	135
#endif
	
#define GPIO_LED		LED_BUILTIN
// whatever you want it to be!
//#define GPIO_SWITCH		D2


#endif





#include <ArduinoJson.h>
#include <FS.h>

#include <vector>
#include <algorithm> 

#ifdef _OTA_AVAILABLE
// sonoff isn't big enough for a decent SPIFFs
#include <ESP8266httpUpdate.h>
#endif

#ifdef _SONOFF_BASIC
#define _VERSION_ROOT	"lightS_"
#elif defined (_SONOFF_BASIC_EXTRA_SWITCH)
#define _VERSION_ROOT	"lightE_"
#elif defined(_AT_RGBSTRIP)
#define _VERSION_ROOT	"lightRGB_"
#else
#define _VERSION_ROOT	"light6_"
#endif


#define _MYVERSION			_VERSION_ROOT "3.00"

#define _HTML_VER_FILE	"/html.json"
unsigned _MYVERSION_HTML = 0;

// set this to reset the file
//#define _ERASE_JSON_CONFIG
// has a leading underscore so we can spot it, and not serve it statically
#define _JSON_CONFIG_FILE "/_config.json"
// legacy, so we can convert old systems 
#define _LEGACY_JSON_CONFIG_FILE "/config.json"

#define JSON_STATIC_BUFSIZE	2048
StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;

#ifdef _DEBUG
#define _TEST_WFI_STATE	
#include <GDBStub.h>
#endif

// forward
void ICACHE_RAM_ATTR HandleCauseAndState(int causeAndState);

// first board that worked, although the pins were swapped around on the output
#define _BOARD_VER_1_1


// my libs
#include <myWifi.h>

//#define _USE_SYSLOG
// for syslog
#ifdef _USE_SYSLOG
syslogDebug dblog(debug::dbWarning, "192.168.51.1", 514, "temp", "lights");
#else
// try to avoid verbose - it causes heisenbugs because all HTTP interactions are artificially slowed down
SerialDebug dblog(debug::dbInfo);
#endif

//#define _OLD_WAY
#ifdef _OLD_WAY


#ifdef _SENSOR_TYPE
#define mdsnNAME	"bjfSensors"
#else
#define mdsnNAME	"bjfLights"
#endif

#if defined(_SONOFF_BASIC) || defined(_SONOFF_BASIC_EXTRA_SWITCH)
myWifiClass wifiInstance("sonoff_", &dblog, mdsnNAME);
#elif defined(_AT_RGBSTRIP)
SerialDebug dblog(debug::dbVerbose);
myWifiClass wifiInstance("rgb_", &dblog, mdsnNAME);

// pull in the AT handler
#include <wire.h>
#include <atLEDS.h>
#define _AT85_ADDR	0x10
ATleds rgbHandler(_AT85_ADDR,&dblog);

#endif

#else

#define mdsnNAME	"barneyman"
myWifiClass wifiInstance("esp_", &dblog, mdsnNAME);

#endif


// millis timeouts
#define QUICK_SWITCH_TIMEOUT_DEFAULT	6000
#define BOUNCE_TIMEOUT_DEFAULT			100


#define _ALLOW_WIFI_RESET_OVER_WIFI
#define _ALLOW_WIFI_RESET_VIA_QUICKSWITCH
#define _ALLOW_WIFI_RESET_AFTER_APIJOIN_TIMOUT (2*60*1000)

#ifdef _ALLOW_WIFI_RESET_AFTER_APIJOIN_TIMOUT
unsigned long runtimeWhenLastJoined=0;
#endif

#define _ALLOW_WIFI_RESET
#define _ALLOW_WIFI_RESET_OVER_WIFI

#ifdef _ALLOW_WIFI_RESET
bool resetWIFI = false;
#endif



#ifdef _OLD_WAY



#ifdef _ALLOW_WIFI_RESET_VIA_QUICKSWITCH
// how many transitions we have to see (on -> off and v-v) within Details.resetWindowms before we assume a resey has been asked for
#define RESET_ARRAY_SIZE 12
#endif




// none, is nothing (sort of logic hole)
// Momentary push switch
// Toggle switch
// Virtual means nothing, just code reflected

struct 
{
	// does this need saving
	bool configDirty;

	// ignore ISRs
	bool ignoreISRrequests;

#ifdef _AT_RGBSTRIP

	unsigned rgbLedCount;

#endif

	// wifi deets
	myWifiClass::wifiDetails wifi;


	// how long to wait for the light switch to settle
	unsigned long debounceThresholdmsToggle, debounceThresholdmsMomentary;

	// 6 switches in this time force an AP reset
	unsigned long resetWindowms;

	// persisted - *USER* supplied
	String friendlyName;

#ifdef NUM_SOCKETS
	// the switches are named
	struct {

		// persisted - RO
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

		// how many times we've see this PHYSICAL switch .. er ... switch
		unsigned long switchCount;

#ifdef _AT_RGBSTRIP
		unsigned long lastRGB;
#endif


// this must be last in the array (it gets cleaned specifically)
#ifdef _ALLOW_WIFI_RESET_VIA_QUICKSWITCH
		unsigned long lastSwitchesSeen[RESET_ARRAY_SIZE];
#endif




	} switches[NUM_SOCKETS];
#endif

} Details = {

	false, false,

#ifdef _AT_RGBSTRIP

	_NUM_LEDS,

#endif

	{
		"","",false, true
	},

	BOUNCE_TIMEOUT_DEFAULT, BOUNCE_TIMEOUT_DEFAULT*3,

	QUICK_SWITCH_TIMEOUT_DEFAULT,

#ifdef _AT_RGBSTRIP
	"Undefined RGB",
#elif defined( _SONOFF_BASIC )
	"Undefined SonoffS",
#elif defined( _SONOFF_BASIC_EXTRA_SWITCH )
	"Undefined SonoffE",
#else
	"Undefined",
#endif

#if defined(_SONOFF_BASIC) || defined(_SONOFF_BASIC_EXTRA_SWITCH)
	{
#ifdef _SONOFF_BASIC_EXTRA_SWITCH
		{ "Sonoff",0,swUnknown, swOff, stMomentary,stToggle, 0, 0 }
#else
		{ "Sonoff",0, swUnknown, swOff, stMomentary, 0,0 }
#endif
	}
#elif defined( _AT_RGBSTRIP )

{
	{ "RGB",0, swUnknown, swOff, stVirtual, 0,0, 0x7f7f7f }
}

#elif defined( _PHYSICAL_SWITCH_EXISTS )
	{
		{ "Device A",0, swUnknown,swOff, stMomentary, 0,0 }
	}

#endif
};


#else


#include "switchSensor.h"

// main config
struct 
{
	// does this need saving
	bool configDirty;
	// wifi deets
	myWifiClass::wifiDetails wifi;
	// persisted - *USER* supplied
	// TODO - change this to device friendly name
	String friendlyName;

	// sensors
	std::vector<baseSensor*>	sensors;

	// switches
	std::vector<baseSwitch*>	switches;

} Details=
{
	false,	// dirty

	// wifi deets
	{
		"","",false, true
	},

	"Undefined"	// friendly name


};


#endif



// where we store the files we serve, statically
std::vector<std::pair<String,size_t>> servedFiles;
// our peers
std::vector<myWifiClass::mdnsService> services;






// how long we slow the web hits down for (millis)
#define _WEB_TAR_PIT_DELAY 20



#ifndef _PHYSICAL_SWITCH_EXISTS

#ifdef NUM_SOCKETS

unsigned MapSwitchToRelay(unsigned switchNumber)
{
	unsigned relayNumber = switchNumber;

	if (relayNumber > NUM_SOCKETS || relayNumber < 0)
	{
		dblog.printf(debug::dbError, "MapSwitchToRelay called out of bounds %u\n\r", relayNumber);
	}
	else
	{
		relayNumber=Details.switches[switchNumber].relay;
	}

	dblog.printf(debug::dbVerbose, "	MapSwitchToRelay %u -> %u\r\n", switchNumber, relayNumber);

	return relayNumber;

}

#endif

#endif

#ifdef GPIO_SWITCH2
// ICACHE_RAM_ATTR  makes it ISR safe
void ICACHE_RAM_ATTR OnSwitchISR2()
{
	if (Details.ignoreISRrequests)
		return;

	// if we're up to our neck in something else (normally WIFI negotiation) ignore this
	if (wifiInstance.busyDoingSomethingIgnoreSwitch)
	{
		dblog.isr_printf(debug::dbInfo, "	OnSwitchISR_2 redundant\n\r");
		return;
	}

	dblog.isr_println(debug::dbInfo, "	OnSwitchISR_2 in");


	// ask what changed, clear interrupt
	int causeAndState =
	(Details.switches[0].altSwitchType == stMomentary ?
		// fake the cause and reflect INVERSE state of relay - because MOMENTARY
		(1 << 8) | (digitalRead(GPIO_RELAY) == HIGH ? 0 : 1) :
		// handle the toggle as a toggle
		(1 << 8) | (digitalRead(GPIO_SWITCH2) == HIGH ? 0 : 1));

	HandleCauseAndState(causeAndState);

	dblog.isr_printf(debug::dbVerbose, "OnSwitchISR2 out\n\r");
}
#endif

#ifdef _OLD_WAY

#ifdef GPIO_SWITCH
void ICACHE_RAM_ATTR OnSwitchISR()
{
	if (Details.ignoreISRrequests)
		return;

	// if we're up to our neck in something else (normally WIFI negotiation) ignore this
	if (wifiInstance.busyDoingSomethingIgnoreSwitch)
	{
		dblog.isr_println(debug::dbInfo, "	OnSwitchISR redundant");

#ifndef _PHYSICAL_SWITCH_EXISTS
		// ask what changed, clear interrupt, so we don't leave the INTerrupt hanging
		mcp.InterruptCauseAndCurrentState(true);
#endif
		return;
	}

	dblog.isr_println(debug::dbInfo,"	OnSwitchISR in");


	// ask what changed, clear interrupt
	int causeAndState =
#ifdef _PHYSICAL_SWITCH_EXISTS
		(Details.switches[0].switchType == stMomentary ?
		// fake the cause and reflect INVERSE state of relay - because MOMENTARY
#ifdef GPIO_RELAY
		(1 << 8) | (digitalRead(GPIO_RELAY) == HIGH ? 0 : 1) :
#else
			0:
#endif
		// fake the cause and rstate of switch - because TOGGLE
		(1 << 8) | (digitalRead(GPIO_SWITCH) == HIGH ? 1 : 0));

#else
		mcp.InterruptCauseAndCurrentState(false);
#endif

	HandleCauseAndState(causeAndState);

	dblog.isr_println(debug::dbVerbose, "OnSwitchISR out");

}
#endif

#endif


#ifdef _AT_RGBSTRIP

void DoRGBSwitch(bool on, int rgb)
{
	if (!on)
	{
		rgbHandler.Clear();
	}
	else
	{
		rgbHandler.SetUserPalette(_COLOR_PALLETE_USER1, (rgb & 0xff0000) >> 16, (rgb & 0xff00) >> 8, (rgb & 0xff));
		rgbHandler.SetAllPalette(_COLOR_PALLETE_USER1);
		Details.switches[0].lastRGB = rgb;
	}


	rgbHandler.DisplayAndWait(true);

	Details.switches[0].switchCount++;

	enum switchState newState = on ? swOn : swOff;;
	// reflect in state
	if (newState != Details.switches[0].lastState)
	{
		Details.switches[0].lastState = newState;
		Details.configDirty = true;
	}
}


void DoRGBPaletteSwitch(bool on, unsigned rgbPalette)
{
	if (!on)
	{
		rgbHandler.Clear();
	}
	else
	{
		rgbHandler.SetAllPalette(rgbPalette);
	}


	rgbHandler.DisplayAndWait(true);

	Details.switches[0].switchCount++;

	enum switchState newState = on ? swOn : swOff;;
	// reflect in state
	if (newState != Details.switches[0].lastState)
	{
		Details.switches[0].lastState = newState;
		Details.configDirty = true;
	}

}

#endif

#ifdef NUM_SOCKETS
#ifdef _OLD_WAY

void DoSwitchAntiBounce(int port, bool on)
{
	int causeAndState =
#ifdef _PHYSICAL_SWITCH_EXISTS
	 (1 << 8) | (on ? 1 : 0) ;
#else
		// the switch numbers are 0 based, but needs to be 1 based to make the bit math work
		((port+1) << 8) | (on ? 1 : 0);
#endif
		HandleCauseAndState(causeAndState);
}

void ICACHE_RAM_ATTR HandleCauseAndState(int causeAndState)
{
	for (unsigned switchPort = 0; switchPort < NUM_SOCKETS; switchPort++)
	{
		dblog.isr_printf(debug::dbVerbose, "Checking port %d [%04x]\r\n", switchPort, causeAndState);

		// +8 to get HIBYTE to see if this port CAUSED the interrupt
		if (causeAndState & (1 << (switchPort + 8)))
		{

#if defined(_ALLOW_WIFI_RESET_VIA_QUICKSWITCH) 
			unsigned long now = micros(), interval = 0;
#endif

			// gate against messy tactile/physical switches
			interval = now - Details.switches[switchPort].last_seen_bounce;

			dblog.isr_printf(debug::dbVerbose, "%lu ms ", interval / 1000UL);

			// if it's been longer than the bounce threshold since we saw this button, honour it
			unsigned long bounceToHonour = Details.switches[switchPort].switchType == stMomentary ? Details.debounceThresholdmsMomentary : Details.debounceThresholdmsToggle;
			if (interval >= (unsigned long)(bounceToHonour * 1000))
			{
#ifdef _ALLOW_WIFI_RESET_VIA_QUICKSWITCH
				// move the last seens along
				for (unsigned mover = 0; mover < RESET_ARRAY_SIZE - 1; mover++)
					Details.switches[switchPort].lastSwitchesSeen[mover] = Details.switches[switchPort].lastSwitchesSeen[mover + 1];

				Details.switches[switchPort].lastSwitchesSeen[RESET_ARRAY_SIZE - 1] = now / 1000;

				dblog.isr_printf(debug::dbVerbose, "lastSwitchesSeen (ms) ");

				for (int each = 0; each < RESET_ARRAY_SIZE; each++)
				{
					dblog.isr_printf(debug::dbVerbose, "%lu ", Details.switches[switchPort].lastSwitchesSeen[each]);
				}

				dblog.isr_printf(debug::dbVerbose, "\n\r");
#endif

				// having CAUSED the interrupt, reflect its STATE in the DoRelay call
				DoSwitch(switchPort, (causeAndState & (1 << switchPort)) ? true : false, false);

				Details.switches[switchPort].switchCount++;

#ifdef _ALLOW_WIFI_RESET_VIA_QUICKSWITCH

				// remember the last RESET_ARRAY_SIZE - i'm assuming we won't wrap
				// only reset if we're currently STA
				if (wifiInstance.currentMode == myWifiClass::wifiMode::modeSTA)
				{
					if (Details.switches[switchPort].lastSwitchesSeen[RESET_ARRAY_SIZE - 1] - Details.switches[switchPort].lastSwitchesSeen[0] < (Details.resetWindowms))
					{
						dblog.isr_printf(debug::dbImportant, "RESETTING WIFI!\n\r");
						resetWIFI = true;
					}
				}
#endif
			}
			else
			{
				dblog.isr_printf(debug::dbInfo, "bounce ignored\n\r");
			}
			Details.switches[switchPort].last_seen_bounce = now;
		}
	}
}
#endif
#endif

// honour current switch state
void RevertAllSwitch()
{

	for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
	{
		(*each)->HonourCurrentSwitch();
	}

}


// override switch state
void DoAllSwitch(bool state, bool force)
{

	dblog.printf(debug::dbInfo, "DoAllSwitch: %s %s\r\n", state ? "ON" : "off", force ? "FORCE" : "");

	for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
	{
		dblog.printf(debug::dbInfo, "Switch: %x\r\n", *each);
		(*each)->DoRelay(state, force);
		yield();
	}

	dblog.println(debug::dbInfo, "DoAllSwitch: Out");

}


#ifdef NUM_SOCKETS

#ifdef _OLD_WAY
// if forceSwitchToReflect change polarity of input switch if necessary to reflect this request
void DoSwitch(unsigned portNumber, bool on, bool forceSwitchToReflect)
{
	if (portNumber > 7 || portNumber < 0)
	{
		dblog.isr_printf(debug::dbError, "DoSwitch called out of bounds %u\n\r", portNumber);
		return;
	}

	// saw the switching of the relay create enough microp noise for the toggle switch to be fired
	Details.ignoreISRrequests = true;


	dblog.isr_printf(debug::dbImportant, "DoSwitch: relay %d %s %s\r\n", portNumber, on ? "ON" : "off", forceSwitchToReflect ? "FORCE" : "");

#ifdef _PHYSICAL_SWITCH_EXISTS

#ifdef GPIO_RELAY
	digitalWrite(GPIO_RELAY, on ? HIGH : LOW);
#endif


#ifdef _SONOFF_BASIC
	// LED is inverted on the sonoff
	digitalWrite(GPIO_LED, on ? LOW : HIGH);
#else
	digitalWrite(GPIO_LED, on ? HIGH : LOW);
#endif


#else

	dblog.isr_printf(debug::dbError, "no impl for DoSwitch");


#endif

	enum switchState newState = on ? swOn : swOff;;
	// reflect in state
	if (newState != Details.switches[portNumber].lastState)
	{
		Details.switches[portNumber].lastState = newState;
		Details.configDirty = true;
	}

	Details.ignoreISRrequests = false;

}
#endif
#endif




void ReadHTLMversion()
{
	_MYVERSION_HTML = 0;
	// check for legacy!
	if (SPIFFS.exists(_HTML_VER_FILE))
	{
		fs::File json = SPIFFS.open(_HTML_VER_FILE, "r");
		String jsonString = json.readString();
		json.close();

		jsonBuffer.clear();
		JsonObject& root = jsonBuffer.parseObject(jsonString);

		if (root.success())
		{
			_MYVERSION_HTML = root["version"];
		}

	}

}


void WriteJSONconfig()
{
	dblog.printf(debug::dbInfo, "WriteJSONconfig");

	// try to create it
	fs::File json = SPIFFS.open(_JSON_CONFIG_FILE, "w");

	if (!json)
	{
		dblog.printf(debug::dbError, "failed to create json\n\r");
		return;
	}

	//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
	jsonBuffer.clear();

	JsonObject &root = jsonBuffer.createObject();

#ifdef _AT_RGBSTRIP
	root["rgbCount"] = Details.rgbLedCount;
#endif

#ifdef _OLD_WAY
	root["debounceThresholdmsMomentary"] = Details.debounceThresholdmsMomentary;	
	root["debounceThresholdmsToggle"] = Details.debounceThresholdmsToggle;

	root["resetWindowms"] = Details.resetWindowms;
#endif

	root["friendlyName"] = Details.friendlyName;

	wifiInstance.WriteDetailsToJSON(root, Details.wifi);

#ifdef NUM_SOCKETS

	AddMapToJSON(root, NUM_SOCKETS);

#endif

	dblog.printf(debug::dbVerbose, "jsonBuffer.size used : %d\n\r", jsonBuffer.size());

	///////////////////// written here

	String jsonText;
	root.prettyPrintTo(jsonText);

	dblog.printf(debug::dbVerbose, "JSON : -- %s --\n\r", jsonText.c_str());

	json.write((byte*)jsonText.c_str(), jsonText.length());

	dblog.printf(debug::dbVerbose, "JSON : written\n\r");

	json.close();

	dblog.printf(debug::dbVerbose, "JSON : closed\n\r");

	Details.configDirty = false;
}


void ReadJSONconfig()
{
	dblog.printf(debug::dbInfo, "ReadJSONconfig\n\r");

#ifdef _ERASE_JSON_CONFIG
	dblog.printf(debug::dbImportant, "erasing JSON file\n\r");
	SPIFFS.remove(_JSON_CONFIG_FILE);
#endif

	// check for legacy!
	if (SPIFFS.exists(_LEGACY_JSON_CONFIG_FILE))
	{
		dblog.println(debug::dbImportant, "Legacy config file found ... ");

		// check for co-existence!
		if (SPIFFS.exists(_JSON_CONFIG_FILE))
		{
			dblog.println(debug::dbImportant, "coexist ... deleting legacy");
			if (!SPIFFS.remove(_LEGACY_JSON_CONFIG_FILE))
			{
				dblog.println(debug::dbError, "delete failed ... bad");
				return;
			}
		}
		else
		{
			dblog.println(debug::dbImportant, "migration ... renaming");

			if (!SPIFFS.rename(_LEGACY_JSON_CONFIG_FILE, _JSON_CONFIG_FILE))
			{
				dblog.println(debug::dbError, "Rename failed ... bad");
				return;
			}
		}

	}

	if (SPIFFS.exists(_LEGACY_JSON_CONFIG_FILE))
	{
		dblog.println(debug::dbError, "Legacy File still exists ... bad");
	}


	if (!SPIFFS.exists(_JSON_CONFIG_FILE))
	{
		dblog.printf(debug::dbImportant, "'%s' does not exist\n\r", _JSON_CONFIG_FILE);
		// file does not exist
		WriteJSONconfig();

		return;

	}
	// first - see if the file is there
	fs::File json = SPIFFS.open(_JSON_CONFIG_FILE, "r");

	String jsonString = json.readString();

	json.close();

	dblog.printf(debug::dbInfo, "JSON: (%d) -- %s --\n\r", jsonString.length(), jsonString.c_str());

	//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
	jsonBuffer.clear();
	JsonObject& root = jsonBuffer.parseObject(jsonString);

	if (!root.success())
	{
		dblog.printf(debug::dbError, "JSON parse failed\n\r");

		// kill it - and write it again
		SPIFFS.remove(_JSON_CONFIG_FILE);

		dblog.printf(debug::dbInfo, "JSON file deleted\n\r");

		WriteJSONconfig();

		return;

	}
	else
	{
		dblog.printf(debug::dbVerbose, "JSON parsed\n\r");
	}

	Details.configDirty = false;

#ifdef _AT_RGBSTRIP
	if(root.containsKey("rgbCount"))
		Details.rgbLedCount=root["rgbCount"];

	dblog.printf(debug::dbInfo,"Changing LED count to %d\n\r", Details.rgbLedCount);

	// tell the handler how big it is
	rgbHandler.Clear();
	rgbHandler.SetSize(Details.rgbLedCount);
	rgbHandler.DisplayAndWait(true);

#endif

#ifdef _OLD_WAY
	Details.debounceThresholdmsMomentary= root["debounceThresholdmsMomentary"];
	Details.debounceThresholdmsToggle=root["debounceThresholdmsToggle"];
	Details.resetWindowms= root["resetWindowms"];

#endif

	if (root.containsKey("friendlyName"))
	{
		//String interim = root["friendlyName"].asString();
		String interim = root["friendlyName"].as<char*>();
		
		if (interim.length())
			Details.friendlyName = interim;
	}

	wifiInstance.ReadDetailsFromJSON(root, Details.wifi);

#ifdef NUM_SOCKETS

#ifdef _OLD_WAY
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

#ifdef _AT_RGBSTRIP
				if(theSwitch.containsKey("lastRGB"))
					Details.switches[each].lastRGB = theSwitch["lastRGB"];
#endif
			}
			else
			{
				dblog.printf(debug::dbImportant, "switchMap switch %d not found\n\r", each);
			}
		}
	}
#endif

#endif

}



// void ResetWIFI()
// {
// 	DEBUG(DEBUG_IMPORTANT, Serial.println("Resetting WIFI"));

// 	wifiMode now = currentMode;

// 	ConnectWifi(wifiMode::modeOff);

// 	ConnectWifi(now);
// }

void RebootMe()
{
	// hangs the esp
	//WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while (1)wdt_reset();
	//ESP.restart();
}

#ifdef _ALLOW_WIFI_RESET

void ResetToAP()
{
	dblog.printf(debug::dbImportant, "Resetting\n\r");

	resetWIFI = false;
	// clear the credentials
	Details.wifi.configured = false;
	Details.wifi.password = String();
	Details.wifi.ssid = String();
	WriteJSONconfig();
	// and reconnect as an AP
	wifiInstance.ConnectWifi(myWifiClass::wifiMode::modeAP,Details.wifi);

}

// called when i haven't been connected to a configurid ssid for x
// sideload an AP so i can be configured
void AddAP()
{
	dblog.printf(debug::dbImportant, "AddAP\n\r");

	wifiInstance.ConnectWifi(myWifiClass::wifiMode::modeSTAandAP,Details.wifi);


}


#endif



void AddSwitch(baseSwitch *newSwitch)
{
	if(newSwitch)
	{
		dblog.println(debug::dbInfo, "Adding switch");
		Details.switches.push_back(newSwitch);
		MultiSwitch* multi=(MultiSwitch*)newSwitch;
		for(unsigned eachChild=0;eachChild<newSwitch->ChildSwitchCount();eachChild++)
		{
			Details.switches.push_back(multi->GetChild(eachChild));
		}
		dblog.println(debug::dbInfo, "Added switch");
		newSwitch->HonourCurrentSwitch();
	}
}




void setup(void) 
{
#ifdef _DEBUG
	gdbstub_init();
#endif


	// tell the debugger its name
#if defined(_USE_SYSLOG)
	dblog.SetHostname(wifiInstance.m_hostName.c_str());
#else
	dblog.begin(115200);
#endif


#ifdef _OLD_WAY

#ifdef NUM_SOCKETS

	// reset the bounce thresh-holds
	for (int eachSwitch = 0; eachSwitch < NUM_SOCKETS; eachSwitch++)
	{
#ifdef _ALLOW_WIFI_RESET_VIA_QUICKSWITCH
		// clean up the switch times
		memset(&Details.switches[eachSwitch].lastSwitchesSeen, 0, sizeof(Details.switches[eachSwitch].lastSwitchesSeen));
#endif

		Details.switches[eachSwitch].switchCount = 0;
		Details.switches[eachSwitch].last_seen_bounce = 0;
	}

#endif

	dblog.printf(debug::dbImportant, "\r\n\n\nRunning %s\n\r", _MYVERSION);
	dblog.printf(debug::dbImportant, "Hostname %s\n\r", wifiInstance.m_hostName.c_str());



	SPIFFS.begin();

	ReadJSONconfig();

	ReadHTLMversion();

	dblog.println(debug::dbImportant, wifiInstance.m_hostName.c_str());
	dblog.printf(debug::dbVerbose, "bounceMomentary %lu\n\r", Details.debounceThresholdmsMomentary);
	dblog.printf(debug::dbVerbose, "bounceToggle %lu\n\r", Details.debounceThresholdmsToggle);
	dblog.printf(debug::dbVerbose, "reset %lu\n\r", Details.resetWindowms);

	enum myWifiClass::wifiMode intent = myWifiClass::wifiMode::modeUnknown;

	if (Details.wifi.configured)
	{
		dblog.println(debug::dbInfo, "wifi credentials found");
		dblog.println(debug::dbVerbose, Details.wifi.ssid);
		dblog.println(debug::dbVerbose, Details.wifi.password);
		intent = myWifiClass::wifiMode::modeSTA;
	}
	else
	{
		dblog.println(debug::dbWarning, "WiFi not configured");
		intent = myWifiClass::wifiMode::modeAP;

	}

#ifdef _AT_RGBSTRIP

	rgbHandler.begin();

#endif



#ifdef _PHYSICAL_SWITCH_EXISTS

	// set the relay pin to output
#ifdef GPIO_RELAY
	pinMode(GPIO_RELAY, OUTPUT);
#endif


#ifdef GPIO_SWITCH
	// and the switch pin to input - pullup
	pinMode(GPIO_SWITCH, INPUT_PULLUP);
	// for momentary switches we just look for low
	attachInterrupt(GPIO_SWITCH, OnSwitchISR, Details.switches[0].switchType == stMomentary ? FALLING : CHANGE);
#endif

#ifdef GPIO_SWITCH2
	// and the switch pin to input - pullup
	pinMode(GPIO_SWITCH2, INPUT_PULLUP);
	// for toggle switches we just look for change
//	attachInterrupt(GPIO_SWITCH2, OnSwitchISR2, Details.switches[0].altSwitchType == stMomentary ? ONLOW : CHANGE);
	attachInterrupt(GPIO_SWITCH2, OnSwitchISR2, Details.switches[0].altSwitchType == stMomentary ? FALLING : CHANGE);
#endif

#endif

#ifdef GPIO_LED
	pinMode(GPIO_LED, OUTPUT);
#endif


	// default off, and don't force switches
	DoAllSwitch(false,false);

	// try to connect to the wifi
	wifiInstance.ConnectWifi(intent, Details.wifi);


	// honour current switch state
	RevertAllSwitch();

	// set up the callback handlers for the webserver
	InstallWebServerHandlers();


#else

	dblog.printf(debug::dbImportant, "\r\n\n\nRunning %s\n\r", _MYVERSION);
	dblog.printf(debug::dbImportant, "Hostname %s\n\r", wifiInstance.m_hostName.c_str());

	SPIFFS.begin();

	ReadJSONconfig();

	ReadHTLMversion();

	dblog.println(debug::dbImportant, wifiInstance.m_hostName.c_str());

	enum myWifiClass::wifiMode intent = myWifiClass::wifiMode::modeUnknown;

	if (Details.wifi.configured)
	{
		dblog.println(debug::dbInfo, "wifi credentials found");
		dblog.println(debug::dbVerbose, Details.wifi.ssid);
		dblog.println(debug::dbVerbose, Details.wifi.password);
		intent = myWifiClass::wifiMode::modeSTA;
	}
	else
	{
		dblog.println(debug::dbWarning, "WiFi not configured");
		intent = myWifiClass::wifiMode::modeAP;

	}

// thermos and lux
//#define WEMOS_COM3
// PIR
#define WEMOS_COM4
// 6switch
//#define WEMOS_COM5 

#ifdef WEMOS_COM3
	// load up the sensors and switches
	AddSwitch(new SonoffBasicNoLED(&dblog));


	// OF COURSE i reused D7 which is used by Sonoff! duh!
	Details.sensors.push_back(new DallasSingleSensor(D7, &dblog));
	Details.sensors.push_back(new BME280Sensor(&dblog));
	Details.sensors.push_back(new MAX44009Sensor(&dblog));

	Details.sensors.push_back(new testInstantSensor(&dblog));

#elif defined(WEMOS_COM4) 

	Details.sensors.push_back(new PIRInstantSensor(&dblog, D7));

#elif defined(WEMOS_COM5) 

	Details.sensors.push_back(new testInstantSensor(&dblog, 10*60*1000));
	AddSwitch(new MCP23017MultiSwitch(&dblog, 6, SDA, SCL, D5));


#endif

	// default off, and don't force switches
	// DoAllSwitch(false,false);

	// try to connect to the wifi
	wifiInstance.ConnectWifi(intent, Details.wifi);


	// set up the callback handlers for the webserver
	InstallWebServerHandlers();


#endif

}



// look for my siblings
void FindPeers()
{
	// if we don't have wifi, dont mdns (it crashes for me)
	switch(wifiInstance.currentMode)
	{
		case myWifiClass::modeOff: 
		case myWifiClass::modeSTA_unjoined:
		case myWifiClass::modeCold:
		case myWifiClass::modeUnknown:
			dblog.println(debug::dbInfo, "No WIFI not doing FindPeers");
			// very bad form
			return;
		case myWifiClass::modeAP:
		case myWifiClass::modeSTA:
		case myWifiClass::modeSTAspeculative:
		case myWifiClass::modeSTAandAP:
		default:
			break;
	}

	dblog.printf(debug::dbInfo, "Looking for '%s' siblings ...\n\r", mdsnNAME);
	
	// get a list of what's out there
	services.clear();
	if (wifiInstance.QueryServices(mdsnNAME, services))
	{
		int found=(int)services.size();
		dblog.printf(debug::dbInfo, "Found %d sibling%c!!\n\r", found, found==1?' ':'s');
		for (auto iterator = services.begin(); iterator != services.end(); iterator++)
		{
			dblog.printf(debug::dbInfo, "\t%s.local @ %s\n\r", iterator->hostName.c_str(), iterator->IP.toString().c_str());
		}
		
	}
	else
	{
		dblog.println(debug::dbInfo, "No others services found");
	}

}

// set up all the handlers for the web server
void InstallWebServerHandlers()
{
	dblog.println(debug::dbVerbose, "InstallWebServerHandlers IN");

	// set up the json handlers
	// POST
	// all ON/OFF 
	// switch ON/OFF
	// revert

	// make all the relays reflect their switches
	wifiInstance.server.on("/revert", []() {

		dblog.println(debug::dbImportant, "/revert");

		RevertAllSwitch();

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();
	});

	// do something to all of them
	wifiInstance.server.on("/all", []() {

		dblog.println(debug::dbImportant, "/all");

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

	// fetch the latest set of html pages from a peer
	wifiInstance.server.on("/json/updatehtml", HTTP_POST, []() {

		// we need the host
		dblog.println(debug::dbImportant, "json updatehtml posted");
		dblog.println(debug::dbImportant, wifiInstance.server.arg("plain"));

		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(wifiInstance.server.arg("plain"));

		String host = root["host"];

		FetchNewPages(host);

		wifiInstance.server.send(200, "application/json", "");
	});

#ifdef _OTA_AVAILABLE

	// do an OTA update from a provided URL
	wifiInstance.server.on("/json/upgrade", HTTP_POST, []() {

		dblog.println(debug::dbImportant, "json upgrade posted");
		dblog.println(debug::dbImportant, wifiInstance.server.arg("plain"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(wifiInstance.server.arg("plain"));

		String host = root["host"];
		int port = root["port"];
		String url= root["url"];

		delay(_WEB_TAR_PIT_DELAY);

		enum HTTPUpdateResult result = ESPhttpUpdate.update(host, port, url, _MYVERSION);
		//enum HTTPUpdateResult result = ESPhttpUpdate.update(wifiInstance.m_wificlient, host, port, url, _MYVERSION);

		switch (result)
		{
		case HTTP_UPDATE_FAILED:
			dblog.printf(debug::dbError, "HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
			break;
		case HTTP_UPDATE_NO_UPDATES:
			dblog.println(debug::dbImportant, "no updates");
			break;
		case HTTP_UPDATE_OK:
			dblog.println(debug::dbImportant, "update succeeded");
			break;
		}

		StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer2;
		
		// 'plain' is the secret source to get to the body
		JsonObject&replyroot = jsonBuffer2.createObject();
		replyroot["result"] = result;
		if (result != HTTP_UPDATE_OK)
		{
			JsonObject &details = replyroot.createNestedObject("Details");
			details["espNarrative"] = ESPhttpUpdate.getLastErrorString();
			details["espResult"] = ESPhttpUpdate.getLastError();
		}

		String bodyText;
		replyroot.printTo(bodyText);

		dblog.println(debug::dbVerbose, bodyText);

		wifiInstance.server.send(200, "application/json", bodyText);

	});

#endif

	// inverse of whatever it's currently doing
	wifiInstance.server.on("/toggle", []() {

		dblog.println(debug::dbImportant, "/toggle");

		delay(_WEB_TAR_PIT_DELAY);


		// TODO pick the right one
		// just go thru them all
		for(auto each=Details.switches.begin();each!=Details.switches.end();each)
			(*each)->ToggleRelay();



	});

	wifiInstance.server.on("/button", []() {

		dblog.println(debug::dbInfo, "/button");

		for (int count = 0; count < wifiInstance.server.args(); count++)
		{
			dblog.printf(debug::dbInfo, "%d. %s = %s \n\r", 
				count+1, 
				wifiInstance.server.argName(count).c_str(), 
				wifiInstance.server.arg(count).c_str()
			);
		}


#ifdef _OLD_WAY


		// one trick pony
		if (wifiInstance.server.hasArg("action"))
		{

#if defined( _AT_RGBSTRIP )
			bool action = wifiInstance.server.arg("action") == "on" ? true : false;

			if (wifiInstance.server.hasArg("rgb") || wifiInstance.server.hasArg("r") || wifiInstance.server.hasArg("g") || wifiInstance.server.hasArg("b"))
			{
				unsigned rgb = 0;
				if (wifiInstance.server.hasArg("rgb"))
				{
					rgb = wifiInstance.server.arg("rgb").toInt();
				}
				else
				{
					rgb = ((wifiInstance.server.arg("r").toInt()&0xff) << 16) |
							((wifiInstance.server.arg("g").toInt() & 0xff) << 8) |
							(wifiInstance.server.arg("b").toInt() & 0xff) ;
				}

				dblog.printf(debug::dbInfo, "DoRGBSwitch 0x%06x\n\r", rgb);


				// squirt that down as user palette
				DoRGBSwitch(action, rgb);
			}
			else
			{ 
				// what it last was
				DoRGBSwitch(action, Details.switches[0].lastRGB);
			}
#elif defined(NUM_SOCKETS)
			bool action = wifiInstance.server.arg("action") == "on" ? true : false;
			DoSwitchAntiBounce(0, action);
#endif		
		}


#endif

		if (wifiInstance.server.hasArg("action"))
		{
			bool action = wifiInstance.server.arg("action") == "on" ? true : false;
			
			if(wifiInstance.server.hasArg("port"))
			{
				int port=wifiInstance.server.arg("port").toInt();
				if(port<Details.switches.size())
				{
					Details.switches[port]->DoRelay(action);
				}
				else
				{
					dblog.printf(debug::dbWarning, "asked to action %d - exceeds maximum %d\r\n", port,Details.switches.size()-1);
				}
			}
			else
			{
				// just go thru them all
				for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
					(*each)->DoRelay(action);
			}
			

		}

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();

	});

	wifiInstance.server.on("/resetCounts", []() {

		dblog.println(debug::dbImportant, "/resetCounts");

		delay(_WEB_TAR_PIT_DELAY);

		for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
			(*each)->ResetTransitionCount();

		wifiInstance.server.send(200,"text/html","<html/>");

	});

	wifiInstance.server.on("/reboot", []() {

		dblog.println(debug::dbImportant, "/reboot");

		RebootMe();

	});

#ifdef _ALLOW_WIFI_RESET_OVER_WIFI

	wifiInstance.server.on("/resetWIFI", []() {

		dblog.println(debug::dbImportant, "/resetWIFI");

		ResetToAP();

	});

#endif

	wifiInstance.server.on("/stopAP", []() {

		dblog.println(debug::dbImportant, "/stopAP");

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

		dblog.println(debug::dbImportant, "/ requested");

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();

	});

	wifiInstance.server.on("/default.htm", []() {

		dblog.println(debug::dbImportant, "/default.htm");

		delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();

	});

	// posted config
	wifiInstance.server.on("/json/config", HTTP_POST, []() {

		dblog.println(debug::dbImportant, "json config posted");
		dblog.println(debug::dbImportant, wifiInstance.server.arg("plain"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(wifiInstance.server.arg("plain"));

#ifdef _OLD_WAY
		if (root.containsKey("bouncemsMomentary"))
		{
			Details.debounceThresholdmsMomentary = root["bouncemsMomentary"];
		}
		if (root.containsKey("bouncemsToggle"))
		{
			Details.debounceThresholdmsToggle = root["bouncemsToggle"];
		}
		if (root.containsKey("resetms"))
		{
			Details.resetWindowms = root["resetms"];
		}
		if (root.containsKey("debugLevel"))
		{
			int debugLevel = root["debugLevel"];
			dblog.printf(debug::dbAlways, "Debug logging changed to %d (was %d)\n\r", dblog.m_currentLevel, (int)debugLevel);
			dblog.m_currentLevel = (debug::dbLevel) debugLevel;
		}
#endif

		if (root.containsKey("friendlyName"))
		{
			Details.friendlyName = root["friendlyName"].as<char*>();
		}


#ifdef _AT_RGBSTRIP

		if (root.containsKey("ledCount"))
		{
			Details.rgbLedCount = root["ledCount"];
			dblog.printf(debug::dbImportant, "Changing LED count to %d\n\r", Details.rgbLedCount);
			rgbHandler.Clear();
			rgbHandler.SetSize(Details.rgbLedCount);
			rgbHandler.DisplayAndWait(true);
		}

#endif


		

		// extract the details
		WriteJSONconfig();
		delay(_WEB_TAR_PIT_DELAY);

		wifiInstance.server.send(200, "text/html", "<html></html>");

		});


	wifiInstance.server.on("/json/listen", HTTP_POST, []() {

		IPAddress recipientAddr = wifiInstance.server.client().remoteIP();

		dblog.printf(debug::dbImportant, "json listen posted from %s\n\r",recipientAddr.toString().c_str());
		dblog.println(debug::dbImportant, wifiInstance.server.arg("plain"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(wifiInstance.server.arg("plain"));

		int recipientPort = root["port"];

		// can be switch or sensor
		if(root.containsKey("sensor"))
		{

			int recipientSensor = root["sensor"];

			if(recipientSensor<Details.sensors.size())
			{
				dblog.printf(debug::dbInfo, "Adding recipient %s:%d Sensor %d\n\r", recipientAddr.toString().c_str(), recipientPort, recipientSensor);
				Details.sensors[recipientSensor]->AddAnnounceRecipient(recipientAddr,recipientPort,wifiInstance.server.arg("plain"));
			}
			else
			{
				dblog.println(debug::dbError, "Sensor exceeded bounds");
			}

		}
		else if (root.containsKey("switch"))
		{
			int recipientSwitch = root["switch"];

			if(recipientSwitch<Details.switches.size())
			{
				dblog.printf(debug::dbInfo, "Adding recipient %s:%d Switch %d\n\r", recipientAddr.toString().c_str(), recipientPort, recipientSwitch);
				Details.switches[recipientSwitch]->AddAnnounceRecipient(recipientAddr,recipientPort,wifiInstance.server.arg("plain"));


			}
			else
			{
				dblog.println(debug::dbError, "Switch exceeded bounds");
			}

		}

		//delay(_WEB_TAR_PIT_DELAY);
		wifiInstance.server.send(200, "text/html", "<html></html>");


	});





	wifiInstance.server.on("/json/wifi", HTTP_POST, []() {

		dblog.println(debug::dbImportant, "json wifi posted");
		dblog.println(debug::dbImportant, wifiInstance.server.arg("plain"));

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

	wifiInstance.server.on("/json/htmlPages", HTTP_GET, []() {
		// give them back the port / switch map

		dblog.println(debug::dbInfo, "json htmlPages called");

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = wifiInstance.m_hostName.c_str();
		root["pageCount"] = servedFiles.size();
		root["version"] = _MYVERSION;
		root["versionHTML"] = _MYVERSION_HTML;

		JsonArray &htmlFiles = root.createNestedArray("htmlPages");
		//for (unsigned each = 0; each < servedFiles.size(); each++)
		for(auto each=servedFiles.begin();each!=servedFiles.end();each++)
		{
			JsonObject &htmlFile = htmlFiles.createNestedObject();
			htmlFile["name"] = each->first.c_str();
			htmlFile["size"] = each->second;
			
			dblog.printf(debug::dbInfo, "adding %s (%lu)\n\r",each->first.c_str(), each->second);
		}

		String jsonText;
		root.prettyPrintTo(jsonText);

		dblog.println(debug::dbVerbose, jsonText);

		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
	});

	wifiInstance.server.on("/json/state", HTTP_GET, []() {
		// give them back the port / switch map

		dblog.println(debug::dbInfo, "json state called");

		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = wifiInstance.m_hostName.c_str();
		root["friendlyName"] = Details.friendlyName;
		if(wifiInstance.localIP().isSet())
			root["ip"] = wifiInstance.localIP().toString();

		root["switchCount"] = Details.switches.size();

		JsonArray &switchState = root.createNestedArray("switchState");
		int count=0;
		for(auto each=Details.switches.begin();each!=Details.switches.end();each++, count++)
		{
			JsonObject &switchRelay = switchState.createNestedObject();
			switchRelay["switch"] = count;
			// html js expects 1 or 0
			switchRelay["state"] = (*each)->GetRelay()?1:0;
			switchRelay["stateChanges"] = (*each)->GetTransitionCount();

			switch ((*each)->GetSwitchType())
			{
			case baseSwitch::stUndefined:
				switchRelay["type"] = "!UNDEFINED!";
				break;

			case baseSwitch::stMomentary:
				switchRelay["type"] = "Momentary";
				break;
			case baseSwitch::stToggle:
				switchRelay["type"] = "Toggle";
				break;
			case baseSwitch::stVirtual:
				switchRelay["type"] = "Virtual";
				break;

			default:
				switchRelay["type"] = "!Unsupported!";
				break;

			}

			switchRelay["name"] = (*each)->GetName();
		}

		root["sensorCount"] = Details.sensors.size();
		JsonArray &sensorState = root.createNestedArray("sensorState");
		count=0;
		for(auto each=Details.sensors.begin();each!=Details.sensors.end();each++, count++)
		{
			JsonObject &switchRelay = sensorState.createNestedObject();
			switchRelay["sensor"] = count;

			(*each)->GetSensorValue(switchRelay);

			switchRelay["name"] = (*each)->GetName();
		}



		String jsonText;
		root.prettyPrintTo(jsonText);

		dblog.println(debug::dbVerbose, jsonText);

		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
	});


	wifiInstance.server.on("/json/config", HTTP_GET, []() {
		// give them back the port / switch map

		dblog.println(debug::dbImportant, "json config called");

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = wifiInstance.m_hostName.c_str();
		root["version"] = _MYVERSION;
		root["versionHTML"] = _MYVERSION_HTML;
		if(wifiInstance.localIP().isSet())
			root["ip"] = wifiInstance.localIP().toString();
		root["mac"] = wifiInstance.macAddress();

#ifdef _OLD_WAY		
		root["bouncemsMomentary"] = Details.debounceThresholdmsMomentary;
		root["bouncemsToggle"] = Details.debounceThresholdmsToggle;
		root["resetms"] = Details.resetWindowms;
		root["debugLevel"] = (int)dblog.m_currentLevel;
#endif

		root["friendlyName"] = Details.friendlyName;

#ifdef _AT_RGBSTRIP
		root["ledCount"] = Details.rgbLedCount;
#endif

		// add sensors
		root["sensorCount"] = Details.sensors.size();
		JsonArray &sensorConfig = root.createNestedArray("sensorConfig");
		int count=0;
		for(auto each=Details.sensors.begin();each!=Details.sensors.end();each++, count++)
		{
			JsonObject &switchRelay = sensorConfig.createNestedObject();
			switchRelay["sensor"] = count;

			(*each)->GetSensorConfig(switchRelay);

			switchRelay["name"] = (*each)->GetName();
		}


		root["switchCount"] = Details.switches.size();
		JsonArray &switchConfig = root.createNestedArray("switchConfig");
		count=0;
		for(auto each=Details.switches.begin();each!=Details.switches.end();each++, count++)
		{
			JsonObject &switchRelay = switchConfig.createNestedObject();
			switchRelay["switch"] = count;

			String impl=(*each)->GetImpl();
			if(impl.length())
				switchRelay["impl"]=impl;

			//(*each)->GetSwitchConfig(switchRelay);

			switchRelay["name"] = (*each)->GetName();
		}

		String jsonText;
		root.prettyPrintTo(jsonText);

		dblog.println(debug::dbVerbose, jsonText);

		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
	});

	wifiInstance.server.on("/json/wificonfig", HTTP_GET, []() {
		// give them back the port / switch map

		dblog.println(debug::dbImportant, "json wificonfig called");

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = wifiInstance.m_hostName.c_str();
		root["ssid"] = wifiInstance.SSID();
		if(wifiInstance.localIP().isSet())
			root["ip"] = wifiInstance.localIP().toString();

		String jsonText;
		root.prettyPrintTo(jsonText);

		dblog.println(debug::dbVerbose, jsonText);

		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
	});

	///json/peers
	wifiInstance.server.on("/json/peers", HTTP_GET, []() {
		// give them back the port / switch map

		dblog.println(debug::dbImportant, "json peers called");

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();
		root["name"] = wifiInstance.m_hostName.c_str();
		root["peerCount"] = services.size();
		if(wifiInstance.localIP().isSet())
			root["ip"] = wifiInstance.localIP().toString();


		// let's get all peers we can see
		
		JsonArray &peers = root.createNestedArray("peers");

		for (size_t each = 0; each < services.size(); each++)
		{
			JsonObject &peer = peers.createNestedObject();

			dblog.printf(debug::dbInfo, "%d '%s' %s\n\r", each + 1, services[each].hostName.c_str(), services[each].IP.toString().c_str());
			peer["name"]=services[each].hostName;
			peer["ip"]=services[each].IP.toString();
		}


		String jsonText;
		root.prettyPrintTo(jsonText);

		dblog.println(debug::dbVerbose, jsonText);

		// do not cache
		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "text/json", jsonText);
	});




	wifiInstance.server.on("/json/wifi", HTTP_GET, []() {
		// give them back the port / switch map

		dblog.println(debug::dbImportant, "json wifi called");

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

			dblog.printf(debug::dbInfo, "%d '%s' %d \n\r", each + 1, allWifis[each].first.c_str(), allWifis[each].second);

		}
		

		String jsonText;
		root.prettyPrintTo(jsonText);

		dblog.println(debug::dbVerbose, jsonText);

		// do not cache
		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "text/json", jsonText);
	});

	// serve up everthing in SPIFFS
	SPIFFS.openDir("/");

	
	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {
		String file = dir.fileName();

		// ensure it doesn't have a leading underscore - hidden flag for me
		if (file.length() > 1 && file[1] == '_')
		{
			dblog.printf(debug::dbInfo, "Skipping %s\n\r", file.c_str());
			continue;
		}

		// cache it for an hour
		wifiInstance.server.serveStatic(file.c_str(), SPIFFS, file.c_str(),"Cache-Control: public, max-age=60");

		dblog.printf(debug::dbInfo, "Serving %s\r\n", file.c_str());

		// remove the slash
		// file.remove(0,1);
		servedFiles.push_back(std::pair<String,size_t>(file, dir.fileSize()));
	}

	dblog.printf(debug::dbVerbose, "InstallWebServerHandlers OUT\n\r");

}

void FetchNewPages(String src)
{
	// build the url
	String url("http://");
	url += src + "/json/htmlPages";
	dblog.printf(debug::dbVerbose, "About to query %s\n\r", url.c_str());
	// query for the pages available
	HTTPClient http;
	//if (!http.begin(wifiInstance.m_wificlient, url))
	if (!http.begin(url))
	{
		dblog.println(debug::dbError, "Failed in http.begin()");
	}

	int httpCode=http.GET();

	// check the result
	if (httpCode != 200)
	{
		dblog.printf(debug::dbError, "http.GET() returned %d\n\r", httpCode);
		return;
	}

	// get the body
	String payload = http.getString();
	dblog.println(debug::dbVerbose,  payload);
	http.end();

	// then iterate thru the files
	JsonObject& root = jsonBuffer.parseObject(payload);

	int htmlPagesCount = root["htmlPages"].size();

	// get the totalsize
	size_t totalFileSize = 0;
	for (int each = 0; each < htmlPagesCount; each++)
		totalFileSize += (size_t)root["htmlPages"][each]["size"];

	fs::FSInfo spiffInfo;
	SPIFFS.info(spiffInfo);

	size_t availSpace = spiffInfo.totalBytes - spiffInfo.usedBytes;
	if (totalFileSize > availSpace)
	{
		dblog.println(debug::dbError, "Not enough space for files!");
		return;
	}

	dblog.printf(debug::dbInfo, "Writing %lu bytes into %lu space\n\r", totalFileSize, availSpace);

	for (int each = 0; each < htmlPagesCount; each++)
	{
		String fileToFetch = root["htmlPages"][each]["name"];
		size_t fileSize = root["htmlPages"][each]["size"];
		String fileToFetchInterim = fileToFetch + ".new";

		dblog.printf(debug::dbInfo, "Fetching %s to %s ", fileToFetch.c_str(), fileToFetchInterim.c_str());

		url = "http://";
		url += src;
		url += fileToFetch;

		//if (!http.begin(wifiInstance.m_wificlient, url))
		if (!http.begin( url))
		{
			dblog.println(debug::dbError, "\n\rFailed in http.begin()");
			return;
		}

		httpCode = http.GET();
		if (httpCode != 200)
		{
			// clean up
			dblog.printf(debug::dbError, "\n\rFailed in http.get() : %d \n\r", httpCode);
			return;
		}

		size_t written = 0;
		fs::File f = SPIFFS.open(fileToFetchInterim, "w");
		if(f)
		{
#define _MAX_STREAM_BLOB	512
			uint8_t blob[_MAX_STREAM_BLOB];
			WiFiClient wific=http.getStream();
			for (size_t progress=0;progress<fileSize;)
			{


				// fix it up for max buffer size
				int available = wific.available();
				dblog.printf(debug::dbInfo, available?".":"_");
				//dblog.printf(debug::dbVerbose, "%d Available\n\r", available);
				size_t count = available > _MAX_STREAM_BLOB ? _MAX_STREAM_BLOB : available;
				count = wific.readBytes(&blob[0], count);
				progress += count;

				if (f.write(&blob[0], count) != count)
				{
					dblog.printf(debug::dbError, "\n\rFailed in write all bytes %d \n\r", count);
					f.close();
					http.end();
					return;
				}

				written += count;
				yield();
			}

			// does this make the numbers match?
			f.flush();
			dblog.println(debug::dbInfo, "");

			// check that we committed the right size!
			size_t currentSize = written;// f.size();
			if (currentSize != fileSize)
			{
				dblog.printf(debug::dbError, "Mismatch in filesize - mine %lu, original %lu \n\r", currentSize, fileSize);
				http.end();
				f.close();
				continue;
			}
			else
			{
				SPIFFS.remove(fileToFetch);
				dblog.printf(debug::dbInfo, "Renaming %s to %s\n\r", fileToFetchInterim.c_str(), fileToFetch.c_str());
				if (!SPIFFS.rename(fileToFetchInterim, fileToFetch))
				{
					dblog.printf(debug::dbError, "Rename of %s Failed\n\r", fileToFetchInterim.c_str());
					http.end();
					f.close();
					return;
				}
			}

			f.close();
			dblog.println(debug::dbVerbose, "written OK");

		}
		http.end();

	}
	dblog.println(debug::dbVerbose, "Finished");
	ReadHTLMversion();

}

void SendServerPage()
{
	// given the current state of the device, send the appropriate page back
	
	String toOpen("/Error.htm");
	switch (wifiInstance.currentMode)
	{
	case myWifiClass::wifiMode::modeSTAandAP:
		toOpen="/STAAPmode.htm";
		break;
	case myWifiClass::wifiMode::modeAP:
		toOpen = "/APmode.htm";
		break;
	case myWifiClass::wifiMode::modeSTA:
		toOpen = "/STAmode.htm";
		break;
	case myWifiClass::wifiMode::modeUnknown:
	default:
		toOpen = "/Error.htm";
		break;

	}

	if(SPIFFS.exists(toOpen))
	{

		File f = SPIFFS.open(toOpen, "r");

		// let's make sure it bloody exists !
		wifiInstance.server.streamFile(f, "text/html");
		f.close();

	}
	else
	{
		dblog.printf(debug::dbError,"SPIFFS error - %s does not exist\n\r", toOpen.c_str());
	}

}

#ifdef NUM_SOCKETS

void AddMapToJSON(JsonObject &root, unsigned numSockets)
{
#ifdef _OLD_WAY
	dblog.printf(debug::dbVerbose, "AddMapToJSON %d\n\r", numSockets);

	root["switchCount"] = numSockets;

	JsonArray &switchMap = root.createNestedArray("switchMap");

	for (unsigned each = 0; each < numSockets; each++)
	{
		JsonObject &theSwitch = switchMap.createNestedObject();
		theSwitch["switch"] = each;
		theSwitch["name"] = Details.switches[each].name.c_str();
		theSwitch["relay"] = Details.switches[each].relay;
		theSwitch["lastState"] = (int)Details.switches[each].lastState;

#ifdef _AT_RGBSTRIP
		theSwitch["lastRGB"]=Details.switches[each].lastRGB;
#endif

	}
#endif
}

#endif

#ifdef _TEST_WFI_STATE
unsigned long lastTested = 0;
#define _TEST_WIFI_MILLIS	(15*60*1000)
#endif

#define _FETCH_PEERS_TIMEOUT_MS	(15*60*1000)	// 15 mins
//#define _FETCH_PEERS_TIMEOUT_MS	(1*60*1000)	// 1 min
unsigned long lastCheckedForPeers = 0;

void loop(void) 
{

#ifdef _ALLOW_WIFI_RESET
	if (resetWIFI)
		ResetToAP();
#endif

#ifdef _ALLOW_WIFI_RESET_AFTER_APIJOIN_TIMOUT

	if(wifiInstance.currentMode!=myWifiClass::modeSTA_unjoined)
	{
		runtimeWhenLastJoined=millis();
	}

	if(millis()-runtimeWhenLastJoined > _ALLOW_WIFI_RESET_AFTER_APIJOIN_TIMOUT)
	{
		AddAP();
		runtimeWhenLastJoined=millis();
	}
#endif

	if (Details.configDirty)
		WriteJSONconfig();

#ifdef _OLD_WAY
	// just in case we get into a bool corner
	Details.ignoreISRrequests = false;
#endif

	wifiInstance.server.handleClient();
	wifiInstance.mdns.update();

	// sensors may need some work
	for(auto eachSensor=Details.sensors.begin();eachSensor!=Details.sensors.end();eachSensor++)
	{
		(*eachSensor)->DoWork();
	}

	for(auto eachSwitch=Details.switches.begin();eachSwitch!=Details.switches.end();eachSwitch++)
	{
		(*eachSwitch)->DoWork();
	}

	// sump any debug from isr
	dblog.isr_pump();

	unsigned long now = micros() / 1000;

	if (!lastCheckedForPeers || ((now - lastCheckedForPeers) > _FETCH_PEERS_TIMEOUT_MS))
	{
		FindPeers();
		lastCheckedForPeers = now;
	}

#ifdef _TEST_WFI_STATE

	now = micros() / 1000;

	if (!lastTested || ((now - lastTested) > _TEST_WIFI_MILLIS))
	{
		WiFiMode_t currentState = WiFi.getMode();

		dblog.printf(debug::dbVerbose, "================ WIFI %d\n\r", currentState);

		WiFi.printDiag(Serial);

		lastTested = now;

	}

#endif

}

