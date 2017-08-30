#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <EEPROM.h>
// for the mcp23017
#include <Wire.h>


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

// relay trigger OUT
//int outputRelayPin = 5;

// the address of the MCP
#define MCPADDR	0x20

// number of relays
#define NUM_SOCKETS	6

// current switch state
//bool switchState = false;

long last_micros = 0;

#define RESET_ARRAY_SIZE 6
long lastSwitchesSeen[RESET_ARRAY_SIZE];

#define MCP_IODIR_A		0x00
#define MCP_IODIR_B		0x01
#define MCP_GPINTE_A	0x04
#define MCP_GPINTE_B	0x05
#define MCP_INTCON_A	0x08
#define MCP_IOCAN_A		0x0a
#define MCP_GPPU_A		0x0C
#define MCP_INTF_A		0x0e
#define MCP_INTCAP_A	0x10
#define MCP_GPIO_A		0x12
#define MCP_GPIO_B		0x13




#define _IGNORE_BOUNCE_LOGIC	

void OnSwitchISR()
{
	// if we're up to our neck in something else (normally WIFI negotiation) ignore this
	if (busyDoingSomethingIgnoreSwitch)
	{
		Serial.println("	OnSwitchISR redundant");
		// ask what changed, clear interrupt
		readMCP23017_InterruptCauseAndState(true);
		return;
	}

	Serial.println("	OnSwitchISR in");

#ifdef _IGNORE_BOUNCE_LOGIC

	// ask what changed, clear interrupt
	int causeAndState = readMCP23017_InterruptCauseAndState(false);

	for (unsigned port = 0; port < 8; port++)
	{
		Serial.printf("Checking %04x\r\n", (1 << (port + 8)));
		// +8 to get HIBYTE
		if (causeAndState & (1 << (port + 8)))
			DoSwitch(port, (causeAndState & (1 << port)) ? true : false, false);
	}

#else

	// gate against messy tactile/physical switches
	if ((long)(micros() - last_micros) >= (Details.debounceThresholdms * 1000))
	{
		// move the last seens along
		memmove(&lastSwitchesSeen[0], &lastSwitchesSeen[1], sizeof(long)*RESET_ARRAY_SIZE -1 );

		// ask what changed
		int causeAndState = readMCP23017_InterruptCauseAndState();

		for (unsigned port = 0; port < 8; port++)
		{
			// +8 to get HIBYTE
			if (causeAndState & (1 << (port+8)))
				DoSwitch(port, (causeAndState & (1 << port))?true:false, false);
		}

		// remember the last 6 - i'm assuming we won't wrap
		lastSwitchesSeen[RESET_ARRAY_SIZE - 1] = last_micros = micros();

		if (lastSwitchesSeen[RESET_ARRAY_SIZE - 1] - lastSwitchesSeen[0] < Details.resetWindowms*1000)
		{
			resetWIFI = true;
		}
	}
#endif

	Serial.println("	OnSwitchISR out");

}

void DoAllSwitch(bool state)
{
	for (int port = 0; port < NUM_SOCKETS; port++)
		DoSwitch(port, state, false);
}

// do, portNumber is 0 thru 7
void DoSwitch(int portNumber, bool on, bool inisr)
{
	if (portNumber > 7 || portNumber < 0)
		return;

	Serial.println("DoSwitch IN");

	if(!inisr)
		Serial.printf("DoSwitch: port %d %s\r\n", portNumber, on?"ON":"off");

	writeMCP23017_OutputState(portNumber, on, inisr);

	Serial.println("DoSwitch OUT");

}


