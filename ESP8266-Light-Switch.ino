

#include <WiFiClient.h>
#include <debugLogger.h>
#include "switchSensor.h"
#include <tuple>

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

// wemos = LOLIN(WEMONS) D1 R2 & mini


#include <ArduinoJson.h>
#include <FS.h>

#include <vector>
#include <algorithm> 

#ifdef _OTA_AVAILABLE
// 
#ifdef ARDUINO_ARCH_ESP32
	#include <HTTPUpdate.h>
	#include <SPIFFS.h>
#else
	#include <ESP8266httpUpdate.h>
#endif	
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
	#elif defined(ESP32)
	#define _VERSION_FRIENDLY	"esp32dev"
	#else
	#define _VERSION_FRIENDLY	"unknown"
	#endif

// devhack platform options
//#define PLATFORM_SONOFF_SWITCH
//#define PLATFORM_WEMOS_SENSOR
//#define PLATFORM_ESP32_CAMERA

#else

	#define _VERSION_FRIENDLY TOSTRING(_VERSION_FRIENDLY_CLI)


#endif

#if !defined(PLATFORM_SONOFF_SWITCH) && !defined(PLATFORM_WEMOS_SENSOR) && !defined(PLATFORM_ESP32_CAMERA)
	// let's sort out our platform
	#ifdef ARDUINO_ESP8266_GENERIC
		#define PLATFORM_SONOFF_SWITCH
	#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI)
		#define PLATFORM_WEMOS_SENSOR
	#elif defined(ESP32)
		#define PLATFORM_ESP32_CAMERA
	#endif
#endif

#ifdef PLATFORM_SONOFF_SWITCH
#pragma message "SONOFF BUILD"
#elif defined(PLATFORM_WEMOS_SENSOR)
#pragma message "WEMOS BUILD"
#elif defined(PLATFORM_ESP32_CAMERA)
#pragma message "ESPCAM BUILD"
#else
#error "Platform not defined"
#endif


// similarly, set by the github action
#ifndef _VERSION_NUM_CLI

//	#define _VERSION_NUM "v99.99.99.pr"
	#define _VERSION_NUM "v0.1.0.pr"
	#define _DEVELOPER_BUILD

#else

	#define _VERSION_NUM TOSTRING(_VERSION_NUM_CLI)

#endif

#ifdef _DEVELOPER_BUILD
#pragma message "Developer Build"
#endif


#ifdef PLATFORM_ESP32_CAMERA
	#include "cameraSensor.h"
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

#ifdef _ERASE_JSON_CONFIG
#pragma message "Erasing Config!"
#endif

// my libs
#include <myWifi.h>

#define mdsnNAME		"barneyman"

#ifdef ESP32
#define hostnameStem	"esp32_"
#else
#define hostnameStem	"esp_"
#endif
myWifiClass wifiInstance(hostnameStem, NULL, mdsnNAME);






#define _ALLOW_WIFI_RESET_OVER_WIFI
#define _ALLOW_WIFI_RESET_VIA_QUICKSWITCH
#define _ALLOW_WIFI_RESET

#ifdef _ALLOW_WIFI_RESET_VIA_QUICKSWITCH
	// millis timeouts
	#define QUICK_SWITCH_TIMEOUT_DEFAULT	6000
#endif



// if joining an AP fails, fall back to STA
#ifdef _DEVELOPER_BUILD
	#define _ALLOW_WIFI_RESET_AFTER_AP_JOIN_TIMEOUT (30*1000)
#else
	#define _ALLOW_WIFI_RESET_AFTER_AP_JOIN_TIMEOUT (15*60*1000)
#endif	

// remember the files we are serving statically
//#define _STORE_STATIC_FILES


#if !defined(PLATFORM_SONOFF_SWITCH)

class baseConfigurator
{
public:

	baseConfigurator(debugBaseClass*dbg, unsigned id, const char*bigname):m_dbg(dbg),m_id(id)
	{
		m_bigName=bigname;
	}

	virtual void getConfigOptionsJSON(JsonObject &tohere)=0;

	virtual baseSensor *createOne(const char*config)=0;

	bool operator ==(const unsigned &id) const
	{
		return id==m_id;
	}

	const char *Name() { return m_bigName.c_str(); }


protected:

	debugBaseClass *m_dbg;
	unsigned m_id;
	String m_bigName;

};


template <class T>
class configurator : public baseConfigurator
{
public:

	configurator(debugBaseClass*dbg, unsigned id, const char*bigname):baseConfigurator(dbg,id, bigname)
	{
	}

	// proxy for the real object
	virtual void getConfigOptionsJSON(JsonObject &tohere)
	{
		tohere["name"]=m_bigName;
		tohere["id"]=m_id;

		if(m_dbg)
			m_dbg->printf(debug::dbError,"getConfigOptionsJSON %s, %u\n\r",m_bigName.c_str(),m_id );

		T::getConfigOptionsJSON(tohere);
	}

	virtual baseSensor *createOne(const char*config)
	{
		baseSensor *tmp=new T(m_dbg,config);//PIRInstantSensor(m_dbg,config);
		return tmp;
	}



protected:


};





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

	// update stuff
	bool updateAvailable;
	String url, urlSpiffs;

	// wifi joins / stops
	bool wifiChangeRequested;
	myWifiClass::wifiMode desiredMode;

	// transient
#ifdef _ALLOW_WIFI_RESET_AFTER_AP_JOIN_TIMEOUT
	unsigned long runtimeWhenLastJoined;
#endif
#if defined( _ALLOW_WIFI_RESET ) || defined( _ALLOW_WIFI_RESET_OVER_WIFI )
	bool resetWIFI;
#endif


	// marker - everything after this is dynamic
	bool ignoreThis;

#if !defined(PLATFORM_SONOFF_SWITCH)
	std::vector<baseConfigurator*> options;
	std::vector<std::tuple<unsigned,String,baseSensor*>> sensors;
	// std::get<offset>()
#define GETSENSOR(a) std::get<2>(a)
#else	
	// sensors
	std::vector<baseSensor*>	sensors;
#define GETSENSOR(a) (a)
#endif	


	// switches
	std::vector<baseSwitch*>	switches;


#ifdef PLATFORM_ESP32_CAMERA
	std::vector<espCamera*>	cameras;
#endif		

#ifdef _STORE_STATIC_FILES
	// where we store the files we serve, statically
	std::vector<std::pair<String,size_t>> servedFiles;
#endif	
	// our peers
	std::vector<myWifiClass::mdnsService> services;


} Details=
{
	CURRENT_SCHEMA_VER,

	false,	// dirty

	// wifi deets
#ifdef _DEVELOPER_BUILD
	{
		// name pwd config dhcp
		"","",false, true
	},
#else	
	{
		"","",false, true
	},
#endif

	"",	// friendly name

	// prerel, upgradeWhileOff
	false, true,

	// logging
#ifdef _DEVELOPER_BUILD	
	debug::dbLevel::dbImportant,
	debug::dbImpl::dbSerial,
#else
	debug::dbLevel::dbImportant,
	debug::dbImpl::dbNone,
#endif	
	NULL,
	"",

	// update
	false,"","",

	// wifi change
	false, myWifiClass::wifiMode::modeAP,


	// transient
#ifdef _ALLOW_WIFI_RESET_AFTER_AP_JOIN_TIMEOUT
	0,
#endif
#if defined( _ALLOW_WIFI_RESET ) || defined( _ALLOW_WIFI_RESET_OVER_WIFI )
	false,
#endif

	false // just a marker for comma handling

};





















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


