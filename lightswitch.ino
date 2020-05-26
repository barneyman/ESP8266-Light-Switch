

#include <WiFiClient.h>
#include <debugLogger.h>
#include "switchSensor.h"

//#define _AT_RGBSTRIP					// a strip of RGBs



// 1mb 128k spiffs gives you ~ 500k for bin file for a sonoff
#define _OTA_AVAILABLE








// IMPORTANT - for programming SONOFF Basics
// https://github.com/arendst/Sonoff-Tasmota/wiki/Arduino-IDE
// Generic ESP8266 (beware, some 8255s out there!)
// Flashmode DOUT, 115200, ck
// FlashSize 1M 128k SPIFFS (any more and spiffs upload will fail)
// Upload Sketch AND Publish Server Files
// Connect the internal header up to an FTDI
// With it off, hold the button down and power it up, keep the button down for a second or two

// a number of exceptions in 2.4.0 & LWIP2 - currently only works reliably with 2.3.0 and LWIP1.4

// do not exceed 1Mb of spiffs in wemos (and maybe others) - you get assertions in DataSource.h




#include <ArduinoJson.h>
#include <FS.h>

#include <vector>
#include <algorithm> 

#ifdef _OTA_AVAILABLE
// 
#include <ESP8266httpUpdate.h>
volatile bool updateInProgress=false;
#endif

// passing strings from command line as defines, don't come thru as strings
// so we need to jump thru preprocessing hell to get what i want
#define VAL(x)	#x
#define TOSTRING(x)	VAL(x)

// this gets set by the github build action
#ifndef _VERSION_FRIENDLY_CLI


	#ifdef ARDUINO_ESP8266_GENERIC
	#define _VERSION_FRIENDLY	"sonoff_basic"
	#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI)
	#define _VERSION_FRIENDLY	"wemosD1"
	#else
	#define _VERSION_FRIENDLY	"unknown"
	#endif

#else

	#define _VERSION_FRIENDLY TOSTRING(_VERSION_FRIENDLY_CLI)

#endif

#ifndef _VERSION_NUM_CLI

	#define _VERSION_NUM "v99.99.99.pr"
//	#define _VERSION_NUM "v0.0.1.pr"
	#define _DEVELOPER_BUILD

#else

	#define _VERSION_NUM TOSTRING(_VERSION_NUM_CLI)

#endif

#define _MYVERSION			_VERSION_FRIENDLY "|" _VERSION_NUM

// set this to reset the file
//#define _ERASE_JSON_CONFIG
// has a leading underscore so we can spot it, and not serve it statically
#define _JSON_CONFIG_FILE "/_config.json"
#define _JSON_PRESERVED_STATE_FILE "/_state.json"
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



#define mdsnNAME	"barneyman"
myWifiClass wifiInstance("esp_", NULL, mdsnNAME);




// millis timeouts
#define QUICK_SWITCH_TIMEOUT_DEFAULT	6000
#define BOUNCE_TIMEOUT_DEFAULT			100


#define _ALLOW_WIFI_RESET_OVER_WIFI
#define _ALLOW_WIFI_RESET_VIA_QUICKSWITCH
#define _ALLOW_WIFI_RESET_AFTER_APIJOIN_TIMEOUT (2*60*1000)

#ifdef _ALLOW_WIFI_RESET_AFTER_APIJOIN_TIMEOUT
unsigned long runtimeWhenLastJoined=0;
#endif

#define _ALLOW_WIFI_RESET
#define _ALLOW_WIFI_RESET_OVER_WIFI

#ifdef _ALLOW_WIFI_RESET
bool resetWIFI = false;
#endif



#define CURRENT_SCHEMA_VER	2

// main config
struct 
{
	unsigned schemaVersion;
	// does this need saving
	bool configDirty;
	// wifi deets
	myWifiClass::wifiDetails wifi;
	// persisted - *USER* supplied
	// TODO - change this to device friendly name
	String friendlyName;

	// do we want prereleases
	bool prereleaseRequired;

	// do we only upgrade when the relay is off?
	bool upgradeOnlyWhenRelayOff;

	
	debug::dbLevel loggingLevel;
	debug::dbImpl loggingImpl;
	debugBaseClass *dblog;
	String loggingImplConfig;


	// sensors
	std::vector<baseSensor*>	sensors;

	// switches
	std::vector<baseSwitch*>	switches;

} Details=
{
	CURRENT_SCHEMA_VER,

	false,	// dirty

	// wifi deets
#ifdef _DEVELOPER_BUILD
	{
		"","",false, true
	},
#else	
	{
		"","",false, true
	},
#endif

	"Undefined",	// friendly name

	// prerel, upgradeWhileOff
	false, true,

	// logging
#ifdef _DEVELOPER_BUILD	
	debug::dbLevel::dbVerbose,
	debug::dbImpl::dbSerial,
#else
	debug::dbLevel::dbInfo,
	debug::dbImpl::dbNone,
#endif	
	NULL,
	""


};






// where we store the files we serve, statically
std::vector<std::pair<String,size_t>> servedFiles;
// our peers
std::vector<myWifiClass::mdnsService> services;






// how long we slow the web hits down for (millis)
#define _WEB_TAR_PIT_DELAY 20









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


