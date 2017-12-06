// 
// 
// 

#include "mcp23017.h"

#include "debug_defines.h"


byte mcp23017::readOneRegister(byte command)
{
	m_wire.beginTransmission(MCPADDR);
	m_wire.write(command); // register
	m_wire.endTransmission();

	m_wire.requestFrom(MCPADDR, 1); // request one byte of data
	byte state = m_wire.read();

	return state;
}

void mcp23017::writeOneRegister(byte command, byte theByte)
{
	m_wire.beginTransmission(MCPADDR);
	m_wire.write(command); // register
	m_wire.write(theByte);
	m_wire.endTransmission();

}

void mcp23017::Initialise()
{
	if(m_resetAvailable)
	{
		// hold the reset pin down (0v reset)
		pinMode(m_resetPin, OUTPUT);
		digitalWrite(m_resetPin, LOW);
		delay(500);
		// high - fires the xistor
		digitalWrite(m_resetPin, HIGH);
		delay(100);
	}

	writeOneRegister(MCP_IOCAN_A, 0x20 | 0x8); // BANK0(0) MIRROR0(0) SEQOPoff(20) DISSLWoff(0) HAENoff(8) ODRoff(0) INTPOLoff(0)

	// make all B ports output
	{
		writeOneRegister(MCP_GPIO_B, 0xff);// set all B ports (hi is off)

		writeOneRegister(MCP_IODIR_B, 0);// set all of port B to outputs

		writeOneRegister(MCP_GPINTE_B, 0);// NO signal interrupt
	}


	// we are going to use A as INs, pullup
	{
		writeOneRegister(MCP_IODIR_A,0xff); // set all of port A to inputs

		writeOneRegister(MCP_GPPU_A, 0xff);// set all of port A to pullup

		// set interrupt to spot A changing
		{

			// turn polarity off
			writeOneRegister(MCP_IPOL_A, 0);// set all of port A to 1:1

			writeOneRegister(MCP_DEFVAL_A, 0);// intcona makes this redundant

			writeOneRegister(MCP_INTCON_A, 0);// change from previous state

			writeOneRegister(MCP_GPINTE_A, 0xff);// all signal interrupt

			// and read them to clear any interrupt flags (to stop spurious ISRs when we attachInterrupt)
			readAllSwitches();
		}

	}
}

void mcp23017::flipPolarityPort(unsigned port)
{
	if (port > 7 || port < 0)
	{
		DEBUG(DEBUG_ERROR, Serial.printf("flipPolarityPort called out of bounds %u\n\r", port));
		return;
	}


#ifdef _IPOL_IN_SOFTWARE
	DEBUG(DEBUG_VERBOSE, Serial.printf("%02x -> ", m_polarity));
	m_polarity ^= ((1 << port));
	DEBUG(DEBUG_VERBOSE, Serial.printf("%02x ", m_polarity));
#else
	// get polarity of A
	byte polarity = readOneRegister(MCP_IPOL_A);

	DEBUG(DEBUG_IMPORTANT, Serial.printf("%02x -> ", polarity));

	// flip the polarity bit for that switch
	polarity ^= (1 << port);

	// enabling this line causes an ISR storm
	writeOneRegister(MCP_IPOL_A, polarity);

	DEBUG(DEBUG_IMPORTANT, Serial.printf("%02x ", polarity));

#endif
	DEBUG(DEBUG_VERBOSE, Serial.println("written"));

}


byte mcp23017::readAllSwitches(bool readInterrupt)
{
	byte state = readInterrupt? readOneRegister(MCP_INTCAP_A) :readOneRegister(MCP_GPIO_A);

#ifdef _IPOL_IN_SOFTWARE

	// flip the bits according to the polarity
	for (int bits = 0; bits < 8; bits++)
	{
		if (m_polarity & (1 << bits))
			state ^= ((1 << bits));
	}

#endif

	return state;
}


bool mcp23017::readSwitch(unsigned switchNumber)
{
	if (switchNumber > 7 || switchNumber < 0)
	{
		DEBUG(DEBUG_ERROR, Serial.printf("readSwitch called out of bounds %u\n\r", switchNumber));
		return false;
	}


	byte state = readAllSwitches();
	return state & (1 << switchNumber) ? true : false;
}

#define _MIN_TIME_BETWEEN_SWITCHES (100*1000)

void mcp23017::SetSwitch(unsigned switchNumber, bool relayState)
{
	// get the switch state for this port
	bool switchState = readSwitch(switchNumber);

	DEBUG(DEBUG_VERBOSE, Serial.printf("port %d switchState = %02x\n\r", switchNumber, switchState));

	// thw switch does NOT mirror the request state
	if (switchState != relayState)
	{
		flipPolarityPort(switchNumber);
	}

}


void mcp23017::SetRelay(unsigned relayNumber, bool relayState)
{
	if (relayNumber > 7 || relayNumber < 0)
	{
		DEBUG(DEBUG_ERROR, Serial.printf("SetRelay called out of bounds %u\n\r", relayNumber));
		return;
	}

	// first, let's insure we haven't been here, for this switch, too soon
	unsigned long timediff = micros()-m_lastRelayMicros[relayNumber];
	if (timediff < _MIN_TIME_BETWEEN_SWITCHES)
	{
		DEBUG(DEBUG_ERROR, Serial.printf("SetRelay called too soon for switch %u\n\r", relayNumber));
		return;
	}

	m_lastRelayMicros[relayNumber] = micros();

	byte state = readOneRegister(MCP_GPIO_B);

	// get the existing state *of the relay*, mask out the bit we want
	state = ((state& (~(1 << relayNumber)))) & 0xff;

	// then set the bit we're after (hi is OFF)
	if (!relayState)
		state = state | (1 << relayNumber);

	writeOneRegister(MCP_GPIO_B, state);
}

int mcp23017::InterruptCauseAndCurrentState(bool justClearInterrupt)
{
	if (justClearInterrupt)
	{
		return readAllSwitches(true);
	}

	byte cause = readOneRegister(MCP_INTF_A);

	// then get current states
	byte state= readAllSwitches(true);

	DEBUG(DEBUG_VERBOSE, Serial.printf("MCPInt cause %02x state %02x [%04x]\n\r", cause, state, (int)((cause & 0xff) << 8) | (state & 0xff)));

	// then send that back
	return (int)((cause & 0xff) << 8) | (state & 0xff);
}