#ifdef PLATFORM_WEMOS_SENSOR
	// write instances

	JsonArray &instances=root.createNestedArray("instances");

	for(auto each=Details.sensors.begin();each!=Details.sensors.end();each++)
	{
		JsonObject &thisInstance=instances.createNestedObject();
		thisInstance["id"]=std::get<0>(*each);
		thisInstance["config"]=std::get<1>(*each);
	}

#endif


	wifiInstance.WriteDetailsToJSON(root, Details.wifi);


	if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "jsonBuffer.size used : %d\n\r", jsonBuffer.size());

	///////////////////// written here

	String jsonText;
#ifdef _DEVELOPER_BUILD
	root.prettyPrintTo(jsonText);
#else		
	root.printTo(jsonText);
#endif		

	if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "JSON : -- %s --\n\r", jsonText.c_str());

	json.write((byte*)jsonText.c_str(), jsonText.length());

	if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "JSON : written\n\r");

	json.close();

	if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "JSON : closed\n\r");

	Details.configDirty = false;

	yield();
}


void ReadJSONconfig()
{
	if(Details.dblog) Details.dblog->printf(debug::dbInfo, "ReadJSONconfig\n\r");

#ifdef _ERASE_JSON_CONFIG
	if(Details.dblog) Details.dblog->printf(debug::dbImportant, "erasing JSON file\n\r");
	if(!SPIFFS.remove(_JSON_CONFIG_FILE))
	{
		if(Details.dblog) Details.dblog->printf(debug::dbError, "Failed to erase JSON file\n\r");
	}
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

#ifdef PLATFORM_WEMOS_SENSOR
	// read instances
	if(root.containsKey("instances"))
	{
		JsonArray &instances=root["instances"];
		// for each one
		for(auto each=instances.begin();each!=instances.end();++each)
		{
			Details.sensors.push_back(std::tuple<unsigned,String,baseSensor*>(each->as<JsonObject>().get<unsigned>("id"),each->as<JsonObject>().get<const char*>("config"),NULL));
		}
	}

#ifdef _KILL_INSTANCES
	Details.sensors.clear();
	Details.dirty=true;

#endif

#endif	

	Details.dblog=NULL;


	wifiInstance.ReadDetailsFromJSON(root, Details.wifi);

	yield();
}

#if !defined(PLATFORM_SONOFF_SWITCH)

bool AddDeviceInstance()
{
	if(!Details.sensors.size())
	{
		if(Details.dblog) Details.dblog->println(debug::dbWarning, "No available devices/sensors");
		return false;
	}
	
	if(Details.dblog) Details.dblog->println(debug::dbVerbose, "Adding instances");

	for(auto eachInstance=Details.sensors.begin();eachInstance!=Details.sensors.end();eachInstance++)
	{
		// this gets called by add to only new if it's not already there
		if(std::get<2>(*eachInstance))
			continue;

		// walk thru
		for(auto each=Details.options.begin();each!=Details.options.end();each++)
		{
			yield();
			if(*(*each)==std::get<0>(*eachInstance))
			{
				if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "creating '%s' instance\n\r", (*each)->Name());
				// found it ... create one
				baseSensor *newOne=(*each)->createOne(std::get<1>(*eachInstance).c_str());
				std::get<2>(*eachInstance)=newOne;
				break;
			}

		}
	}

	return true;

}

#endif	

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
#ifdef _DEVELOPER_BUILD
	root.prettyPrintTo(jsonText);
#else		
	root.printTo(jsonText);
#endif		

	if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "JSON : -- %s --\n\r", jsonText.c_str());

	json.write((byte*)jsonText.c_str(), jsonText.length());

	json.close();

}

bool RestoreState()
{
	if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "RestoreState ");

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
		if(Details.dblog) Details.dblog->println(debug::dbVerbose, " ignored");
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
	if(Details.dblog) Details.dblog->printf(debug::dbImportant, "Resetting wifi to AP\n\r");

	Details.resetWIFI = false;
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
void AddAPtoSTA()
{
	if(Details.dblog) Details.dblog->printf(debug::dbImportant, "AddAPtoSTA\n\r");

	wifiInstance.ConnectWifi(myWifiClass::wifiMode::modeSTAandAP,Details.wifi);


}


#endif

#ifdef PLATFORM_ESP32_CAMERA

void AddCamera(espCamera *newCamera)
{
	if(Details.dblog) Details.dblog->println(debug::dbInfo, "Adding camera");

	if(newCamera->InitialisedOk())
	{
		Details.cameras.push_back(newCamera);
	}
	else
	{
		if(Details.dblog) Details.dblog->println(debug::dbInfo, "camera invalid");
		delete newCamera;
	}
}

#endif


