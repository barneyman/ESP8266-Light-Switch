#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

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
int gpio0_pin = 0;

// relay trigger OUT
int gpio2_pin = 2;

// current switch state
bool switchState = false;

long last_micros = 0;

#define RESET_ARRAY_SIZE 6
long lastSwitchesSeen[RESET_ARRAY_SIZE];

// don't attempt to join the hosting WIFI for x seconds, because the router hasn't booted
//#define WAIT_FOR_HOST_WIFI_TO_BOOT_SECS 60

void OnSwitchISR()
{
	if (busyDoingSomethingIgnoreSwitch)
		return;

	// gate against messy tactile/physical switches
	if ((long)(micros() - last_micros) >= (Details.debounceThresholdms * 1000))
	{
		// move the last seens along
		memmove(&lastSwitchesSeen[0], &lastSwitchesSeen[1], sizeof(long)*RESET_ARRAY_SIZE -1 );

		// do the toggle
		ToggleSwitch();


		// remember the last 6 - i'm assuming we won't wrap
		lastSwitchesSeen[RESET_ARRAY_SIZE - 1] = last_micros = micros();

		if (lastSwitchesSeen[RESET_ARRAY_SIZE - 1] - lastSwitchesSeen[0] < Details.resetWindowms*1000)
		{
			resetWIFI = true;
		}

	}
}

/// just toggle
void ToggleSwitch()
{
	DoSwitch(!switchState);
}

// do, and remember
void DoSwitch(bool on)
{
	if (on)
	{
		// pin low is relay ON
		digitalWrite(gpio2_pin, LOW);
		Serial.println("switch on");
	}
	else
	{
		// pin high is relay OFF
		digitalWrite(gpio2_pin, HIGH);
		Serial.println("switch off");
	}

	switchState = on;

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
#define _INITIALISE_EEPROM

void setup(void) 
{
	char idstr[20];
	sprintf(idstr,"%0x", system_get_chip_id());
	esphostname += idstr;

	// clean up the switch times
	memset(&lastSwitchesSeen, 0, sizeof(lastSwitchesSeen));

	// start eeprom library
	EEPROM.begin(512);

#ifdef _INITIALISE_EEPROM
	WriteEeprom(false, NULL, NULL, 250,3000);
#endif

	// try reading the eeprom
	// the marker is to spot virgin EEPROM (can i vape it on a build?)
	EEPROM.get(0, Details);




	// sure, this could be prettier
	webPageAPtry = webPageAP = webPageSTA = "<h1>"+ esphostname +"</h1>";

	webPageSTA += "<p>Socket<a href=\"socket2On\"><button>ON</button></a>&nbsp;<a href=\"socket2Off\"><button>OFF</button></a></p>";

	char noise[25];
	webPageAP += "<form action = \"associate\">SSID:<input name = ssid size = 15 type = text /><br/>PWD : <input name = psk size = 15 type = text / ><br/><hr/>";
	webPageAP += "Bounce : <input name = bounce size = 15 type =number value="+String(ltoa(Details.debounceThresholdms, noise, 10))+" />ms<br/>";
	webPageAP += "Reset : <input name = reset size = 15 type =number value="+String(ltoa(Details.resetWindowms, noise, 10))+" />ms<br/>";
	webPageAP += "<input name=Submit type = submit value = \"join\" /></form>";


	;
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
	pinMode(gpio0_pin, INPUT_PULLUP);
	attachInterrupt((gpio0_pin), OnSwitchISR, CHANGE);

	// output
	pinMode(gpio2_pin, OUTPUT);
	digitalWrite(gpio2_pin, LOW);

	// default on
	DoSwitch(true);

	// wait 60 seconds for wifi network to come up
	// delay(WAIT_FOR_HOST_WIFI_TO_BOOT_SECS * 1000);

	// try to connect to the wifi
	ConnectWifi(intent);

	// default off
	DoSwitch(false);

	server.on("/socket2On", []() {
		server.send(200, "text/html", webPageSTA);
		DoSwitch(true);
		delay(1000);
	});
	server.on("/socket2Off", []() {
		server.send(200, "text/html", webPageSTA);
		DoSwitch(false);
		delay(1000);
	});

	// handlers can't be replaced
	server.on("/", []() {
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


	server.begin();
	Serial.println("HTTP server started");
}

void loop(void) 
{
	if (resetWIFI)
		ResetMe();

	//if (currentMode==wifiMode::modeSTA && WiFi.status() != WL_CONNECTED)
	//{
	//	ConnectWifi(currentMode);
	//}

	server.handleClient();
}