// honour current switch state
void RevertAllSwitch()
{
	if(Details.dblog) Details.dblog->println(debug::dbInfo, "RevertAllSwitch");

	for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
	{
		(*each)->HonourCurrentSwitch();
		yield();
	}

}


// override switch state
void DoAllSwitch(bool state, bool force)
{

	if(Details.dblog) Details.dblog->printf(debug::dbInfo, "DoAllSwitch: %s %s\r\n", state ? "ON" : "off", force ? "FORCE" : "");

	for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
	{
		if(Details.dblog) Details.dblog->printf(debug::dbInfo, "Switch: %x\r\n", *each);
		(*each)->DoRelay(state, force);
		yield();
	}

	if(Details.dblog) Details.dblog->println(debug::dbInfo, "DoAllSwitch: Out");

}








void WriteJSONconfig()
{
	if(Details.dblog) Details.dblog->printf(debug::dbInfo, "WriteJSONconfig\n\r");

	// try to create it
	fs::File json = SPIFFS.open(_JSON_CONFIG_FILE, "w");

	if (!json)
	{
		if(Details.dblog) Details.dblog->printf(debug::dbError, "failed to create json\n\r");
		return;
	}

	//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
	jsonBuffer.clear();

	JsonObject &root = jsonBuffer.createObject();

#ifdef _AT_RGBSTRIP
	root["rgbCount"] = Details.rgbLedCount;
#endif


	root["schemaVersion"]=CURRENT_SCHEMA_VER;

	root["friendlyName"] = Details.friendlyName;

	root["prerelease"]=Details.prereleaseRequired;

	root["upgradeOnlyWhenRelayOff"]=Details.upgradeOnlyWhenRelayOff;

	root["loggingLevel"]=(int)Details.loggingLevel;
	root["loggingImpl"]=(int)Details.loggingImpl;
	root["loggingImplConfig"]=Details.loggingImplConfig;


	wifiInstance.WriteDetailsToJSON(root, Details.wifi);


	if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "jsonBuffer.size used : %d\n\r", jsonBuffer.size());

	///////////////////// written here

	String jsonText;
	root.printTo(jsonText);

	if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "JSON : -- %s --\n\r", jsonText.c_str());

	json.write((byte*)jsonText.c_str(), jsonText.length());

	if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "JSON : written\n\r");

	json.close();

	if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "JSON : closed\n\r");

	Details.configDirty = false;
}


void ReadJSONconfig()
{
	if(Details.dblog) Details.dblog->printf(debug::dbInfo, "ReadJSONconfig\n\r");

#ifdef _ERASE_JSON_CONFIG
	if(Details.dblog) Details.dblog->printf(debug::dbImportant, "erasing JSON file\n\r");
	SPIFFS.remove(_JSON_CONFIG_FILE);
#endif

	// check for legacy!
	if (SPIFFS.exists(_LEGACY_JSON_CONFIG_FILE))
	{
		if(Details.dblog) Details.dblog->println(debug::dbImportant, "Legacy config file found ... ");

		// check for co-existence!
		if (SPIFFS.exists(_JSON_CONFIG_FILE))
		{
			if(Details.dblog) Details.dblog->println(debug::dbImportant, "coexist ... deleting legacy");
			if (!SPIFFS.remove(_LEGACY_JSON_CONFIG_FILE))
			{
				if(Details.dblog) Details.dblog->println(debug::dbError, "delete failed ... bad");
				return;
			}
		}
		else
		{
			if(Details.dblog) Details.dblog->println(debug::dbImportant, "migration ... renaming");

			if (!SPIFFS.rename(_LEGACY_JSON_CONFIG_FILE, _JSON_CONFIG_FILE))
			{
				if(Details.dblog) Details.dblog->println(debug::dbError, "Rename failed ... bad");
				return;
			}
		}

	}

	if (SPIFFS.exists(_LEGACY_JSON_CONFIG_FILE))
	{
		if(Details.dblog) Details.dblog->println(debug::dbError, "Legacy File still exists ... bad");
	}


	if (!SPIFFS.exists(_JSON_CONFIG_FILE))
	{
		if(Details.dblog) Details.dblog->printf(debug::dbImportant, "'%s' does not exist\n\r", _JSON_CONFIG_FILE);
		// file does not exist
		WriteJSONconfig();

		return;

	}
	// first - see if the file is there
	fs::File json = SPIFFS.open(_JSON_CONFIG_FILE, "r");

	String jsonString = json.readString();

	json.close();

	if(Details.dblog) Details.dblog->printf(debug::dbInfo, "JSON: (%d) -- %s --\n\r", jsonString.length(), jsonString.c_str());

	//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
	jsonBuffer.clear();
	JsonObject& root = jsonBuffer.parseObject(jsonString);

	if (!root.success())
	{
		if(Details.dblog) Details.dblog->printf(debug::dbError, "JSON parse failed\n\r");

		// kill it - and write it again
		SPIFFS.remove(_JSON_CONFIG_FILE);

		if(Details.dblog) Details.dblog->printf(debug::dbInfo, "JSON file deleted\n\r");

		WriteJSONconfig();

		return;

	}
	else
	{
		if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "JSON parsed\n\r");
	}

	Details.configDirty = false;