void AddSwitch(baseSwitch *newSwitch)
{
	if(newSwitch)
	{
		if(Details.dblog) Details.dblog->println(debug::dbInfo, "Adding switch");
		Details.switches.push_back(newSwitch);

#ifndef PLATFORM_SONOFF_SWITCH

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
#ifdef _DEVELOPER_BUILD	
	Details.loggingImpl=debug::dbImpl::dbSerial;
#endif	

	// sort out the debugger
	switch(Details.loggingImpl)
	{
		case debug::dbImpl::dbNone:
			break;
		case debug::dbImpl::dbSerial:
			{
				SerialDebug*newOne=new SerialDebug(Details.loggingLevel);
				// Sonoff doesn't APPEAR to handle any faster
#ifdef PLATFORM_SONOFF_SWITCH
				newOne->begin(57600);
#else
				newOne->begin(115200);
#endif				
				Details.dblog=newOne;
			}
			break;
		case debug::dbImpl::dbSysLog:
			{
				syslogDebug *newOne=new syslogDebug(Details.loggingLevel,wifiInstance.m_hostName.c_str(),Details.loggingImplConfig);
				//syslogDebug *newOne=new syslogDebug(Details.loggingLevel,"wibble",IPAddress(129,168,42,112));
				newOne->SetAppName(_MYVERSION);
				Details.dblog=newOne;
			}
			break;
	}

	// tell wifi
	wifiInstance.SetDebug(Details.dblog);

}

void loadOptions()
{
#ifdef PLATFORM_WEMOS_SENSOR	

	// these IDs *must not change*
	Details.options.push_back(new configurator<DallasSingleSensor>(Details.dblog, 0x100, "Dallas Temperature Sensor"));
	Details.options.push_back(new configurator<BME280Sensor>(Details.dblog, 0x101, "BME280 Environment Sensor"));
	Details.options.push_back(new configurator<MAX44009Sensor>(Details.dblog, 0x102, "MAX44009 Lux Sensor"));
	Details.options.push_back(new configurator<PIRInstantSensor>(Details.dblog, 0x103, "HC-SR501 PIR Sensor"));

#endif	
}


void setup(void) 
{

#ifdef _DEVELOPER_BUILD
	// just sleep (to let the serial monitor attach)
	delay(10000);
#endif	

	SPIFFS.begin();

	ReadJSONconfig();

	createLogger();
	// ONLY AFTER HERE WILL THERE BE LOGGING!
	// createLogger nulls dblog, so be aware of that


	// add the options
	loadOptions();

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


#ifdef PLATFORM_SONOFF_SWITCH

	AddSwitch(new SonoffBasic(Details.dblog));

#elif defined(PLATFORM_WEMOS_SENSOR)

	// create from the stored details
	if(!AddDeviceInstance())
	{
	}



#elif defined(PLATFORM_ESP32_CAMERA)

#define _USE_FAKE_CAMERAS
#ifdef _USE_FAKE_CAMERAS
	AddCamera(new fakeCamera(Details.dblog));
	AddCamera(new fakeCamera(Details.dblog));
	AddCamera(new fakeCamera(Details.dblog));
	AddCamera(new fakeCamera(Details.dblog));
	AddCamera(new fakeCamera(Details.dblog));
	AddCamera(new fakeCamera(Details.dblog));
#else
	AddCamera(new esp32Cam(Details.dblog));
#endif

#endif


	if(!RestoreState())
	{
		// wander thru all the switches, honouring
		RevertAllSwitch();
	}

	// try to connect to the wifi
	wifiInstance.ConnectWifi(intent, Details.wifi);


	// set up the callback handlers for the webserver
	InstallWebServerHandlers(true);

	// reset the clock
	Details.runtimeWhenLastJoined=millis();



}



// look for my siblings
void FindPeers()
{
	bool doFind=true;
	// if we don't have wifi, dont mdns (it crashes for me)
	switch(wifiInstance.currentMode)
	{
		case myWifiClass::modeSTA_unjoined:
		case myWifiClass::modeSTA_unjoinedAndAP:
		case myWifiClass::modeSTAspeculative:
		case myWifiClass::modeAP:
			if(Details.dblog) Details.dblog->println(debug::dbInfo, "No WIFI joined, not doing FindPeers");
			doFind=false;
			break;
		case myWifiClass::modeOff: 
		case myWifiClass::modeCold:
		case myWifiClass::modeUnknown:
			if(Details.dblog) Details.dblog->println(debug::dbInfo, "No WIFI, not doing FindPeers");
			doFind=false;
			break;
		case myWifiClass::modeSTA:
		case myWifiClass::modeSTAandAP:
		default:
			break;
	}

	if(!doFind)
		return;

	if(Details.dblog) Details.dblog->printf(debug::dbInfo, "Looking for '%s' siblings ...\n\r", mdsnNAME);
	
	// get a list of what's out there
	Details.services.clear();
	if (wifiInstance.QueryServices(mdsnNAME, Details.services))
	{
		int found=(int)Details.services.size();
		if(Details.dblog) Details.dblog->printf(debug::dbInfo, "Found %d sibling%c!!\n\r", found, found==1?' ':'s');
		for (auto iterator = Details.services.begin(); iterator != Details.services.end(); iterator++)
		{
			if(Details.dblog) Details.dblog->printf(debug::dbInfo, "\t%s @ %s\n\r", iterator->hostName.c_str(), iterator->IP.toString().c_str());
		}
		
	}
	else
	{
		if(Details.dblog) Details.dblog->println(debug::dbInfo, "No others services found");
	}

}

void performUpdate(String url, String urlSpiffs)
{
	if(Details.dblog) Details.dblog->println(debug::dbImportant,"performing update");

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
#ifdef ESP32
			result=httpUpdate.updateSpiffs(wifiInstance.m_wificlient ,urlSpiffs+urlArgs,_MYVERSION);
#else			
			result=ESPhttpUpdate.updateSpiffs(wifiInstance.m_wificlient ,urlSpiffs+urlArgs,_MYVERSION);
#endif			
		}
		else
		{
			if(Details.dblog) Details.dblog->println(debug::dbImportant, "updating BIN ...");
			// we do some clean up so let me boot
#ifdef ESP32
			httpUpdate.rebootOnUpdate(false);
			result=httpUpdate.update(wifiInstance.m_wificlient, url+urlArgs, _MYVERSION);
#else
			ESPhttpUpdate.rebootOnUpdate(false);
			result=ESPhttpUpdate.update(wifiInstance.m_wificlient,url+urlArgs, _MYVERSION);
#endif			
		}

		switch (result)
		{
		case HTTP_UPDATE_FAILED:
#ifdef ESP32
			if(Details.dblog) Details.dblog->printf(debug::dbError, "%s HTTP_UPDATE_FAILED Error (%d): %s\r",(!updates?urlSpiffs.c_str():url.c_str()), httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
#else
			if(Details.dblog) Details.dblog->printf(debug::dbError, "%s HTTP_UPDATE_FAILED Error (%d): %s\r",(!updates?urlSpiffs.c_str():url.c_str()), ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
#endif			
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
#ifdef ESP32			
			details["espNarrative"] = httpUpdate.getLastErrorString();
			details["espResult"] = httpUpdate.getLastError();
#else
			details["espNarrative"] = ESPhttpUpdate.getLastErrorString();
			details["espResult"] = ESPhttpUpdate.getLastError();
#endif			

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

		// first time round, save our config, always, even if it fails
		if(!updates) // && (result==HTTP_UPDATE_OK))
		{
			if(Details.dblog) Details.dblog->println(debug::dbImportant, "preserving config");
			WriteJSONconfig();
		}

	}

	// preserve, reboot, if it worked
	if(result==HTTP_UPDATE_OK)
	{
		RebootMe(true);
	}

}


#ifdef _ESP_USE_ASYNC_WEB
void onPostBodyHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{

		if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "onPostBodyHandler %d\n\r", len);

		request->_tempObject=malloc(len+1);
		memcpy(request->_tempObject,data,len);
		char *dest=(char*)(request->_tempObject);
		dest[len]=0;



}
#endif


// set up all the handlers for the web server
void InstallWebServerHandlers(bool enableCORS)
{
	if(Details.dblog) Details.dblog->println(debug::dbVerbose, "InstallWebServerHandlers IN");

	// set up the json handlers
	// POST
	// all ON/OFF 
	// switch ON/OFF
	// revert

	wifiInstance.server.enableCORS(enableCORS);



	// make all the relays reflect their switches
#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/revert", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/revert", HTTP_POST, []() {
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/revert");

		RevertAllSwitch();

#ifdef _ESP_USE_ASYNC_WEB
		SendServerPage(request);
#else
		SendServerPage();
#endif		
	});

	// do something to all of them
#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/all", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/all",HTTP_POST, []() {
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/all");

#ifdef _ESP_USE_ASYNC_WEB
		for (uint8_t i = 0; i < request->params(); i++)
#else
		for (uint8_t i = 0; i < wifiInstance.server.args(); i++)
