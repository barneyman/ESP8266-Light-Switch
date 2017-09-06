#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <EEPROM.h>

#include "mcp23017.h"


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
		char ssid[128];
		char password[128];
		bool configured;

	} wifi;

	// how long to wait for the light switch to settle
	long debounceThresholdms;

	// 6 switches in this time force an AP reset
	long resetWindowms;

} Details;

// needs to be persisted or the event is unsubscribed
WiFiEventHandler onConnect, onDisconnect;


volatile bool busyDoingSomethingIgnoreSwitch = false;

String esphostname = "esp8266_";


enum wifiMode { modeAP, modeSTA, modeUnknown } ;
wifiMode currentMode = modeUnknown;

bool resetWIFI = false;

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

long last_micros[NUM_SOCKETS];

#define RESET_ARRAY_SIZE 6
long lastSwitchesSeen[NUM_SOCKETS][RESET_ARRAY_SIZE];


#define _IGNORE_BOUNCE_LOGIC	

void OnSwitchISR()
{
	// if we're up to our neck in something else (normally WIFI negotiation) ignore this
	if (busyDoingSomethingIgnoreSwitch)
	{
		Serial.println("	OnSwitchISR redundant");
		// ask what changed, clear interrupt
		mcp.InterruptCauseAndCurrentState(true);
		return;
	}

	Serial.println("	OnSwitchISR in");


	// ask what changed, clear interrupt
	int causeAndState =
		mcp.InterruptCauseAndCurrentState(false);

	for (unsigned port = 0; port < NUM_SOCKETS; port++)
	{
		Serial.printf("Checking port %d\r\n",port);
		// +8 to get HIBYTE to see if this port CAUSED the interrupt
		if (causeAndState & (1 << (port + 8)))
		{

#ifndef _IGNORE_BOUNCE_LOGIC
			// gate against messy tactile/physical switches
			if ((long)(micros() - last_micros[port]) >= (Details.debounceThresholdms * 1000))
			{
				// move the last seens along
				memmove(&lastSwitchesSeen[port][0], &lastSwitchesSeen[port][1], sizeof(long)*RESET_ARRAY_SIZE - 1);

				Serial.printf("lastSwitchesSeen ");

				for (int each = 0; each < NUM_SOCKETS; each++)
				{
					Serial.printf("%lx ", lastSwitchesSeen[port][each]);
				}

				Serial.printf("\n\r");


#else
				// having CAUSED the interrupt, reflect its STATE in the DoSwitch call
				DoSwitch(port, (causeAndState & (1 << port)) ? true : false, false);

#endif

#ifndef _IGNORE_BOUNCE_LOGIC

				// remember the last 6 - i'm assuming we won't wrap
				lastSwitchesSeen[port][RESET_ARRAY_SIZE - 1] = last_micros[port] = micros();

				if (lastSwitchesSeen[port][RESET_ARRAY_SIZE - 1] - lastSwitchesSeen[port][0] < Details.resetWindowms * 1000)
				{
					Serial.println("RESETTING WIFI!\n\r");
					resetWIFI = true;
				}
			}
#endif
		}
	}




	Serial.println("	OnSwitchISR out");

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
void DoAllSwitch(bool state)
{
	for (int port = 0; port < NUM_SOCKETS; port++)
		DoSwitch(port, state, true);
}

// do, portNumber is 0 thru 7
// if forceSwitchToReflect change polarity of input switch if necessary to reflect this request
void DoSwitch(int portNumber, bool on, bool forceSwitchToReflect)
{
	if (portNumber > 7 || portNumber < 0)
		return;

	Serial.printf("DoSwitch: port %d %s %s\r\n", portNumber, on?"ON":"off", forceSwitchToReflect?"FORCE":"");

	mcp.SetRelay(portNumber, on, forceSwitchToReflect);

}


void WriteEeprom(bool apset,const char *ssid,const char *pwd, long bounce, long reset)
{
	Serial.println("Writing EEPROM");
	Serial.println(bounce);
	Serial.println(reset);

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


void BeginWebServer()
{
	server.begin();
	Serial.println("HTTP server started");
}

// disjoin and rejoin, optionally force a STA attempt
void ConnectWifi(wifiMode intent)
{
	busyDoingSomethingIgnoreSwitch = true;

	WiFi.persistent(false);

	Serial.println("ConnectWifi");

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

	Serial.println("disconnected");


	if (intent == wifiMode::modeSTA)
	{
		currentMode = wifiMode::modeSTA;

		// turn bonjour off??
		Serial.print("Attempting connect to ");
		Serial.println(Details.wifi.ssid);

		WiFi.mode(WIFI_STA);
		WiFi.begin(Details.wifi.ssid, Details.wifi.password);

		// Wait for connection
		for(int attempts=0;attempts<15;attempts++)
		{
			if (WiFi.status() != WL_CONNECTED) 
			{
				delay(1000);
				// flash the light, simply as feedback
				// ToggleSwitch();
				Serial.print(".");
			}
			else
				break;
		}

		if (WiFi.status() == WL_CONNECTED)
		{

			Serial.println("");
			Serial.print("Connected to ");
			Serial.println(Details.wifi.ssid);
			Serial.print("IP address: ");
			Serial.println(WiFi.localIP());

			if (mdns.begin(esphostname.c_str(), WiFi.localIP()))
			{
				Serial.println("MDNS responder started");
			}

			WiFi.setAutoReconnect(true);

			BeginWebServer();
		}
	}

	if (intent == wifiMode::modeAP)
	{
		// defaults to 192.168.4.1
		Serial.println("Attempting to start AP");

		// we were unable to connect, so start our own access point
		WiFi.mode(WIFI_AP);
		WiFi.softAP(esphostname.c_str());

		Serial.println("Started hostname AP");
		Serial.println(WiFi.softAPIP().toString());
	
		currentMode = wifiMode::modeAP;


	}

	busyDoingSomethingIgnoreSwitch = false;

}

// if we see more than x switches in y time, we reset the flash and enter AP mode (so we can be joined to another wifi network)

void ResetMe()
{
	Serial.println("resetting");

	resetWIFI = false;
	// clear the credentials
	WriteEeprom(false, NULL, NULL, Details.debounceThresholdms, Details.resetWindowms);

	// and reconnect as an AP
	ConnectWifi(wifiMode::modeAP);

}




// run it ONCE with this flag set, just to write sane values into the EEPROM
//#define _INITIALISE_EEPROM

void setup(void) 
{
	char idstr[20];
	sprintf(idstr,"%0x", system_get_chip_id());
	esphostname += idstr;

	// clean up the switch times
	memset(&lastSwitchesSeen, 0, sizeof(lastSwitchesSeen));

	// reset the bounce thresh-holds
	for (int eachSwitch = 0; eachSwitch < NUM_SOCKETS; eachSwitch++)
	{
		last_micros[eachSwitch] = 0;
	}

	// start eeprom library
	EEPROM.begin(512);

#ifdef _INITIALISE_EEPROM
	WriteEeprom(false, NULL, NULL, 250,3000);
#endif

	// try reading the eeprom
	// the marker is to spot virgin EEPROM (can i vape it on a build?)
	EEPROM.get(0, Details);

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

	// mandatory "let it settle" delay
	delay(1000);
	Serial.begin(115200);
	Serial.println("starting");
	Serial.println(esphostname);
	Serial.print("bounce "); Serial.println(Details.debounceThresholdms);
	Serial.print("reset "); Serial.println(Details.resetWindowms);

	enum wifiMode intent = wifiMode::modeUnknown;

	if (Details.wifi.configured)
	{
		Serial.println("credentials found");
		Serial.println(Details.wifi.ssid);
		Serial.println(Details.wifi.password);
		intent = wifiMode::modeSTA;
	}
	else
	{
		Serial.println("no stored credentials");
		intent = wifiMode::modeAP;

	}


	// set callbacks for wifi
	onConnect=WiFi.onStationModeConnected([](const WiFiEventStationModeConnected&c) {
	
		Serial.print("EVENT connected ");
		//Serial.println(c.ssid);

	});

	onDisconnect=WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &c) {
	
		Serial.print("EVENT disconnected ");
		//Serial.println(c.ssid);

	});

	// initialise the MCP
	mcp.Initialise();


	// preparing GPIOs
	pinMode(inputSwitchPin, INPUT_PULLUP);
	// the MCP interrupt is configured to fire on change, and goes LOW when fired
	attachInterrupt(inputSwitchPin, OnSwitchISR, ONLOW);


	// default on
	DoAllSwitch(false);

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
				DoAllSwitch(server.arg(i) == "on"?true:false);
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

		WriteEeprom(true, ssid.c_str(), pwd.c_str(), bounce, reset);

		// force attempt
		ConnectWifi(wifiMode::modeSTA);

		delay(1000);
	});

}

void loop(void) 
{
	if (resetWIFI)
		ResetMe();

	
	server.handleClient();
}