#ifdef _AT_RGBSTRIP
	if(root.containsKey("rgbCount"))
		Details.rgbLedCount=root["rgbCount"];

	if(Details.dblog) Details.dblog->printf(debug::dbInfo,"Changing LED count to %d\n\r", Details.rgbLedCount);

	// tell the handler how big it is
	rgbHandler.Clear();
	rgbHandler.SetSize(Details.rgbLedCount);
	rgbHandler.DisplayAndWait(true);

#endif

	// check for schemaVersion
	if(!root.containsKey("schemaVersion") || root["schemaVersion"]<CURRENT_SCHEMA_VER)
	{
		if(Details.dblog) Details.dblog->printf(debug::dbError, "JSON parsed OLD file\n\r");

		// kill it - and write it again
		SPIFFS.remove(_JSON_CONFIG_FILE);

		if(Details.dblog) Details.dblog->printf(debug::dbInfo, "JSON file deleted\n\r");

		WriteJSONconfig();

		return;

	}


	if (root.containsKey("friendlyName"))
	{
		//String interim = root["friendlyName"].asString();
		String interim = root["friendlyName"].as<char*>();
		
		if (interim.length())
			Details.friendlyName = interim;
	}

	if(root.containsKey("prerelease"))
	{
		Details.prereleaseRequired=root["prerelease"];
	}
	else
	{
		Details.prereleaseRequired=false;
	}

	if(root.containsKey("upgradeOnlyWhenRelayOff"))
		Details.upgradeOnlyWhenRelayOff=root["upgradeOnlyWhenRelayOff"];
	else
		Details.upgradeOnlyWhenRelayOff=true;
	
	if(root.containsKey("loggingLevel"))
		Details.loggingLevel=(debug::dbLevel)root["loggingLevel"].as<int>();

	if(root.containsKey("loggingImpl"))
		Details.loggingImpl=(debug::dbImpl)root["loggingImpl"].as<int>();

	if(root.containsKey("loggingImplConfig"))
		Details.loggingImplConfig=root["loggingImplConfig"].as<char*>();
	else 
		Details.loggingImplConfig="";

	
	Details.dblog=NULL;


	wifiInstance.ReadDetailsFromJSON(root, Details.wifi);


}


void PreserveState()
{
	if(Details.dblog) Details.dblog->println(debug::dbImportant, "PreserveState");

	jsonBuffer.clear();
	JsonObject &root = jsonBuffer.createObject();
	JsonArray &stateArray=root.createNestedArray("states");
	root["count"]=Details.switches.size();

	// store the state of switches
	int count=0;
	for(auto each=Details.switches.begin();each!=Details.switches.end();each++, count++)
	{
		JsonObject &switchState = stateArray.createNestedObject();
		switchState["switch"] = count;
		// html js expects 1 or 0
		switchState["state"] = (*each)->GetRelay()?1:0;
	}

	fs::File json = SPIFFS.open(_JSON_PRESERVED_STATE_FILE, "w");

	String jsonText;
	root.printTo(jsonText);

	if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "JSON : -- %s --\n\r", jsonText.c_str());

	json.write((byte*)jsonText.c_str(), jsonText.length());

	json.close();

}

bool RestoreState()
{
	if(Details.dblog) Details.dblog->println(debug::dbVerbose, "RestoreState");

	// see if there is a preserved state
	if (SPIFFS.exists(_JSON_PRESERVED_STATE_FILE))
	{

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "PreserveState file exists");

		fs::File json = SPIFFS.open(_JSON_PRESERVED_STATE_FILE, "r");

		String jsonString = json.readString();

		json.close();

		jsonBuffer.clear();
		JsonObject& root = jsonBuffer.parseObject(jsonString);

		int maxcount=root["count"];

		for(int count=0;count<maxcount;count++)
		{
			int state=root["states"][count]["state"];
			int switchNum=root["states"][count]["switch"];

			if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "switch %d state %d\n\r",switchNum,state);

			if(switchNum<Details.switches.size())
			{
				Details.switches[switchNum]->SetRelay(state?true:false);
			}
		}

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "remove PreserveState file");
		SPIFFS.remove(_JSON_PRESERVED_STATE_FILE);

	}
	else
	{
		return false;
	}
	
	return true;
}



// void ResetWIFI()
// {
// 	DEBUG(DEBUG_IMPORTANT, Serial.println("Resetting WIFI"));

// 	wifiMode now = currentMode;

// 	ConnectWifi(wifiMode::modeOff);

// 	ConnectWifi(now);
// }

void RebootMe(bool preserve)
{
	if(Details.dblog) Details.dblog->printf(debug::dbImportant, "REBOOTING\n\r");
	delay(1000);
	if(preserve)
	{
		PreserveState();
	}

	delay(2000);

	ESP.restart();
}

#ifdef _ALLOW_WIFI_RESET

void ResetToAP()
{
	if(Details.dblog) Details.dblog->printf(debug::dbImportant, "Resetting\n\r");

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
	if(Details.dblog) Details.dblog->printf(debug::dbImportant, "AddAP\n\r");

	wifiInstance.ConnectWifi(myWifiClass::wifiMode::modeSTAandAP,Details.wifi);


}