#endif		
		{
#ifdef _ESP_USE_ASYNC_WEB
			AsyncWebParameter* p = request->getParam(i);
			if(p->name()=="action")
			{
				DoAllSwitch(p->value() == "on" ? true : false, true);
			}
#else
			if (wifiInstance.server.argName(i) == "action")
			{
				DoAllSwitch(wifiInstance.server.arg(i) == "on" ? true : false, true);
			}
#endif			
		}

#ifdef _ESP_USE_ASYNC_WEB
		SendServerPage(request);
#else
		SendServerPage();
#endif		
	});


#ifdef _OTA_AVAILABLE

	// do an OTA update from a provided URL
#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/upgrade", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/upgrade", HTTP_POST, []() {
#endif
		if(Details.dblog) Details.dblog->println(debug::dbImportant, "json upgrade posted");



#ifdef _ESP_USE_ASYNC_WEB
		String body((char*)request->_tempObject);
		if(Details.dblog) Details.dblog->println(debug::dbImportant, body.c_str());
#else
		if(Details.dblog) Details.dblog->println(debug::dbImportant, wifiInstance.server.arg("plain"));
#endif		

		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = 
#ifdef _ESP_USE_ASYNC_WEB
			jsonBuffer.parseObject(body.c_str());
#else		
			jsonBuffer.parseObject(wifiInstance.server.arg("plain"));
#endif		

		if(Details.upgradeOnlyWhenRelayOff)
		{
			for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
			{
				if((*each)->GetRelay())
				{
					// it's on ... bounce
					if(Details.dblog) Details.dblog->println(debug::dbImportant, "ignoring upgrade because the relay is ON");
#ifdef _ESP_USE_ASYNC_WEB
					request->send(403, "application/json", "{ reason: 'not while turned on' }");
#else
					wifiInstance.server.send(403, "application/json", "{ reason: 'not while turned on' }");
#endif					
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
		Details.updateAvailable=true;
		Details.url=root["url"].as<char*>();
		Details.urlSpiffs=root["urlSpiffs"].as<char*>();


		// no point - core killed the IP stack during the update
#ifdef _ESP_USE_ASYNC_WEB
		request->send(200, "text/html", "<html/>");
#else
		wifiInstance.server.send(200, "text/html", "<html/>");
#endif		


#ifdef _ESP_USE_ASYNC_WEB
	}).onBody(onPostBodyHandler);
#else
	});
#endif

#endif

	// inverse of whatever it's currently doing
#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/toggle", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/toggle",HTTP_POST, []() {
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/toggle");

		// TODO pick the right one
		// just go thru them all
		for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
			(*each)->ToggleRelay();



	});

#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/button", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/button",HTTP_POST, []() {
#endif

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "/button");

#ifdef _ESP_USE_ASYNC_WEB
		for (int count = 0; count < request->params(); count++)
#else
		for (int count = 0; count < wifiInstance.server.args(); count++)
#endif		
		{
#ifdef _ESP_USE_ASYNC_WEB			
			AsyncWebParameter* p = request->getParam(count);
#endif			

			if(Details.dblog) Details.dblog->printf(debug::dbInfo, "%d. %s = %s \n\r", 
				count+1, 
#ifdef _ESP_USE_ASYNC_WEB
				p->name().c_str(), 
				p->value().c_str()
#else				
				wifiInstance.server.argName(count).c_str(), 
				wifiInstance.server.arg(count).c_str()
#endif				
			);
		}



#ifdef _ESP_USE_ASYNC_WEB
		if(request->hasParam("action"))
#else
		if (wifiInstance.server.hasArg("action"))
#endif		
		{
#ifdef _ESP_USE_ASYNC_WEB			
			AsyncWebParameter* p = request->getParam("action");
			bool action = p->value() == "on" ? true : false;
#else
			bool action = wifiInstance.server.arg("action") == "on" ? true : false;
#endif			

			
#ifdef _ESP_USE_ASYNC_WEB
			if(request->hasParam("port"))
#else
			if(wifiInstance.server.hasArg("port"))
#endif			
			{
#ifdef _ESP_USE_ASYNC_WEB			
				AsyncWebParameter* p = request->getParam("port");
				int port=p->value().toInt();
#else
				int port=wifiInstance.server.arg("port").toInt();
#endif

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

#ifdef _ESP_USE_ASYNC_WEB
		SendServerPage(request);
#else
		SendServerPage();
#endif		

	});

	// LEGACY - remove when the beach house is upgraded past 0.0.32 and home assistant does POSTS
#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/button", [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/button", []() {
#endif

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "/button");

#ifdef _ESP_USE_ASYNC_WEB
		for (int count = 0; count < request->params(); count++)
#else
		for (int count = 0; count < wifiInstance.server.args(); count++)
#endif		
		{
#ifdef _ESP_USE_ASYNC_WEB
			AsyncWebParameter* p = request->getParam(count);
#endif

			if(Details.dblog) Details.dblog->printf(debug::dbInfo, "%d. %s = %s \n\r", 
				count+1, 
#ifdef _ESP_USE_ASYNC_WEB
				p->name().c_str(),
				p->value().c_str()
#else				
				wifiInstance.server.argName(count).c_str(), 
				wifiInstance.server.arg(count).c_str()
#endif				
			);
		}



#ifdef _ESP_USE_ASYNC_WEB
		if(request->hasParam("action"))
#else
		if (wifiInstance.server.hasArg("action"))
#endif		
		{
#ifdef _ESP_USE_ASYNC_WEB
			AsyncWebParameter* p = request->getParam("action");
			bool action = p->value() == "on" ? true : false;
#else
			bool action = wifiInstance.server.arg("action") == "on" ? true : false;
#endif			
			
#ifdef _ESP_USE_ASYNC_WEB
			if(request->hasParam("port"))
#else
			if(wifiInstance.server.hasArg("port"))
#endif			
			{
#ifdef _ESP_USE_ASYNC_WEB
				AsyncWebParameter* p = request->getParam("port");
				int port=p->value().toInt();
#else				
				int port=wifiInstance.server.arg("port").toInt();
#endif				
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

#ifdef _ESP_USE_ASYNC_WEB
		SendServerPage(request);
#else
		SendServerPage();
#endif		

	});

#ifndef PLATFORM_SONOFF_SWITCH

#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/devices/del", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/devices/del",HTTP_POST, []() {
#endif		
		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/json/devices/del");

		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body

#ifdef _ESP_USE_ASYNC_WEB
		String body((char*)request->_tempObject);
#endif

		JsonObject& root = 
#ifdef _ESP_USE_ASYNC_WEB
			jsonBuffer.parseObject(body.c_str());
#else		
			jsonBuffer.parseObject(wifiInstance.server.arg("plain"));
#endif		

		if(root.containsKey("id") && root.containsKey("config") && root.containsKey("instance"))
		{

			unsigned id=root["id"];
			String config=root["config"];
			unsigned long instance=root["instance"];

			// remove instance and sensor
			for(auto each=Details.sensors.begin();each!=Details.sensors.end();each++)
			{
				if(std::get<0>(*each)==id && std::get<1>(*each)==config)
				{
					if(Details.dblog) Details.dblog->printf(debug::dbImportant, "removing sensor %d - '%s'\n\r",std::get<0>(*each),std::get<2>(*each)->GetName().c_str());

					// remove it
					delete std::get<2>(*each);
					std::get<2>(*each)=NULL;
					Details.sensors.erase(each);
					break;
				}
			}

			WriteJSONconfig();

		}
		else
		{
		
#ifdef _ESP_USE_ASYNC_WEB
			if(Details.dblog) Details.dblog->printf(debug::dbImportant, "malformed data '%s'\n\r",body.c_str());
#else		
			if(Details.dblog) Details.dblog->printf(debug::dbError, "malformed data '%s'\n\r",wifiInstance.server.arg("plain").c_str());
#endif
		}

