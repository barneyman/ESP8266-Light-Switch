#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

#include "mcp23017.h"

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



extern "C" {

#include <user_interface.h>		// for system_get_chip_id();

}


#include "debug_defines.h"



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
WiFiEventHandler onConnect, onDisconnect;


volatile bool busyDoingSomethingIgnoreSwitch = false;

String esphostname = "esp8266_";


enum wifiMode { modeAP, modeSTA, modeUnknown } ;
wifiMode currentMode = modeUnknown;


void ConnectWifi(wifiMode intent);


ESP8266WebServer server(80);

String webPageSTA = "";
String webPageAP = "";
String webPageAPtry = "";

// light manual trigger IN
int inputSwitchPin = 14; // D5
int resetMCPpin = 16;// D0;

mcp23017 mcp(4, 5, resetMCPpin);


// number of relays & switches
#define NUM_SOCKETS	6


#define _RESET_VIA_QUICK_SWITCH
//#define _IGNORE_BOUNCE_LOGIC	

#ifndef _IGNORE_BOUNCE_LOGIC
	unsigned long last_micros[NUM_SOCKETS];
#endif

#ifdef _RESET_VIA_QUICK_SWITCH
	#define RESET_ARRAY_SIZE 6
	unsigned long lastSwitchesSeen[NUM_SOCKETS][RESET_ARRAY_SIZE];
	bool resetWIFI = false;
#endif

	// millis timeouts