#endif



void AddSwitch(baseSwitch *newSwitch)
{
	if(newSwitch)
	{
		if(Details.dblog) Details.dblog->println(debug::dbInfo, "Adding switch");
		Details.switches.push_back(newSwitch);

#ifndef ARDUINO_ESP8266_GENERIC

		MultiSwitch* multi=(MultiSwitch*)newSwitch;
		for(unsigned eachChild=0;eachChild<newSwitch->ChildSwitchCount();eachChild++)
		{
			Details.switches.push_back(multi->GetChild(eachChild));
		}

#endif		
		if(Details.dblog) Details.dblog->println(debug::dbInfo, "Added switch");
	}
}

void createLogger()
{
	// sort out the debugger
	switch(Details.loggingImpl)
	{
		case debug::dbImpl::dbNone:
			break;
		case debug::dbImpl::dbSerial:
			{
				SerialDebug*newOne=new SerialDebug(Details.loggingLevel);
				// Sonoff doesn't APPEAR to handle any faster
				newOne->begin(9600);
				Details.dblog=newOne;
			}
			break;
		case debug::dbImpl::dbSysLog:
			{
				//syslogDebug *newOne=new syslogDebug(debug::dbVerbose, "192.168.42.112", 514, "temp", _MYVERSION);
				syslogDebug *newOne=new syslogDebug(Details.loggingLevel,wifiInstance.m_hostName.c_str(),Details.loggingImplConfig);
				//syslogDebug *newOne=new syslogDebug(Details.loggingLevel,"wibble",Details.loggingImplConfig);
				newOne->SetAppName(_MYVERSION);
				Details.dblog=newOne;
			}
			break;
	}

	// tell wifi
	wifiInstance.SetDebug(Details.dblog);

}


void setup(void) 
{
#ifdef _DEBUG
	gdbstub_init();
#endif



#ifdef _DEVELOPER_BUILD
	// just sleep (to let the serial monitor attach)
	delay(10000);
#endif	

	SPIFFS.begin();
	ReadJSONconfig();

	createLogger();


	if(Details.dblog) Details.dblog->printf(debug::dbInfo, "\r\n\n\n===========================================");



	if(Details.dblog) Details.dblog->printf(debug::dbImportant, "Running %s\n\r", _MYVERSION);
	if(Details.dblog) Details.dblog->printf(debug::dbImportant, "Hostname %s\n\r", wifiInstance.m_hostName.c_str());


	if(Details.dblog) Details.dblog->println(debug::dbImportant, wifiInstance.m_hostName.c_str());

	enum myWifiClass::wifiMode intent = myWifiClass::wifiMode::modeUnknown;

	if (Details.wifi.configured)
	{
		if(Details.dblog) Details.dblog->println(debug::dbInfo, "wifi credentials found");
		if(Details.dblog) Details.dblog->println(debug::dbVerbose, Details.wifi.ssid);
		if(Details.dblog) Details.dblog->println(debug::dbVerbose, Details.wifi.password);
		intent = myWifiClass::wifiMode::modeSTA;
	}
	else
	{
		if(Details.dblog) Details.dblog->println(debug::dbWarning, "WiFi not configured");
		intent = myWifiClass::wifiMode::modeAP;

	}

#ifdef ARDUINO_ESP8266_GENERIC

	AddSwitch(new SonoffBasic(Details.dblog));

#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI)

#ifdef _PIR_VARIANT

	Details.sensors.push_back(new PIRInstantSensor(Details.dblog, D7));
	AddSwitch(new WemosRelayShield(Details.dblog));

#else

	Details.sensors.push_back(new BME280Sensor(Details.dblog));
	Details.sensors.push_back(new MAX44009Sensor(Details.dblog));


#endif // PIR


#endif

#ifdef _WIP

// thermos and lux
//#define WEMOS_COM3
// PIR
//#define WEMOS_COM4
// 6switch
#define WEMOS_COM5 

#ifdef WEMOS_COM3
	// load up the sensors and switches
	AddSwitch(new SonoffBasicNoLED(Details.dblog));


	// OF COURSE i reused D7 which is used by Sonoff! duh!
	Details.sensors.push_back(new DallasSingleSensor(D7, Details.dblog));
	Details.sensors.push_back(new BME280Sensor(Details.dblog));
	Details.sensors.push_back(new MAX44009Sensor(Details.dblog));

	Details.sensors.push_back(new testInstantSensor(Details.dblog));

#elif defined(WEMOS_COM4) 

	Details.sensors.push_back(new PIRInstantSensor(Details.dblog, D7));

#elif defined(WEMOS_COM5) 

	Details.sensors.push_back(new testInstantSensor(Details.dblog, 10*60*1000));
	AddSwitch(new MCP23017MultiSwitch(Details.dblog, 6, SDA, SCL, D5));


#endif

#endif // _WIP

	if(!RestoreState())
	{
		// wander thru all the switches, honouring
		RevertAllSwitch();
	}

	// try to connect to the wifi
	wifiInstance.ConnectWifi(intent, Details.wifi);


	// set up the callback handlers for the webserver
	InstallWebServerHandlers();




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
			if(Details.dblog) Details.dblog->println(debug::dbInfo, "No WIFI not doing FindPeers");
			// very bad form
			return;
		case myWifiClass::modeAP:
		case myWifiClass::modeSTA:
		case myWifiClass::modeSTAspeculative:
		case myWifiClass::modeSTAandAP:
		default:
			break;
	}

	if(Details.dblog) Details.dblog->printf(debug::dbInfo, "Looking for '%s' siblings ...\n\r", mdsnNAME);
	
	// get a list of what's out there
	services.clear();
	if (wifiInstance.QueryServices(mdsnNAME, services))
	{
		int found=(int)services.size();
		if(Details.dblog) Details.dblog->printf(debug::dbInfo, "Found %d sibling%c!!\n\r", found, found==1?' ':'s');
		for (auto iterator = services.begin(); iterator != services.end(); iterator++)
		{
			if(Details.dblog) Details.dblog->printf(debug::dbInfo, "\t%s @ %s\n\r", iterator->hostName.c_str(), iterator->IP.toString().c_str());
		}
		
	}
	else
	{
		if(Details.dblog) Details.dblog->println(debug::dbInfo, "No others services found");
	}

}