#ifdef _ESP_USE_ASYNC_WEB
		request->send(200,"text/html","<html/>");
#else
		wifiInstance.server.send(200,"text/html","<html/>");
#endif		

	});



#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/devices/add", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/devices/add",HTTP_POST, []() {
#endif		


		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/json/devices/add");
#ifdef _ESP_USE_ASYNC_WEB
		String body((char*)request->_tempObject);
		if(Details.dblog) Details.dblog->println(debug::dbImportant, body.c_str());
#else
		if(Details.dblog) Details.dblog->println(debug::dbImportant, wifiInstance.server.arg("plain"));
#endif		

		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = 
#ifdef _ESP_USE_ASYNC_WEB
			jsonBuffer.parseObject(body.c_str());
#else		
			jsonBuffer.parseObject(wifiInstance.server.arg("plain"));
#endif		

		bool reboot=false, add=true;

		String newInstanceConfig;

		if(root.containsKey("config"))
		{
			root["config"].printTo(newInstanceConfig);
		}

		// check it's not already there
		unsigned id=root["id"];

		// 
		for(auto each=Details.sensors.begin();each!=Details.sensors.end();each++)
		{
			if(std::get<0>(*each)==id)
			{
				if(std::get<1>(*each)==newInstanceConfig)
				{
					// er - no
					add=false;
					reboot=false;
					if(Details.dblog) Details.dblog->println(debug::dbWarning, "adding same instance ignored");
					break;
				}
			}
		}

		if(add)
		{
			Details.sensors.push_back(std::tuple<unsigned,String,baseSensor*>(id,newInstanceConfig,NULL));
			WriteJSONconfig();
			AddDeviceInstance();
		}



#ifdef _ESP_USE_ASYNC_WEB
		request->send(200,"text/html","<html/>");
#else
		wifiInstance.server.send(200,"text/html","<html/>");
#endif		

		// minimise heap fracture
		if(reboot)
		{
			RebootMe(true);
			if(Details.dblog) Details.dblog->println(debug::dbImportant, "rebooting");
		}


#ifdef _ESP_USE_ASYNC_WEB
	}).onBody(onPostBodyHandler);
#else
	});
#endif

#endif //ARDUINO_ESP8266_GENERIC

#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/logging", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/logging",HTTP_POST, []() {
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/json/logging");
#ifdef _ESP_USE_ASYNC_WEB
		String body((char*)request->_tempObject);
		if(Details.dblog) Details.dblog->println(debug::dbImportant, body.c_str());
#else
		if(Details.dblog) Details.dblog->println(debug::dbImportant, wifiInstance.server.arg("plain"));
#endif		

		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = 
#ifdef _ESP_USE_ASYNC_WEB
			jsonBuffer.parseObject(body.c_str());
#else		
			jsonBuffer.parseObject(wifiInstance.server.arg("plain"));
#endif		

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

#ifdef _ESP_USE_ASYNC_WEB
		request->send(200,"text/html","<html/>");
#else
		wifiInstance.server.send(200,"text/html","<html/>");
#endif		

		// minimise heap fracture
		if(reboot)
			RebootMe(true);

#ifdef _ESP_USE_ASYNC_WEB
	}).onBody(onPostBodyHandler);
#else
	});
#endif







#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/resetCounts", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/resetCounts",HTTP_POST, []() {
#endif

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/resetCounts");

		for(auto each=Details.switches.begin();each!=Details.switches.end();each++)
			(*each)->ResetTransitionCount();

#ifdef _ESP_USE_ASYNC_WEB
		request->send(200, "text/html", "<html/>");
#else
		wifiInstance.server.send(200,"text/html","<html/>");
#endif		

	});

#ifdef _DEVELOPER_BUILD

#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/reboot",HTTP_POST, []() {
#endif

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/reboot");

		RebootMe(true);

	});

#endif

#ifdef _ALLOW_WIFI_RESET_OVER_WIFI

#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/resetWIFI", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/resetWIFI",HTTP_POST, []() {
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/resetWIFI");

#ifdef _ESP_USE_ASYNC_WEB
		request->send(200, "text/html", "<html/>");
#else
		wifiInstance.server.send(200, "text/html", "<html/>");
#endif			

		Details.resetWIFI=true;

	});

#endif

#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/stopAP", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/stopAP",HTTP_POST, []() {
#endif

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "/stopAP");

		if (wifiInstance.currentMode == myWifiClass::wifiMode::modeSTAandAP)
		{
			wifiInstance.ConnectWifi(myWifiClass::wifiMode::modeSTA, Details.wifi);
#ifdef _ESP_USE_ASYNC_WEB
			request->send(200, "text/html", "<html/>");
#else
			wifiInstance.server.send(200, "text/html", "<html/>");
#endif			
		}
		else
		{
#ifdef _ESP_USE_ASYNC_WEB
			request->send(500, "text/html", "<html/>");
#else
			wifiInstance.server.send(500, "text/html", "<html/>");
#endif			
		}


	});



#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/",  [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/", []() {
#endif

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "/ requested");

#ifdef _ESP_USE_ASYNC_WEB
		SendServerPage(request);
#else
		SendServerPage();
#endif		

	});

#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/default.htm",  [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/default.htm", []() {
#endif

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "/default.htm");

#ifdef _ESP_USE_ASYNC_WEB
		SendServerPage(request);
#else
		SendServerPage();
#endif		

	});

	// posted config
#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/config", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/config", HTTP_POST, []() {
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "json config posted");
#ifdef _ESP_USE_ASYNC_WEB
		String body((char*)request->_tempObject);
		if(Details.dblog) Details.dblog->println(debug::dbImportant, body.c_str());
#else
		if(Details.dblog) Details.dblog->println(debug::dbImportant, wifiInstance.server.arg("plain"));
#endif		

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = 
#ifdef _ESP_USE_ASYNC_WEB
			jsonBuffer.parseObject(body.c_str());
#else		
			jsonBuffer.parseObject(wifiInstance.server.arg("plain"));
#endif		


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

#ifdef _ESP_USE_ASYNC_WEB
		request->send(200, "text/html", "<html/>");
#else
		wifiInstance.server.send(200, "text/html", "<html/>");
#endif		

#ifdef _ESP_USE_ASYNC_WEB
	}).onBody(onPostBodyHandler);
#else
	});
#endif


#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/listen", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/listen", HTTP_POST, []() {
#endif		

		IPAddress recipientAddr = 
#ifdef _ESP_USE_ASYNC_WEB
			request->client()->remoteIP();
#else
			wifiInstance.server.client().remoteIP();
