// mcp23017.h

#ifndef _MCP23017_h
#define _MCP23017_h

#include "Arduino.h"
#include <Wire.h>

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
#define _IPOL_IN_SOFTWARE




class mcp23017
{
public:
	mcp23017(int sdaPin, int sclPin, int resetPin):
		m_resetAvailable(true), m_sdaPin(sdaPin), m_sclPin(sclPin), m_resetPin(resetPin)
	{
		m_wire.begin(sdaPin, sclPin);
		memset(&m_lastRelayMicros,0,sizeof(m_lastRelayMicros));

#ifdef _IPOL_IN_SOFTWARE
		m_polarity = 0;
#endif
	}

	mcp23017(int sdaPin, int sclPin) :
		m_resetAvailable(false), m_sdaPin(sdaPin), m_sclPin(sclPin), m_resetPin(0)
	{
		m_wire.begin(sdaPin, sclPin);

#ifdef _IPOL_IN_SOFTWARE
		m_polarity = 0;
#endif
		memset(&m_lastRelayMicros, 0, sizeof(m_lastRelayMicros));
	}

	// spin it up
	virtual void Initialise();

	// read switch state
	bool readSwitch(unsigned switchNumber);
	// set the output
	void SetRelay(unsigned relayNumber, bool relayState, bool forceSwitchToReflect);
	// find out what caused the isr
	int InterruptCauseAndCurrentState(bool justClearInterrupt);
	// get all the switches
	byte readAllSwitches(bool readInterrupt=false);


protected:
	byte readOneRegister(byte command);
	void writeOneRegister(byte command, byte theByte);
	void flipPolarityPort(unsigned port);

	bool m_resetAvailable;
	int m_sdaPin, m_sclPin, m_resetPin;

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
	mcp23017AndRelay(int sdaPin, int sclPin, int resetPin, int powerHubNPN):mcp23017(sdaPin, sclPin, resetPin),m_powerHubNPN(powerHubNPN)
	{
	
		// set up the xistor that looks after the relay
		pinMode(m_powerHubNPN, OUTPUT);
		// ensure relay is off
		digitalWrite(m_powerHubNPN, 0);

	}


	virtual void Initialise()
	{
		// ensure relay is off
		digitalWrite(m_powerHubNPN, 0);

		// initialise
		mcp23017::Initialise();

		// then turn the relay on
		digitalWrite(m_powerHubNPN, 255);
	}

private:

	int m_powerHubNPN;

};

#endif

