// mcp23017.h

#ifndef _MCP23017_h
#define _MCP23017_h

#include "Arduino.h"
#include <Wire.h>
#include <debugLogger.h>

// the address of the MCP
#define MCPADDR	0x20

#define MCP_IODIR_A		0x00
#define MCP_IODIR_B		0x01
#define MCP_IPOL_A		0x02
#define MCP_GPINTE_A	0x04
#define MCP_GPINTE_B	0x05
#define MCP_DEFVAL_A	0x06
#define MCP_INTCON_A	0x08
#define MCP_IOCAN_A		0x0a
#define MCP_GPPU_A		0x0C
#define MCP_INTF_A		0x0e
#define MCP_INTCAP_A	0x10
#define MCP_GPIO_A		0x12
#define MCP_GPIO_B		0x13

// can't get IPOL to work, so emulate in software
// #define _IPOL_IN_SOFTWARE




class mcp23017
{


public:
	mcp23017(debugBaseClass *dblog, int sdaPin, int sclPin, int resetPin):
#ifdef ESP32
		// check this bus number!
		m_wire(0),
#endif	
		m_resetAvailable(true), m_sdaPin(sdaPin), m_sclPin(sclPin), m_resetPin(resetPin),m_dblog(dblog)
	{
		commonConstruct();
	}

	mcp23017(debugBaseClass *dblog, int sdaPin, int sclPin) :
#ifdef ESP32
		// check this bus number!
		m_wire(0),
#endif	
		m_resetAvailable(false), m_sdaPin(sdaPin), m_sclPin(sclPin), m_resetPin(0),m_dblog(dblog)
	{
		commonConstruct();
	}

	// spin it up
	virtual void Initialise();

	// read switch state
	bool readSwitch(unsigned switchNumber);
	// set the output
	bool SetRelay(unsigned relayNumber, bool relayState);
	bool ToggleRelay(unsigned relayNumber);
	void SetSwitch(unsigned switchNumber, bool relayState);
	// find out what caused the isr
	unsigned QueryInterruptCauseAndCurrentState(bool justClearInterrupt);
	// get all the switches
	byte readAllSwitches(bool readInterrupt=false);
	// read the RELAY
	bool GetRelay(unsigned relayNumber, bool &relayState);
	// set all relays
	void setAllRelays(byte state);


protected:

	void commonConstruct()
	{
		m_wire.begin(m_sdaPin, m_sclPin);
		memset(&m_lastRelayMicros,0,sizeof(m_lastRelayMicros));

#ifdef _IPOL_IN_SOFTWARE
		m_polarity = 0;
#endif

	}

	byte readOneRegister(byte command);
	void writeOneRegister(byte command, byte theByte);
	void flipPolarityPort(unsigned port);

	bool m_resetAvailable;
	int m_sdaPin, m_sclPin, m_resetPin;

	debugBaseClass *m_dblog;

private:

#ifdef _IPOL_IN_SOFTWARE
	byte m_polarity;	// 0 is 1:1
#endif

	TwoWire m_wire;

	unsigned long m_lastRelayMicros[8];
};


// controls the mcp and the relay
class mcp23017AndRelay: public mcp23017
{
public:
	mcp23017AndRelay(debugBaseClass *dblog, int sdaPin, int sclPin, int intPin, int resetPin, int powerHubNPN):
		mcp23017(dblog, sdaPin, sclPin, resetPin),m_powerHubNPN(powerHubNPN)
	{
	
		// set up the xistor that looks after the relay
		pinMode(m_powerHubNPN, OUTPUT);
		// ensure relay is off
		digitalWrite(m_powerHubNPN, LOW);

	}


	virtual void Initialise()
	{
		// ensure relay is off
		digitalWrite(m_powerHubNPN, LOW);

		// initialise
		mcp23017::Initialise();

		// then turn the relay on
		digitalWrite(m_powerHubNPN, HIGH);
	}

private:

	int m_powerHubNPN;

};

#endif