// set up all the handlers for the web server
void InstallWebServerHandlers()
{
	if(Details.dblog) Details.dblog->println(debug::dbVerbose, "InstallWebServerHandlers IN");

	// set up the json handlers
	// POST
	// all ON/OFF 
	// switch ON/OFF
	// revert

	// make all the relays reflect their switches
	wifiInstance.server.on("/revert", HTTP_POST, []() {

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/revert");

		RevertAllSwitch();

		//delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();
	});

	// do something to all of them
	wifiInstance.server.on("/all",HTTP_POST, []() {

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/all");

		for (uint8_t i = 0; i < wifiInstance.server.args(); i++)
		{
			if (wifiInstance.server.argName(i) == "action")
			{
				DoAllSwitch(wifiInstance.server.arg(i) == "on" ? true : false, true);
			}
		}

		//delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();
	});


#ifdef _OTA_AVAILABLE

	// do an OTA update from a provided URL
	wifiInstance.server.on("/json/upgrade", HTTP_POST, []() {

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "json upgrade posted");
		if(Details.dblog) Details.dblog->println(debug::dbImportant, wifiInstance.server.arg("plain"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(wifiInstance.server.arg("plain"));

		if(Details.upgradeOnlyWhenRelayOff)
		{
			for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
			{
				if((*each)->GetRelay())
				{
					// it's on ... bounce
					if(Details.dblog) Details.dblog->println(debug::dbImportant, "ignoring upgrade because the relay is ON");
					wifiInstance.server.send(403, "application/json", "{ reason: 'not while turned on' }");
					return;
				}
			}
		}

/*
		// Legacy data was this

		String host = root["host"];
		int port = root["port"];
		String url= root["url"];		

*/
		updateInProgress=true;

		String url= root["url"];
		String urlSpiffs= root["urlSpiffs"];

		//delay(_WEB_TAR_PIT_DELAY);

		StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer2;
		JsonObject&replyroot = jsonBuffer2.createObject();
		String bodyText;
		enum HTTPUpdateResult result=HTTP_UPDATE_OK;

		for(int updates=0;(updates<2) && (result==HTTP_UPDATE_OK);updates++)
		{

			// augment the url with a prerelease parameter
			String urlArgs="?prerelease="+String((Details.prereleaseRequired?"true":"false"));
			if(Details.dblog) Details.dblog->println(debug::dbInfo,urlArgs.c_str());


			// lets check for SPIFFs update first
			
			if(!updates)
			{
				if(Details.dblog) Details.dblog->println(debug::dbImportant, "updating SPIFFS ...");
				result=ESPhttpUpdate.updateSpiffs(wifiInstance.m_wificlient ,urlSpiffs+urlArgs,_MYVERSION);
			}
			else
			{
				if(Details.dblog) Details.dblog->println(debug::dbImportant, "updating BIN ...");
				// we do some clean up so let me boot
				ESPhttpUpdate.rebootOnUpdate(false);
				result=ESPhttpUpdate.update(url+urlArgs, _MYVERSION);
			}

			switch (result)
			{
			case HTTP_UPDATE_FAILED:
				if(Details.dblog) Details.dblog->printf(debug::dbError, "HTTP_UPDATE_FAILED Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
				break;
			case HTTP_UPDATE_NO_UPDATES:
				if(Details.dblog) Details.dblog->println(debug::dbImportant, "no updates");
				break;
			case HTTP_UPDATE_OK:
				if(Details.dblog) Details.dblog->println(debug::dbImportant, "update succeeded");
				break;
			}

			JsonObject &thisLoop=replyroot.createNestedObject((!updates)?"SPIFFS":"BIN");

			thisLoop["result"]=result;

			if (result != HTTP_UPDATE_OK)
			{
				JsonObject &details = thisLoop.createNestedObject("Details");
				details["espNarrative"] = ESPhttpUpdate.getLastErrorString();
				details["espResult"] = ESPhttpUpdate.getLastError();

			}

			bodyText=String();
			replyroot.printTo(bodyText);

			if (result != HTTP_UPDATE_OK)
			{
				if(Details.dblog) Details.dblog->println(debug::dbError, bodyText);
				break;
			}
			else
			{
				if(Details.dblog) Details.dblog->println(debug::dbImportant, bodyText);
			}

			// first time round, save our config
			if(!updates && (result==HTTP_UPDATE_OK))
			{
				if(Details.dblog) Details.dblog->println(debug::dbImportant, "preserving config");
				WriteJSONconfig();
			}

		}

		updateInProgress=false;

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "saving switch state\n\r");

		// no point - core killed the IP stack during the update
		wifiInstance.server.send(200, "application/json", bodyText);

		// preserve, reboot, if it worked
		if(result==HTTP_UPDATE_OK)
		{
			RebootMe(true);
		}

	});

#endif

	// inverse of whatever it's currently doing
	wifiInstance.server.on("/toggle",HTTP_POST, []() {

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/toggle");

		//delay(_WEB_TAR_PIT_DELAY);


		// TODO pick the right one
		// just go thru them all
		for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
			(*each)->ToggleRelay();



	});

	wifiInstance.server.on("/button",HTTP_POST, []() {

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "/button");

		for (int count = 0; count < wifiInstance.server.args(); count++)
		{
			if(Details.dblog) Details.dblog->printf(debug::dbInfo, "%d. %s = %s \n\r", 
				count+1, 
				wifiInstance.server.argName(count).c_str(), 
				wifiInstance.server.arg(count).c_str()
			);
		}



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
					if(Details.dblog) Details.dblog->printf(debug::dbWarning, "asked to action %d - exceeds maximum %d\r\n", port,Details.switches.size()-1);
				}
			}
			else
			{
				// just go thru them all
				for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
					(*each)->DoRelay(action);
			}
			

		}

		//delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();

	});

	// LEGACY - remove when the beach house is upgraded past 0.0.32 and home assistant does POSTS
	wifiInstance.server.on("/button", []() {

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "/button");

		for (int count = 0; count < wifiInstance.server.args(); count++)
		{
			if(Details.dblog) Details.dblog->printf(debug::dbInfo, "%d. %s = %s \n\r", 
				count+1, 
				wifiInstance.server.argName(count).c_str(), 
				wifiInstance.server.arg(count).c_str()
			);
		}



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
					if(Details.dblog) Details.dblog->printf(debug::dbWarning, "asked to action %d - exceeds maximum %d\r\n", port,Details.switches.size()-1);
				}
			}
			else
			{
				// just go thru them all
				for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
					(*each)->DoRelay(action);
			}
			

		}

		//delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();

	});




	wifiInstance.server.on("/json/logging",HTTP_POST, []() {

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/json/logging");
		if(Details.dblog) Details.dblog->println(debug::dbImportant, wifiInstance.server.arg("plain"));

		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(wifiInstance.server.arg("plain"));

		bool reboot=false;

		if(root.containsKey("level"))
		{
			Details.loggingLevel=(debug::dbLevel)root["level"].as<int>();
			if(Details.dblog)
				Details.dblog->m_currentLevel=Details.loggingLevel;
		}

		if(root.containsKey("impl"))
		{
			Details.loggingImpl=(debug::dbImpl)root["impl"].as<int>();
			reboot=true;
		}

		if(root.containsKey("config"))
		{
			Details.loggingImplConfig.clear();
			root["config"].printTo(Details.loggingImplConfig);
		}

		WriteJSONconfig();

		wifiInstance.server.send(200,"text/html","<html/>");

		// minimise heap fracture
		if(reboot)
			RebootMe(true);

	});







	wifiInstance.server.on("/resetCounts",HTTP_POST, []() {

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/resetCounts");

		//delay(_WEB_TAR_PIT_DELAY);

		for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
			(*each)->ResetTransitionCount();

		wifiInstance.server.send(200,"text/html","<html/>");

	});