#endif		

		if(Details.dblog) Details.dblog->printf(debug::dbInfo, "json listen posted from %s\n\r",recipientAddr.toString().c_str());
#ifdef _ESP_USE_ASYNC_WEB
		String body((char*)request->_tempObject);
		if(Details.dblog) Details.dblog->println(debug::dbImportant, body.c_str());
#else
		if(Details.dblog) Details.dblog->println(debug::dbImportant, wifiInstance.server.arg("plain"));
#endif		

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = 
#ifdef _ESP_USE_ASYNC_WEB
			jsonBuffer.parseObject(body.c_str());
#else		
			jsonBuffer.parseObject(wifiInstance.server.arg("plain"));
#endif		

		int recipientPort = root["port"];

		// can be switch or sensor
		if(root.containsKey("sensor"))
		{

			int recipientSensor = root["sensor"];

			if(recipientSensor<Details.sensors.size())
			{
				if(Details.dblog) Details.dblog->printf(debug::dbInfo, "Adding recipient %s:%d Sensor %d\n\r", recipientAddr.toString().c_str(), recipientPort, recipientSensor);
#ifdef _ESP_USE_ASYNC_WEB
				String body((char*)request->_tempObject);
				GETSENSOR(Details.sensors[recipientSensor])->AddAnnounceRecipient(recipientAddr,recipientPort,(body.c_str()));
#else				
				GETSENSOR(Details.sensors[recipientSensor])->AddAnnounceRecipient(recipientAddr,recipientPort,(wifiInstance.server.arg("plain")));
#endif				
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
#ifdef _ESP_USE_ASYNC_WEB
				String body((char*)request->_tempObject);
				Details.switches[recipientSwitch]->AddAnnounceRecipient(recipientAddr,recipientPort,body.c_str());
#else				
				Details.switches[recipientSwitch]->AddAnnounceRecipient(recipientAddr,recipientPort,wifiInstance.server.arg("plain"));
#endif				


			}
			else
			{
				if(Details.dblog) Details.dblog->println(debug::dbError, "Switch exceeded bounds");
			}

		}

#ifdef _ESP_USE_ASYNC_WEB
		request->send(200, "text/html", "<html/>");
#else
		wifiInstance.server.send(200, "text/html", "<html/>");
#endif

#ifdef _ESP_USE_ASYNC_WEB
	}).onBody(onPostBodyHandler);
#else
	});
#endif





#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/wifi", HTTP_POST, []() {
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbImportant, "json wifi posted");
#ifdef _ESP_USE_ASYNC_WEB
		String body((char*)request->_tempObject);
		if(Details.dblog) Details.dblog->println(debug::dbImportant, body.c_str());
#else
		if(Details.dblog) Details.dblog->println(debug::dbImportant, wifiInstance.server.arg("plain"));
#endif		

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();
		// 'plain' is the secret source to get to the body
		JsonObject& root = 
#ifdef _ESP_USE_ASYNC_WEB
			jsonBuffer.parseObject(body.c_str());
#else		
			jsonBuffer.parseObject(wifiInstance.server.arg("plain"));
#endif		

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

		Details.desiredMode=myWifiClass::wifiMode::modeSTAspeculative;
		Details.wifiChangeRequested=true;

		// and update json
		WriteJSONconfig();

#ifdef _ESP_USE_ASYNC_WEB
		request->send(200, "text/html", "<html/>");
#else
		wifiInstance.server.send(200, "text/html", "<html/>");
#endif

#ifdef _ESP_USE_ASYNC_WEB
	}).onBody(onPostBodyHandler);
#else
	});
#endif

	// GET
	
#ifdef PLATFORM_ESP32_CAMERA

#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/camera", HTTP_GET, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/camera", HTTP_GET, []() {
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "json camera called");


#ifdef _ESP_USE_ASYNC_WEB
		if(request->hasParam("cam"))
#else
		if (wifiInstance.server.hasArg("cam"))
#endif		
		{
#ifdef _ESP_USE_ASYNC_WEB			
			AsyncWebParameter* p = request->getParam("cam");
			int cam = p->value().toInt();
#else
			int cam = wifiInstance.server.arg("cam").toInt();
#endif			
			if(cam>=0 && cam<Details.cameras.size())
			{
#ifdef _ESP_USE_ASYNC_WEB
				//AsyncWebServerResponse *response = request->beginResponse(200, "image/jpg", "<html/>");
				//baseCamera *theCam=Details.cameras[cam];
				debugBaseClass *dblog=Details.dblog;


				Details.cameras[cam]->AddWork([=](baseCamera *theCam)->void{

					// get an image from that one
					size_t imgSizeReq=theCam->requestFrame();

					if(imgSizeReq>0)
					{
						size_t imgSize=0;
						uint8_t *imgBytes=NULL;
						if(theCam->fetchFrame(&imgBytes,&imgSize))
						{

							if(dblog) dblog->printf(debug::dbVerbose, "json camera:%d - img %u size @ 0x%x\n\r",cam,imgSize,imgBytes);		

//#define _SEND_IMG_IMMEDIATE							

#ifdef _SEND_IMG_IMMEDIATE
							request->send("image/jpeg", imgSize, [=](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
#else								
							AsyncWebServerResponse *response = request->beginResponse("image/jpeg", imgSize, [=](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
#endif									
									if(dblog) dblog->printf(debug::dbVerbose, "called with @ 0x%x %u %u\n\r",buffer,maxLen,index);

									size_t sizeLeft=imgSize-index;
									size_t sizeToSend=((sizeLeft>maxLen)?maxLen:sizeLeft);

									if(dblog) dblog->printf(debug::dbVerbose, "sending %u bytes\n\r",sizeToSend);

									memcpy(buffer, imgBytes+index, sizeToSend);

									if(!(sizeLeft-sizeToSend))
									{
										if(dblog) dblog->println(debug::dbVerbose, "Freeing image");
										free(imgBytes);
									}

									return sizeToSend;


								});

#ifndef _SEND_IMG_IMMEDIATE
							response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
							request->send(response);
#endif

							if(dblog) dblog->println(debug::dbVerbose, "image sent");

						}
						else
						{
							if(dblog) dblog->println(debug::dbError, "json camera error - fetchFrame error");		
						}
					}
					else
					{
						if(dblog) dblog->println(debug::dbError, "json camera error - no img size");		
					}



				});

				return;

#else

				// get an image from that one
				size_t imgSizeReq=Details.cameras[cam]->requestFrame();

				if(imgSizeReq>0)
				{
					size_t imgSize;
					uint8_t *imgBytes=NULL;
					if(Details.cameras[cam]->fetchFrame(&imgBytes,&imgSize))
					{
						if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "json camera %u size\n\r",imgSize);		
						wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
						wifiInstance.server.send(200, "image/jpeg", imgBytes, imgSize);
						free(imgBytes);

						return;
					}
					else
					{
						if(Details.dblog) Details.dblog->println(debug::dbError, "json camera error - fetchFrame error");		
					}
				}
				else
				{
					if(Details.dblog) Details.dblog->println(debug::dbError, "json camera error - no img size");		
				}

#endif


			}
			else
			{
				if(Details.dblog) Details.dblog->println(debug::dbError, "json camera error - cam out of bounds");		
			}
			
		}
		else
		{
			if(Details.dblog) Details.dblog->println(debug::dbError, "json camera error - no cam number");		
		}
		


#ifdef _ESP_USE_ASYNC_WEB
		AsyncWebServerResponse *response = request->beginResponse(500, "html/text", "<html/>");
		response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		request->send(response);
#else
		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(500, "html/text", "<html/>");
#endif		


	});


