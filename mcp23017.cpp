// 
// 
// 

#include "mcp23017.h"


void mcp23017::Initialise()
{
	if(m_resetAvailable)
	{
		// hold the reset pin down (0v reset)
		pinMode(m_resetPin, OUTPUT);
		digitalWrite(m_resetPin, LOW);
		delay(100);
		// high - fires the xistor
		digitalWrite(m_resetPin, HIGH);
		delay(100);
	}


	// set up state
	m_wire.beginTransmission(MCPADDR);
	m_wire.write(MCP_IOCAN_A); // IOCON register
	m_wire.write(0x20 | 0x8); // BANK0(0) MIRROR0(0) SEQOPoff(20) DISSLWoff(0) HAENoff(8) ODRoff(0) INTPOLoff(0)
	m_wire.endTransmission();

	// we are going to use A as INs, pullup
	{
		m_wire.beginTransmission(MCPADDR);
		m_wire.write(MCP_IODIR_A); // IODIRA register
		m_wire.write(0xff); // set all of port A to inputs
		m_wire.endTransmission();

		m_wire.beginTransmission(MCPADDR);
		m_wire.write(MCP_GPPU_A); // GPPUA register
		m_wire.write(0xff); // set all of port A to pullup
		m_wire.endTransmission();

		// set interrupt to spot A changing
		{

			// turn polarity off
			m_wire.beginTransmission(MCPADDR);
			m_wire.write(MCP_IOPOL_A); // IPOLA register
			m_wire.write(0x00); // set all of port A to 1:1
			m_wire.endTransmission();

			m_wire.beginTransmission(MCPADDR);
			m_wire.write(MCP_DEFVAL_A); // DEFVALA register
			m_wire.write(0x00); // intcona makes this redundant
			m_wire.endTransmission();

			m_wire.beginTransmission(MCPADDR);
			m_wire.write(MCP_INTCON_A); // INTCONA register
			m_wire.write(0x00); // change from previous state
			m_wire.endTransmission();

			m_wire.beginTransmission(MCPADDR);
			m_wire.write(MCP_GPINTE_A); // GPINTENA register
			m_wire.write(0xff); // all signal interrupt
			m_wire.endTransmission();

		}

	}

	m_wire.beginTransmission(MCPADDR);
	m_wire.write(MCP_IODIR_B); // IODIRB register
	m_wire.write(0x00); // set all of port B to outputs
	m_wire.endTransmission();

	m_wire.beginTransmission(MCPADDR);
	m_wire.write(MCP_GPINTE_B); // GPINTENA register
	m_wire.write(0x0); // NO signal interrupt
	m_wire.endTransmission();


}


bool mcp23017::readSwitch(unsigned switchNumber)
{
	m_wire.beginTransmission(MCPADDR);
	//m_wire.write(0x0e); // INTFA register
	m_wire.write(MCP_GPIO_A); // GPIOA register
	m_wire.endTransmission();

	m_wire.requestFrom(MCPADDR, 1); // request one byte of data
	byte state = m_wire.read();

	return state & (1 << switchNumber) ? true : false;

}

void mcp23017::SetRelay(unsigned relayNumber, bool relayState, bool forceSwitchToReflect)
{
	m_wire.beginTransmission(MCPADDR);
	m_wire.write(MCP_GPIO_B); // GPIOB register
	m_wire.endTransmission();

	m_wire.requestFrom(MCPADDR, 1); // request one byte of data
	byte state = m_wire.read();

	// get the existing state *of the relay*, mask out the bit we want
	state = ((state& (~(1 << relayNumber)))) & 0xff;

	// then set the bit we're after (hi is OFF)
	if (!relayState)
		state = state | (1 << relayNumber);

	// if we have to force the switch to relect the current decision
	if (forceSwitchToReflect)
	{
		// get the switch state for this port
		bool switchState = readSwitch(relayNumber);

		// thw switch does NOT mirror the request state
		if (switchState != relayState)
		{
			Serial.println("switch does NOT reflect request - asked to alter");

			// get polarity of A
			m_wire.beginTransmission(MCPADDR);
			m_wire.write(MCP_IOPOL_A); // IOPOLA register
			m_wire.endTransmission();

			m_wire.requestFrom(MCPADDR, 1); // request one byte of data
			byte polarity = m_wire.read();

			Serial.printf("%02x -> ", polarity);

			// flip the polarity bit for that switch
			polarity ^= (1 << relayNumber);

			Serial.printf("%02x\n\r", polarity);

			//m_wire.beginTransmission(MCPADDR);
			//m_wire.write(MCP_IOPOL_A); // IOPOLA register
			//m_wire.write(polarity);
			//m_wire.endTransmission();

		}


	}

	m_wire.beginTransmission(MCPADDR);
	m_wire.write(MCP_GPIO_B); // GPIOB register
	m_wire.write(state);
	m_wire.endTransmission();
}

int mcp23017::InterruptCauseAndCurrentState(bool justClearInterrupt)
{
	if (justClearInterrupt)
	{
		// then get current interrupt states
		m_wire.beginTransmission(MCPADDR);
		m_wire.write(MCP_GPIO_A); // GPIOA register
		m_wire.endTransmission();

		m_wire.requestFrom(MCPADDR, 1); // request one byte of data
		byte state = m_wire.read();

		return 0;

	}
	// work out what pin(s) caused the interrupt - this is essentially the mask
	m_wire.beginTransmission(MCPADDR);
	m_wire.write(MCP_INTF_A); // INTFA register
	m_wire.endTransmission();

	m_wire.requestFrom(MCPADDR, 1); // request one byte of data
	int cause = m_wire.read();

	// then get current interrupt states
	m_wire.beginTransmission(MCPADDR);
	m_wire.write(MCP_INTCAP_A); // INTCAPA register
	m_wire.endTransmission();

	m_wire.requestFrom(MCPADDR, 1); // request one byte of data
	int state = m_wire.read();

	Serial.printf("MCPInt cause %02x state %02x [%04x]\n\r", cause, state, (int)((cause & 0xff) << 8) | (state & 0xff));

	// then send that back
	return (int)((cause & 0xff) << 8) | (state & 0xff);
}