#ifdef _DEVELOPER_BUILD

	wifiInstance.server.on("/reboot",HTTP_POST, []() {

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/reboot");

		RebootMe(true);

	});

#endif

#ifdef _ALLOW_WIFI_RESET_OVER_WIFI

	wifiInstance.server.on("/resetWIFI",HTTP_POST, []() {

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/resetWIFI");

		ResetToAP();

	});

#endif

	wifiInstance.server.on("/stopAP",HTTP_POST, []() {

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/stopAP");

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

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "/ requested");

		//delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();

	});

	wifiInstance.server.on("/default.htm", []() {

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "/default.htm");

		//delay(_WEB_TAR_PIT_DELAY);

		SendServerPage();

	});

	// posted config
	wifiInstance.server.on("/json/config", HTTP_POST, []() {

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "json config posted");
		if(Details.dblog) Details.dblog->println(debug::dbImportant, wifiInstance.server.arg("plain"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(wifiInstance.server.arg("plain"));


		if (root.containsKey("friendlyName"))
		{
			Details.friendlyName = root["friendlyName"].as<char*>();
		}

		if (root.containsKey("prerelease"))
		{
			Details.prereleaseRequired = root["prerelease"]?true:false;
		}

		if(root.containsKey("upgradeOnlyWhenRelayOff"))
			Details.upgradeOnlyWhenRelayOff=root["upgradeOnlyWhenRelayOff"]?true:false;


#ifdef _AT_RGBSTRIP

		if (root.containsKey("ledCount"))
		{
			Details.rgbLedCount = root["ledCount"];
			if(Details.dblog) Details.dblog->printf(debug::dbImportant, "Changing LED count to %d\n\r", Details.rgbLedCount);
			rgbHandler.Clear();
			rgbHandler.SetSize(Details.rgbLedCount);
			rgbHandler.DisplayAndWait(true);
		}

#endif


		

		// extract the details
		WriteJSONconfig();
		//delay(_WEB_TAR_PIT_DELAY);

		wifiInstance.server.send(200, "text/html", "<html></html>");

		});


	wifiInstance.server.on("/json/listen", HTTP_POST, []() {

		IPAddress recipientAddr = wifiInstance.server.client().remoteIP();

		if(Details.dblog) Details.dblog->printf(debug::dbInfo, "json listen posted from %s\n\r",recipientAddr.toString().c_str());
		if(Details.dblog) Details.dblog->println(debug::dbInfo, wifiInstance.server.arg("plain"));

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
				if(Details.dblog) Details.dblog->printf(debug::dbInfo, "Adding recipient %s:%d Sensor %d\n\r", recipientAddr.toString().c_str(), recipientPort, recipientSensor);
				Details.sensors[recipientSensor]->AddAnnounceRecipient(recipientAddr,recipientPort,wifiInstance.server.arg("plain"));
			}
			else
			{
				if(Details.dblog) Details.dblog->println(debug::dbError, "Sensor exceeded bounds");
			}

		}
		else if (root.containsKey("switch"))
		{
			int recipientSwitch = root["switch"];

			if(recipientSwitch<Details.switches.size())
			{
				if(Details.dblog) Details.dblog->printf(debug::dbInfo, "Adding recipient %s:%d Switch %d\n\r", recipientAddr.toString().c_str(), recipientPort, recipientSwitch);
				Details.switches[recipientSwitch]->AddAnnounceRecipient(recipientAddr,recipientPort,wifiInstance.server.arg("plain"));


			}
			else
			{
				if(Details.dblog) Details.dblog->println(debug::dbError, "Switch exceeded bounds");
			}

		}

		//delay(_WEB_TAR_PIT_DELAY);
		wifiInstance.server.send(200, "text/html", "<html></html>");


	});





	wifiInstance.server.on("/json/wifi", HTTP_POST, []() {

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "json wifi posted");
		if(Details.dblog) Details.dblog->println(debug::dbImportant, wifiInstance.server.arg("plain"));

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = jsonBuffer.parseObject(wifiInstance.server.arg("plain"));

		String ssid = root["ssid"];
		String pwd = root["pwd"];
		String friendly = root["friendlyName"];

		// sanity check these values

		Details.wifi.ssid = ssid;
		Details.wifi.password = pwd;
		Details.friendlyName=friendly;

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

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "json state called");

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
		root.printTo(jsonText);

		if(Details.dblog) Details.dblog->println(debug::dbVerbose, jsonText);

		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
	});



	wifiInstance.server.on("/json/logging", HTTP_GET, []() {

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "json config called");

		StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBufferConfig;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["impl"]=(int)Details.loggingImpl;
		root["level"]=(int)Details.loggingLevel;

		JsonArray &impls=root.createNestedArray("impls");
		for(int eachLevel=(int)debug::dbNone;eachLevel<(int)debug::dbUnknown;eachLevel++)
		{
			JsonObject &thisOne=impls.createNestedObject();
			thisOne["value"]=eachLevel;
			
			JsonVariant configJSON;

			switch(eachLevel)
			{
				case debug::dbNone:
					thisOne["name"]="None";
					configJSON=jsonBufferConfig.parse(NullDebug::getConfigOptionsJSON());
					thisOne["config"]=configJSON;
					break;
				case debug::dbSerial:
					thisOne["name"]="Serial";
					configJSON=jsonBufferConfig.parse(SerialDebug::getConfigOptionsJSON());
					thisOne["config"]=configJSON;
					break;
				case debug::dbSysLog:
					thisOne["name"]="SysLog";
					configJSON=jsonBufferConfig.parse(syslogDebug::getConfigOptionsJSON());
					thisOne["config"]=configJSON;
					break;
				default:
					break;
			}

		}

		JsonArray &levels=root.createNestedArray("levels");
		for(int eachLevel=(int)debug::dbVerbose;eachLevel<(int)debug::dbAlways;eachLevel++)
		{
			JsonObject &thisOne=levels.createNestedObject();
			thisOne["value"]=eachLevel;
			switch(eachLevel)
			{
				case debug::dbVerbose:
					thisOne["name"]="Verbose";
					break;
				case debug::dbInfo:
					thisOne["name"]="Info";
					break;
				case debug::dbImportant:
					thisOne["name"]="Important";
					break;
				case debug::dbWarning:
					thisOne["name"]="Warning";
					break;
				case debug::dbError:
					thisOne["name"]="Error";
					break;
				default:
					break;
			}
		}

		String jsonText;
		root.printTo(jsonText);

		if(Details.dblog) Details.dblog->println(debug::dbVerbose, jsonText);

		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);




	});



	wifiInstance.server.on("/json/config", HTTP_GET, []() {
		// give them back the port / switch map

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "json config called");

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = wifiInstance.m_hostName.c_str();
		root["version"] = _MYVERSION;
		if(wifiInstance.localIP().isSet())
			root["ip"] = wifiInstance.localIP().toString();
		root["mac"] = wifiInstance.macAddress();

#ifdef _DEVELOPER_BUILD
		root["developer"] = 1;
#endif


		root["friendlyName"] = Details.friendlyName;
		root["prerelease"]=Details.prereleaseRequired?1:0;
		root["upgradeOnlyWhenRelayOff"]=Details.upgradeOnlyWhenRelayOff?1:0;

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

			switchRelay["name"] = (*each)->GetName();
		}

		String jsonText;
		root.printTo(jsonText);

		if(Details.dblog) Details.dblog->println(debug::dbVerbose, jsonText);

		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
	});

	wifiInstance.server.on("/json/wificonfig", HTTP_GET, []() {
		// give them back the port / switch map

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "json wificonfig called");

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = wifiInstance.m_hostName.c_str();
		root["ssid"] = wifiInstance.SSID();
		if(wifiInstance.localIP().isSet())
			root["ip"] = wifiInstance.localIP().toString();

		root["friendlyName"]=Details.friendlyName;

		String jsonText;
		root.printTo(jsonText);

		if(Details.dblog) Details.dblog->println(debug::dbVerbose, jsonText);

		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
	});

	///json/peers
	wifiInstance.server.on("/json/peers", HTTP_GET, []() {
		// give them back the port / switch map

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "json peers called");

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

			if(Details.dblog) Details.dblog->printf(debug::dbInfo, "%d '%s' %s\n\r", each + 1, services[each].hostName.c_str(), services[each].IP.toString().c_str());
			peer["name"]=services[each].hostName;
			peer["ip"]=services[each].IP.toString();
		}


		String jsonText;
		root.printTo(jsonText);

		if(Details.dblog) Details.dblog->println(debug::dbVerbose, jsonText);

		// do not cache
		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "text/json", jsonText);
	});




	wifiInstance.server.on("/json/wifi", HTTP_GET, []() {
		// give them back the port / switch map

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "json wifi called");

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();
		root["name"] = wifiInstance.m_hostName.c_str();
		root["friendlyName"] = Details.friendlyName;

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

			if(Details.dblog) Details.dblog->printf(debug::dbInfo, "%d '%s' %d \n\r", each + 1, allWifis[each].first.c_str(), allWifis[each].second);

		}
		

		String jsonText;
		root.printTo(jsonText);

		if(Details.dblog) Details.dblog->println(debug::dbVerbose, jsonText);

		// do not cache
		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "text/json", jsonText);
	});

	// serve up everthing in SPIFFS
	SPIFFS.openDir("/");

	
	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {
		String file = dir.fileName();

#ifndef _DEVELOPER_BUILD
		// ensure it doesn't have a leading underscore - hidden flag for me
		if (file.length() > 1 && file[1] == '_')
		{
			if(Details.dblog) Details.dblog->printf(debug::dbInfo, "Skipping %s\n\r", file.c_str());
			continue;
		}
#endif

		// cache it for an hour
		wifiInstance.server.serveStatic(file.c_str(), SPIFFS, file.c_str(),"Cache-Control: public, max-age=60");

		if(Details.dblog) Details.dblog->printf(debug::dbInfo, "Serving %s\r\n", file.c_str());

		// remove the slash
		// file.remove(0,1);
		servedFiles.push_back(std::pair<String,size_t>(file, dir.fileSize()));
	}

	if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "InstallWebServerHandlers OUT\n\r");

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
		if(Details.dblog) Details.dblog->printf(debug::dbError,"SPIFFS error - %s does not exist\n\r", toOpen.c_str());
	}

}