#endif



#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/state", HTTP_GET, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/state", HTTP_GET, []() {
#endif		
		// give them back the port / switch map

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "json state called");

		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = wifiInstance.m_hostName.c_str();
		root["friendlyName"] = Details.friendlyName;
		

		if(wifiInstance.isLocalIPset())
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

			yield();

		}

		root["sensorCount"] = Details.sensors.size();
		JsonArray &sensorState = root.createNestedArray("sensorState");
		count=0;
		for(auto each=Details.sensors.begin();each!=Details.sensors.end();each++, count++)
		{
			JsonObject &switchRelay = sensorState.createNestedObject();
			switchRelay["sensor"] = count;

			GETSENSOR(*each)->GetSensorValue(switchRelay);
			switchRelay["name"] = GETSENSOR(*each)->GetName();

			yield();

		}

#ifdef PLATFORM_ESP32_CAMERA

		root["cameraCount"] = Details.cameras.size();
		JsonArray &cameraState = root.createNestedArray("cameraState");
		count=0;
		for(auto each=Details.cameras.begin();each!=Details.cameras.end();each++, count++)
		{
			JsonObject &camera = cameraState.createNestedObject();
			camera["camera"] = count;

			camera["name"] = (*each)->GetName();

			yield();

		}
#else
		root["cameraCount"] =0;
#endif


		String jsonText;
#ifdef _DEVELOPER_BUILD
		root.prettyPrintTo(jsonText);
#else		
		root.printTo(jsonText);
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbVerbose, jsonText);

#ifdef _ESP_USE_ASYNC_WEB
		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonText);
		response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		request->send(response);
#else
		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
#endif		
	});



#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/logging", HTTP_GET, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/logging", HTTP_GET, []() {
#endif		

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
#ifdef _DEVELOPER_BUILD
		root.prettyPrintTo(jsonText);
#else		
		root.printTo(jsonText);
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbVerbose, jsonText);

#ifdef _ESP_USE_ASYNC_WEB
		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonText);
		response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		request->send(response);
#else
		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
#endif



	});




#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/devices", HTTP_GET, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/devices", HTTP_GET, []() {
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "json devices called");

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		JsonArray &instances=root.createNestedArray("instances");

#ifdef PLATFORM_WEMOS_SENSOR		

		for(auto each=Details.sensors.begin();each!=Details.sensors.end();each++)
		{
			auto found=Details.options.begin();

			for(;found!=Details.options.end();found++)
			{
				// deref to get to pointer, deref to get to object
				if(*(*found)==std::get<0>(*each))
					break;
			}


			if(found==Details.options.end())
			{
				// this is bad!
				if(Details.dblog) Details.dblog->println(debug::dbError, "device not found");
				continue;
			}

			JsonObject &instance=instances.createNestedObject();
			instance["id"]=(unsigned int)std::get<0>(*each);
			instance["config"]=std::get<1>(*each);
			instance["name"]=(*found)->Name();
			instance["instance"]=(unsigned long)std::get<2>(*each);
		}

#endif		

		JsonArray &options=root.createNestedArray("options");

#if !defined(PLATFORM_SONOFF_SWITCH)

		for(auto each=Details.options.begin();each!=Details.options.end();each++)
		{
			JsonObject& option=options.createNestedObject();

			(*each)->getConfigOptionsJSON(option);


		}

#endif



		String jsonText;
#ifdef _DEVELOPER_BUILD
		root.prettyPrintTo(jsonText);
#else		
		root.printTo(jsonText);
#endif		


#ifdef _ESP_USE_ASYNC_WEB
		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonText);
		response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		request->send(response);
#else
		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
#endif		
	});


#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/config", HTTP_GET, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/config", HTTP_GET, []() {
#endif		
		// give them back the port / switch map

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "json config called");

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = wifiInstance.m_hostName.c_str();
		root["version"] = _MYVERSION;

		if(wifiInstance.isLocalIPset())
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
		if(Details.sensors.size())
		{
			root["sensorCount"] = Details.sensors.size();
			JsonArray &sensorConfig = root.createNestedArray("sensorConfig");
			int count=0;
			for(auto each=Details.sensors.begin();each!=Details.sensors.end();each++, count++)
			{
				JsonObject &switchRelay = sensorConfig.createNestedObject();
				switchRelay["sensor"] = count;

				GETSENSOR(*each)->GetSensorConfig(switchRelay);
				switchRelay["name"] = GETSENSOR(*each)->GetName();

				yield();
			}
		}

		// add switches
		if(Details.switches.size())
		{
			root["switchCount"] = Details.switches.size();
			JsonArray &switchConfig = root.createNestedArray("switchConfig");
			int count=0;
			for(auto each=Details.switches.begin();each!=Details.switches.end();each++, count++)
			{
				JsonObject &switchRelay = switchConfig.createNestedObject();
				switchRelay["switch"] = count;

				String impl=(*each)->GetImpl();
				if(impl.length())
					switchRelay["impl"]=impl;

				switchRelay["name"] = (*each)->GetName();

				yield();

			}
		}

#ifdef PLATFORM_ESP32_CAMERA

		// add cameras
		if(Details.cameras.size())
		{
			root["cameraCount"] = Details.cameras.size();
			JsonArray &cameraConfig = root.createNestedArray("cameraConfig");
			int count=0;
			for(auto each=Details.cameras.begin();each!=Details.cameras.end();each++, count++)
			{
				JsonObject &camera = cameraConfig.createNestedObject();
				camera["camera"] = count;

				String impl=(*each)->GetImpl();
				if(impl.length())
					camera["impl"]=impl;

				camera["name"] = (*each)->GetName();

				yield();

			}
		}

#endif

		String jsonText;
#ifdef _DEVELOPER_BUILD
		root.prettyPrintTo(jsonText);
#else		
		root.printTo(jsonText);
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbVerbose, jsonText);

#ifdef _ESP_USE_ASYNC_WEB
		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonText);
		response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		request->send(response);
#else
		//wifiInstance.server.sendHeader("Cache-Control", "max-age=120");
		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
#endif		
	});

#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/wificonfig", HTTP_GET, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/wificonfig", HTTP_GET, []() {
#endif		
		// give them back the port / switch map

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "json wificonfig called");

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();

		root["name"] = wifiInstance.m_hostName.c_str();
		root["ssid"] = wifiInstance.SSID();

		if(wifiInstance.isLocalIPset())
			root["ip"] = wifiInstance.localIP().toString();

		root["friendlyName"]=Details.friendlyName;

		String jsonText;
#ifdef _DEVELOPER_BUILD
		root.prettyPrintTo(jsonText);
#else		
		root.printTo(jsonText);
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbVerbose, jsonText);

#ifdef _ESP_USE_ASYNC_WEB
		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonText);
		response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		request->send(response);
