// mcp23017.h

#ifndef _MCP23017_h
#define _MCP23017_h

#include "Arduino.h"
#include <Wire.h>

// the address of the MCP
#define MCPADDR	0x20

#define MCP_IODIR_A		0x00
#define MCP_IODIR_B		0x01
#define MCP_IOPOL_A		0x02
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


class mcp23017
{
public:
	mcp23017(int sdaPin, int sclPin, int resetPin):
		m_resetAvailable(true), m_sdaPin(sdaPin), m_sclPin(sclPin), m_resetPin(resetPin)
	{
		m_wire.begin(sdaPin, sclPin);
	}

	mcp23017(int sdaPin, int sclPin) :
		m_resetAvailable(false), m_sdaPin(sdaPin), m_sclPin(sclPin), m_resetPin(0)
	{
		m_wire.begin(sdaPin, sclPin);
	}

	// spin it up
	void Initialise();

	// read switch state
	bool readSwitch(unsigned switchNumber);
	// set the output
	void SetRelay(unsigned relayNumber, bool relayState, bool forceSwitchToReflect);
	// find out what caused the isr
	int InterruptCauseAndCurrentState(bool justClearInterrupt);


protected:

	bool m_resetAvailable;
	int m_sdaPin, m_sclPin, m_resetPin;

private:

	TwoWire m_wire;
};


#endif