#ifdef _TEST_WFI_STATE
unsigned long lastTested = 0;
#define _TEST_WIFI_MILLIS	(15*60*1000)
#endif

#define _FETCH_PEERS_TIMEOUT_MS	(15*60*1000)	// 15 mins
//#define _FETCH_PEERS_TIMEOUT_MS	(1*60*1000)	// 1 min
unsigned long lastCheckedForPeers = 0;

void loop(void) 
{

	if(updateInProgress)
		return;

#ifdef _ALLOW_WIFI_RESET
	if (resetWIFI)
		ResetToAP();
#endif

#ifdef _ALLOW_WIFI_RESET_AFTER_APIJOIN_TIMEOUT

	if(wifiInstance.currentMode!=myWifiClass::modeSTA_unjoined)
	{
		runtimeWhenLastJoined=millis();
	}

	if(millis()-runtimeWhenLastJoined > _ALLOW_WIFI_RESET_AFTER_APIJOIN_TIMEOUT)
	{
		AddAP();
		runtimeWhenLastJoined=millis();
	}
#endif

	if (Details.configDirty)
		WriteJSONconfig();


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
	if(Details.dblog) Details.dblog->isr_pump();

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

		if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "================ WIFI %d\n\r", currentState);

		WiFi.printDiag(Serial);

		lastTested = now;

	}

#endif

}