#else
		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
#endif		
	});

	///json/peers
#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/peers", HTTP_GET, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/peers", HTTP_GET, []() {
#endif		
		// give them back the port / switch map

		if(Details.dblog) Details.dblog->println(debug::dbInfo, "json peers called");

		//StaticJsonBuffer<JSON_STATIC_BUFSIZE> jsonBuffer;
		jsonBuffer.clear();

		JsonObject &root = jsonBuffer.createObject();
		root["name"] = wifiInstance.m_hostName.c_str();
		root["peerCount"] = Details.services.size();

		if(wifiInstance.isLocalIPset())
			root["ip"] = wifiInstance.localIP().toString();


		// let's get all peers we can see
		
		JsonArray &peers = root.createNestedArray("peers");

		for (size_t each = 0; each < Details.services.size(); each++)
		{
			JsonObject &peer = peers.createNestedObject();

			if(Details.dblog) Details.dblog->printf(debug::dbInfo, "%d '%s' %s\n\r", each + 1, Details.services[each].hostName.c_str(), Details.services[each].IP.toString().c_str());
			peer["name"]=Details.services[each].hostName;
			peer["ip"]=Details.services[each].IP.toString();
		}


		String jsonText;
#ifdef _DEVELOPER_BUILD
		root.prettyPrintTo(jsonText);
#else		
		root.printTo(jsonText);
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbVerbose, jsonText);

		// do not cache
#ifdef _ESP_USE_ASYNC_WEB
		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonText);
		response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		request->send(response);
#else
		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
#endif		
	});



#ifdef _ESP_USE_ASYNC_WEB
	wifiInstance.server.on("/json/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
#else	
	wifiInstance.server.on("/json/wifi", HTTP_GET, []() {
#endif		
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

		if(Details.dblog) Details.dblog->printf(debug::dbInfo, "jScanNetworks found %d\r",found);

		JsonArray &wifis = root.createNestedArray("wifi");

		// get the 10 strongest (it's a sorted list)
		int maxFound = found < 10 ? found : 10;

		for (int each = 0; each < maxFound ; each++)
		{
			JsonObject &wifi = wifis.createNestedObject();
			wifi["ssid"] = allWifis[each].first;
			wifi["sig"] = allWifis[each].second;

			if(Details.dblog) Details.dblog->printf(debug::dbInfo, "%d '%s' %d \n\r", each + 1, allWifis[each].first.c_str(), allWifis[each].second);

		}
		

		String jsonText;
#ifdef _DEVELOPER_BUILD
		root.prettyPrintTo(jsonText);
#else		
		root.printTo(jsonText);
#endif		

		if(Details.dblog) Details.dblog->println(debug::dbVerbose, jsonText);

		// do not cache
#ifdef _ESP_USE_ASYNC_WEB
		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonText);
		response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		request->send(response);
#else
		wifiInstance.server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		wifiInstance.server.send(200, "application/json", jsonText);
#endif		
	});

	// serve up everthing in SPIFFS
#ifdef ESP32
	File root=SPIFFS.open("/");
	File dir;
	for (File dir=root.openNextFile();dir;dir=root.openNextFile()) 
#else	
	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) 
#endif		
	{
#ifdef ESP32
		String file=dir.name();
#else
		String file = dir.fileName();
#endif		

#ifndef _DEVELOPER_BUILD_
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

#ifdef _STORE_STATIC_FILES
	#ifdef ESP32
		Details.servedFiles.push_back(std::pair<String,size_t>(file, dir.size()));
	#else
		Details.servedFiles.push_back(std::pair<String,size_t>(file, dir.fileSize()));
	#endif		
#endif
	}

	if(Details.dblog) Details.dblog->printf(debug::dbVerbose, "InstallWebServerHandlers OUT\n\r");

}



#ifdef _ESP_USE_ASYNC_WEB
void SendServerPage(AsyncWebServerRequest *request)
#else
void SendServerPage()
#endif
{
	// given the current state of the device, send the appropriate page back
	
	String toOpen("/_Error.htm");
	switch (wifiInstance.currentMode)
	{
	case myWifiClass::wifiMode::modeSTAandAP:
		toOpen="/_STAAPmode.htm";
		break;
	case myWifiClass::wifiMode::modeSTA_unjoinedAndAP:
	case myWifiClass::wifiMode::modeAP:
		toOpen = "/_APmode.htm";
		break;
	case myWifiClass::wifiMode::modeSTA:
		toOpen = "/_STAmode.htm";
		break;
	case myWifiClass::wifiMode::modeUnknown:
	default:
		toOpen = "/_Error.htm";
		break;

	}

	if(SPIFFS.exists(toOpen))
	{
#ifdef _ESP_USE_ASYNC_WEB
		request->send(SPIFFS, toOpen);

#else
		File f = SPIFFS.open(toOpen, "r");

		// let's make sure it bloody exists !

		wifiInstance.server.streamFile(f, "text/html");
		f.close();
#endif
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

	if(Details.updateAvailable)
	{
		performUpdate(Details.url,Details.urlSpiffs);
		// shouldn't get here, unless the update fails or there's nothing to do
		Details.updateAvailable=false;
		Details.url.clear();
		Details.urlSpiffs.clear();
	}

	if(Details.wifiChangeRequested)
	{
		// Details.desiredMode
		if (wifiInstance.ConnectWifi(Details.desiredMode, Details.wifi) == myWifiClass::wifiMode::modeSTAandAP)
		{
			
			Details.wifi.configured = true;
		}
		else
		{
			Details.wifi.configured = false;
		}

		Details.wifiChangeRequested=false;
		// and update json
		WriteJSONconfig();
	}

#ifdef _ALLOW_WIFI_RESET
	if (Details.resetWIFI)
		ResetToAP();
#endif

#ifdef _ALLOW_WIFI_RESET_AFTER_AP_JOIN_TIMEOUT

	if(wifiInstance.currentMode!=myWifiClass::modeSTA_unjoined)
	{
		Details.runtimeWhenLastJoined=millis();
	}
	else if(millis()-Details.runtimeWhenLastJoined > _ALLOW_WIFI_RESET_AFTER_AP_JOIN_TIMEOUT)
	{
		if(Details.dblog) Details.dblog->println(debug::dbImportant, "Timedout trying to join Wifi - adding AP");
		AddAPtoSTA();
		Details.runtimeWhenLastJoined=millis();
	}
#endif

	if (Details.configDirty)
		WriteJSONconfig();

	// let internals work
	wifiInstance.serviceComponents();

	// sensors may need some work
	for(auto eachSensor=Details.sensors.begin();eachSensor!=Details.sensors.end();eachSensor++)
	{
		GETSENSOR(*eachSensor)->DoWork();
	}

	for(auto eachSwitch=Details.switches.begin();eachSwitch!=Details.switches.end();eachSwitch++)
	{
		(*eachSwitch)->DoWork();
	}

#ifdef PLATFORM_ESP32_CAMERA
	for(auto eachCamera=Details.cameras.begin();eachCamera!=Details.cameras.end();eachCamera++)
	{
		(*eachCamera)->DoWork();
	}
#endif

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

