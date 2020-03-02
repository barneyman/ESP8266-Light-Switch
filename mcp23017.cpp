// 
// 
// 

#include "mcp23017.h"


byte mcp23017::readOneRegister(byte command)
{
	m_wire.beginTransmission(MCPADDR);
	m_wire.write(command); // register
	int res=m_wire.endTransmission();

	if(res)
	{
		m_dblog->printf(debug::dbError,"readOneRegister endTransmission returned %d\n\r", res);
		return 0;
	}

	// request one byte of data
	res=m_wire.requestFrom(MCPADDR, 1);
	if(1!=res)
	{
		m_dblog->printf(debug::dbError,"readOneRegister requestFrom returned %d\n\r", res);	
		return 0;
	}
	byte state = m_wire.read();

	return state;
}

void mcp23017::writeOneRegister(byte command, byte theByte)
{
	m_wire.beginTransmission(MCPADDR);
	m_wire.write(command); // register
	m_wire.write(theByte);

	int res=m_wire.endTransmission();

	if(res)
	{
		m_dblog->printf(debug::dbError,"writeOneRegister endTransmission returned %d\n\r", res);
		return;
	}

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

	// turn sequential reads off, and hardware address mod off
	writeOneRegister(MCP_IOCAN_A, 0x20 | 0x8); // BANK0(0) MIRROR0(0) SEQOPoff(20) DISSLWoff(0) HAENoff(8) ODRoff(0) INTPOLlo(0)

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

			// because inputs are pullup, their open state is high, so reverse the polarity
			writeOneRegister(MCP_IPOL_A, 0xff);// set all of port A to 1:1

			writeOneRegister(MCP_DEFVAL_A, 0);// intcona makes this redundant

			writeOneRegister(MCP_INTCON_A, 0);// interrupt on change from previous state

			writeOneRegister(MCP_GPINTE_A, 0x3f);// all 6 signal interrupt
			//writeOneRegister(MCP_GPINTE_A, 0);// all 6 signal interrupt

			// and read them to clear any interrupt flags (to stop spurious ISRs when we attachInterrupt)
			readAllSwitches(true);
		}

	}
}

void mcp23017::flipPolarityPort(unsigned port)
{
	if (port > 7 || port < 0)
	{
		m_dblog->printf(debug::dbError,"flipPolarityPort called out of bounds %u\n\r", port);
		return;
	}


#ifdef _IPOL_IN_SOFTWARE
	m_dblog->printf(debug::dbVerbose,"%02x -> ", m_polarity);
	m_polarity ^= ((1 << port));
	m_dblog->printf(debug::dbVerbose,"%02x ", m_polarity);
#else
	// get polarity of A
	byte polarity = readOneRegister(MCP_IPOL_A);

	m_dblog->printf(debug::dbInfo, "%02x -> ", polarity);

	// flip the polarity bit for that switch
	polarity ^= (1 << port);

	// enabling this line causes an ISR storm
	writeOneRegister(MCP_IPOL_A, polarity);

	m_dblog->printf(debug::dbInfo, "%02x ", polarity);

#endif
	m_dblog->printf(debug::dbVerbose,"written");

}


byte mcp23017::readAllSwitches(bool readInterrupt)
{
	m_dblog->println(debug::dbVerbose,"readAllSwitches");

	byte state = readInterrupt ? readOneRegister(MCP_INTCAP_A) : readOneRegister(MCP_GPIO_A);

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
		m_dblog->printf(debug::dbError,"readSwitch called out of bounds %u\n\r", switchNumber);
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

	m_dblog->printf(debug::dbVerbose,"port %d switchState = %02x\n\r", switchNumber, switchState);

	// thw switch does NOT mirror the request state
	if (switchState != relayState)
	{
		flipPolarityPort(switchNumber);
	}

}

bool mcp23017::ToggleRelay(unsigned relayNumber)
{
	bool currentState = false;
	if (GetRelay(relayNumber, currentState))
	{
		m_dblog->printf(debug::dbInfo,"ToggleRelay %u %s -> %s\n\r", relayNumber,currentState?"ON":"off", !currentState ? "ON" : "off");
		return SetRelay(relayNumber, !currentState);
	}

	return false;
}

bool mcp23017::GetRelay(unsigned relayNumber, bool &relayState)
{
	
	if (relayNumber > 7)
	{
		m_dblog->printf(debug::dbError,"GetRelay called out of bounds %u\n\r", relayNumber);
		return false;
	}

	byte state = readOneRegister(MCP_GPIO_B);

	m_dblog->printf(debug::dbInfo,"GetRelay state %02x\n\r", (int)state);

	// get the existing state *of the relay* HI is OFF
	relayState = ((state & (1 << relayNumber))&0xff) ? false : true;



	return true;
}


bool mcp23017::SetRelay(unsigned relayNumber, bool relayState)
{
	if (relayNumber > 7)
	{
		m_dblog->printf(debug::dbError,"SetRelay called out of bounds %u\n\r", relayNumber);
		return false;
	}

	// first, let's insure we haven't been here, for this switch, too soon
	unsigned long now = micros();
	unsigned long timediff = now-m_lastRelayMicros[relayNumber];
	if (timediff < _MIN_TIME_BETWEEN_SWITCHES)
	{
		m_dblog->printf(debug::dbError,"SetRelay called too soon for relay %u (%lums)\n\r", relayNumber, timediff/1000);
		return false;
	}

	m_lastRelayMicros[relayNumber] = now;

	byte state = readOneRegister(MCP_GPIO_B);

	m_dblog->printf(debug::dbInfo,"SetRelay - current state %02x\n\r", (int)state);

	// get the existing state *of the relay*, mask out the bit we want
	state = ((state& (~(1 << relayNumber)))) & 0xff;


	// then set the bit we're after (hi is OFF)
	if (!relayState)
		state = state | (1 << relayNumber);

	m_dblog->printf(debug::dbInfo,"SetRelay - new state %02x\n\r", (int)state);

	writeOneRegister(MCP_GPIO_B, state);

	// then confirm
	if(state!=readOneRegister(MCP_GPIO_B))
	{
		m_dblog->println(debug::dbError,"State did not stick!");	
	}


	return true;
}

void mcp23017::setAllRelays(byte state)
{
	writeOneRegister(MCP_GPIO_B, state);
}

unsigned mcp23017::QueryInterruptCauseAndCurrentState(bool justClearInterrupt)
{
	if (justClearInterrupt)
	{
		return readAllSwitches(true);
	}

	byte cause = readOneRegister(MCP_INTF_A);

	// then get current states
	byte state= readAllSwitches(true);

	unsigned retval=((cause) << 8) | (state);

	m_dblog->printf(debug::dbInfo,"MCPInt cause %02x state %02x [%04x]\n\r", cause, state, retval);

	// then send that back
	return retval;
}