#define QUICK_SWITCH_TIMEOUT_DEFAULT	3000
#define BOUNCE_TIMEOUT_DEFAULT			100



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

	for (unsigned port = 0; port < NUM_SOCKETS; port++)
	{
		DEBUG(DEBUG_VERBOSE,Serial.printf("Checking port %d\r\n",port));

		// +8 to get HIBYTE to see if this port CAUSED the interrupt
		if (causeAndState & (1 << (port + 8)))
		{

#if defined(_RESET_VIA_QUICK_SWITCH) || !defined(_IGNORE_BOUNCE_LOGIC)
			unsigned long now = micros(), interval=0;
#endif

#ifndef _IGNORE_BOUNCE_LOGIC
			// gate against messy tactile/physical switches
			interval = now - last_micros[port];

			DEBUG(DEBUG_WARN, Serial.printf("%lu ms ", interval / 1000UL));

			if (interval >= (unsigned long)(Details.debounceThresholdms * 1000))
#endif
			{
#ifdef _RESET_VIA_QUICK_SWITCH
				// move the last seens along
				memmove(&lastSwitchesSeen[port][0], &lastSwitchesSeen[port][1], sizeof(lastSwitchesSeen[port][1])*RESET_ARRAY_SIZE - 1);
				lastSwitchesSeen[port][RESET_ARRAY_SIZE - 1] =now/1000;

				DEBUG(DEBUG_INFO, Serial.printf("lastSwitchesSeen "));

				for (int each = 0; each < NUM_SOCKETS; each++)
				{
					DEBUG(DEBUG_INFO, Serial.printf("%lu ", lastSwitchesSeen[port][each]));
				}

				DEBUG(DEBUG_INFO, Serial.printf("\n\r"));
#endif

				// having CAUSED the interrupt, reflect its STATE in the DoSwitch call
				DoSwitch(port, (causeAndState & (1 << port)) ? true : false, false);



#ifdef _RESET_VIA_QUICK_SWITCH

				// remember the last 6 - i'm assuming we won't wrap
				// only reset if we're currently STA
				if (currentMode == wifiMode::modeSTA)
				{
					if (lastSwitchesSeen[port][RESET_ARRAY_SIZE - 1] - lastSwitchesSeen[port][0] < (Details.resetWindowms))
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
			last_micros[port] = now;
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
		DoSwitch(port, 
			mcp.readSwitch(port),
			false);

	}
}

// override switch state
void DoAllSwitch(bool state, bool force)
{
	for (int port = 0; port < NUM_SOCKETS; port++)
		DoSwitch(port, state, force);
}

// do, portNumber is 0 thru 7
// if forceSwitchToReflect change polarity of input switch if necessary to reflect this request
void DoSwitch(int portNumber, bool on, bool forceSwitchToReflect)
{
	if (portNumber > 7 || portNumber < 0)
		return;

	DEBUG(DEBUG_IMPORTANT, Serial.printf("DoSwitch: port %d %s %s\r\n", portNumber, on?"ON":"off", forceSwitchToReflect?"FORCE":""));

	mcp.SetRelay(portNumber, on, forceSwitchToReflect);

}

#ifdef _USE_JSON_NOT_EEPROM

void WriteJSON(bool apset, const char *ssid, const char *pwd, long bounce, long reset)
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

	String jsonText;
	root.prettyPrintTo(jsonText);

	DEBUG(DEBUG_VERBOSE, Serial.printf("JSON : %s\n\r",jsonText.c_str()));

	json.write((byte*)jsonText.c_str(),jsonText.length());
	
	json.close();
}

void ReadJSON()
{
	DEBUG(DEBUG_IMPORTANT, Serial.println("ReadJSON"));

#ifdef _ERASE_JSON_CONFIG
	SPIFFS.remove(_JSON_CONFIG_FILE);
#endif

	if (!SPIFFS.exists(_JSON_CONFIG_FILE))
	{
		DEBUG(DEBUG_IMPORTANT, Serial.println("Config.json does not exist"));
		// file does not exist
		WriteJSON(false, NULL, NULL, BOUNCE_TIMEOUT_DEFAULT, QUICK_SWITCH_TIMEOUT_DEFAULT);

		return;

	}
	// first - see if the file is there
	fs::File json = SPIFFS.open(_JSON_CONFIG_FILE, "r");

	String jsonString=json.readString();

	DEBUG(DEBUG_INFO, Serial.printf("JSON: %s\n\r",jsonString.c_str()));

	StaticJsonBuffer<1024> jsonBuffer;

	JsonObject& root = jsonBuffer.parseObject(jsonString);
	
	if (!root.success())
	{
		DEBUG(DEBUG_ERROR, Serial.println("JSON parse failed"));
		WriteJSON(false, NULL, NULL, Details.debounceThresholdms, Details.resetWindowms);
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
	WriteJSON(false, NULL, NULL, Details.debounceThresholdms, Details.resetWindowms);
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

	ReadJSON();

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


	String boilerPlate("<html><head><style type=\"text/css\">$$$STYLE$$$</style></head><body>$$BODY$$</body></html>");
	String style("body {font: Arial; font-size: 20px;} .big{height:20px;width:100px;}");

	boilerPlate.replace("$$$STYLE$$$", style);


	// sure, this could be prettier
	webPageAPtry = webPageAP = "<h1>"+ esphostname +"</h1>";

	// set the handler string up
	String webPageSTAbody(webPageAPtry);

	for (int socket = 0; socket < NUM_SOCKETS; socket++)
	{
		webPageSTAbody += String("<p>Socket ")+String(socket)+String("<a href=\"button?action=on&port=") +String(socket)+ String("\"><button class='big'>ON</button></a>&nbsp;<a href=\"button?action=off&port=") + String(socket) + String("\"><button  class='big'>OFF</button></a></p>");
	}

	webPageSTAbody += String("<p><a href='revert'>Revert All</a></p>");
	webPageSTAbody += String("<p><a href='all?action=off'>All OFF</a></p>");
	webPageSTAbody += String("<p><a href='all?action=on'>All ON</a></p>");

	String webPageSTA = boilerPlate;
	webPageSTA.replace("$$BODY$$", webPageSTAbody);


//	webPageSTA += "<p>Socket<a href=\"socket2On\"><button>ON</button></a>&nbsp;<a href=\"socket2Off\"><button>OFF</button></a></p>";

	char noise[25];
	webPageAP += "<form action = \"associate\">SSID:<input name = ssid size = 15 type = text /><br/>PWD : <input name = psk size = 15 type = text / ><br/><hr/>";
	webPageAP += "Bounce : <input name = bounce size = 15 type =number value="+String(ltoa(Details.debounceThresholdms, noise, 10))+" />ms<br/>";
	webPageAP += "Reset : <input name = reset size = 15 type =number value="+String(ltoa(Details.resetWindowms, noise, 10))+" />ms<br/>";
	webPageAP += "<input name=Submit type = submit value = \"join\" /></form>";

	ltoa(Details.resetWindowms, noise, 10);

	webPageAPtry += "trying";


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
	
		DEBUG(DEBUG_IMPORTANT, Serial.println("EVENT wifi connected"));
		//Serial.println(c.ssid);

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

	server.on("/revert", [webPageSTA]() {

		RevertAllSwitch();

 		server.send(200, "text/html", webPageSTA);
	});

	server.on("/all", [webPageSTA]() {

		for (uint8_t i = 0; i < server.args(); i++)
		{
			if (server.argName(i) == "action")
			{
				DoAllSwitch(server.arg(i) == "on"?true:false,true);
			}
		}

		server.send(200, "text/html", webPageSTA);
	});



	server.on("/button", [webPageSTA]() {

		String action, port;

		for (uint8_t i = 0; i < server.args(); i++)
		{
			if (server.argName(i) == "port")
			{
				port = server.arg(i).c_str();
			}
			if (server.argName(i) == "action")
			{
				action = server.arg(i).c_str();
			}

		}

		if (action.length() && port.length())
		{
			DoSwitch(port.toInt(), action == "on" ? true : false, true);
		}

		server.send(200, "text/html", webPageSTA);

	});


	// handlers can't be replaced
	server.on("/", [webPageSTA]() {
		switch (currentMode)
		{
		case wifiMode::modeAP:
			server.send(200, "text/html", webPageAP);
			break;
		case wifiMode::modeSTA:
			server.send(200, "text/html", webPageSTA);
			break;
		case wifiMode::modeUnknown:
			break;

		}

	});


	server.on("/join", []() {
		server.send(200, "text/html", webPageAP);
	});

	server.on("/associate", []() {
		server.send(200, "text/html", webPageAPtry);

		String ssid, pwd;
		long bounce = 250, reset = 3000;

		// get the args
		if (currentMode==wifiMode::modeAP && server.args() > 0) {
			for (uint8_t i = 0; i < server.args(); i++) 
			{
				if (server.argName(i) == "ssid") 
				{
					ssid= server.arg(i).c_str();
				}
				if (server.argName(i) == "psk")
				{
					pwd=server.arg(i).c_str();
				}

				if (server.argName(i) == "bounce")
					bounce = atol(server.arg(i).c_str());

				if (server.argName(i) == "reset")
					reset = atol(server.arg(i).c_str());

			}
		} 
#ifdef _USE_JSON_NOT_EEPROM
		WriteJSON(true, ssid.c_str(), pwd.c_str(), bounce, reset);
#else
		WriteEeprom(true, ssid.c_str(), pwd.c_str(), bounce, reset);
#endif

		// force attempt
		ConnectWifi(wifiMode::modeSTA);

		delay(1000);
	});

}

void loop(void) 
{
#ifdef _RESET_VIA_QUICK_SWITCH
	if (resetWIFI)
		ResetMe();
#endif

	
	server.handleClient();
}