void WriteEeprom(bool apset,const char *ssid,const char *pwd, long bounce, long reset)
{
	Serial.println("Writing EEPROM");
	Serial.println(bounce);
	Serial.println(reset);


	if(Details.wifi.configured = apset)
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

// A is in, B is out
// return cause mask and state - mask is HIBYTE
int readMCP23017_InterruptCauseAndState(bool justClearInterrupt)
{
	if (justClearInterrupt)
	{
		// then get current interrupt states
		Wire.beginTransmission(MCPADDR);
		Wire.write(MCP_GPIO_A); // GPIOA register
		Wire.endTransmission();

		Wire.requestFrom(MCPADDR, 1); // request one byte of data
		byte state = Wire.read();

		return 0;

	}
	// work out what pin(s) caused the interrupt - this is essentially the mask
	Wire.beginTransmission(MCPADDR);
	Wire.write(MCP_INTF_A); // INTFA register
	Wire.endTransmission();

	Wire.requestFrom(MCPADDR, 1); // request one byte of data
	int cause = Wire.read();

	// then get current interrupt states
	Wire.beginTransmission(MCPADDR);
	Wire.write(MCP_INTCAP_A); // INTCAPA register
	Wire.endTransmission();

	Wire.requestFrom(MCPADDR, 1); // request one byte of data
	int state = Wire.read();

	Serial.printf("MCPInt cause %02x state %02x [%04x]\n\r", cause, state, (int)((cause & 0xff) << 8) | (state & 0xff));

	// then send that back
	return (int)((cause&0xff) << 8) | (state&0xff);
}


// A is in, B is out
void writeMCP23017_OutputState(unsigned portNumber, bool portstate, bool inisr)
{

	Wire.beginTransmission(MCPADDR);
	Wire.write(MCP_GPIO_B); // GPIOB register
	Wire.endTransmission();

	Wire.requestFrom(MCPADDR, 1); // request one byte of data
	byte state = Wire.read();

	// get the existing state, mask out the bit we want
	state = ((state& (~(1 << portNumber)))) & 0xff;

	// then set the bit we're after (hi os OFF)
	if(!portstate)
		state = state | (1 << portNumber);

	Wire.beginTransmission(MCPADDR);
	Wire.write(MCP_GPIO_B); // GPIOB register
	Wire.write(state);
	Wire.endTransmission();

}

// port num = 0 thru 7
bool readMCP23017_InputState(unsigned portNumber)
{
	Wire.beginTransmission(MCPADDR);
	//Wire.write(0x0e); // INTFA register
	Wire.write(MCP_GPIO_A); // GPIOA register
	Wire.endTransmission();

	Wire.requestFrom(MCPADDR, 1); // request one byte of data
	byte state = Wire.read();

	return state & (1<<portNumber) ? true : false;
}

//#define MCP_BASIC

// A is in, B is out
void SetupMCP23017()
{

	// set up state
	Wire.beginTransmission(MCPADDR);
	Wire.write(MCP_IOCAN_A); // IOCON register
	Wire.write(0x20 | 0x8); // BANK0(0) MIRROR0(0) SEQOPoff(20) DISSLWoff(0) HAENoff(8) ODRoff(0) INTPOLoff(0)
	Wire.endTransmission();

	// we are going to use A as INs, pullup
	{
		Wire.beginTransmission(MCPADDR);
		Wire.write(MCP_IODIR_A); // IODIRA register
		Wire.write(0xff); // set all of port A to inputs
		Wire.endTransmission();

		Wire.beginTransmission(MCPADDR);
		Wire.write(MCP_GPPU_A); // GPPUA register
		Wire.write(0xff); // set all of port A to pullup
		Wire.endTransmission();

#ifndef MCP_BASIC

		// set interrupt to spot A changing
		{

			Wire.beginTransmission(MCPADDR);
			Wire.write(0x06); // DEFVALA register
			Wire.write(0x00); // intcona makes this redundant
			Wire.endTransmission();

			Wire.beginTransmission(MCPADDR);
			Wire.write(MCP_INTCON_A); // INTCONA register
			Wire.write(0x00); // change from previous state
			Wire.endTransmission();

			Wire.beginTransmission(MCPADDR);
			Wire.write(MCP_GPINTE_A); // GPINTENA register
			Wire.write(0xff); // all signal interrupt
			Wire.endTransmission();

		}

#else



#endif

	}
	
	Wire.beginTransmission(MCPADDR);
	Wire.write(MCP_IODIR_B); // IODIRB register
	Wire.write(0x00); // set all of port B to outputs
	Wire.endTransmission();

	Wire.beginTransmission(MCPADDR);
	Wire.write(MCP_GPINTE_B); // GPINTENA register
	Wire.write(0x0); // NO signal interrupt
	Wire.endTransmission();




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

	// start wire
	Wire.begin(4, 5);


	// start eeprom library
	EEPROM.begin(512);

#ifdef _INITIALISE_EEPROM
	WriteEeprom(false, NULL, NULL, 250,3000);
#endif

	// try reading the eeprom
	// the marker is to spot virgin EEPROM (can i vape it on a build?)
	EEPROM.get(0, Details);

	String boilerPlate("<html><head><style type=\"text/css\">$$$STYLE$$$</style></head><body>$$BODY$$</body></html>");
	String style("body {font: Arial; font-size: 2.5em;}");

	boilerPlate.replace("$$$STYLE$$$", style);


	// sure, this could be prettier
	webPageAPtry = webPageAP = "<h1>"+ esphostname +"</h1>";

	// set the handler string up
	String webPageSTAbody(webPageAPtry);

	for (int socket = 0; socket < NUM_SOCKETS; socket++)
	{
		webPageSTAbody += String("<p>Socket ")+String(socket)+String("<a href=\"button?action=on&port=") +String(socket)+ String("\"><button>ON</button></a>&nbsp;<a href=\"button?action=off&port=") + String(socket) + String("\"><button>OFF</button></a></p>");
	}

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


	// preparing GPIOs
	pinMode(inputSwitchPin, INPUT_PULLUP);
	attachInterrupt((inputSwitchPin), OnSwitchISR, ONLOW);

	// initialise the MCP
	SetupMCP23017();

	// default on
	DoAllSwitch(true);

	// try to connect to the wifi
	ConnectWifi(intent);

	// default off
	DoAllSwitch(false);

	server.on("/button", [webPageSTA]() {
		server.send(200, "text/html", webPageSTA);

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
			DoSwitch(port.toInt(), action == "on" ? true : false, false);
		}

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