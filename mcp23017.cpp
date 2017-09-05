// 
// 
// 

#include "mcp23017.h"

byte mcp23017::readOneRegister(byte command)
{
	m_wire.beginTransmission(MCPADDR);
	m_wire.write(command); // GPIOB register
	m_wire.endTransmission();

	m_wire.requestFrom(MCPADDR, 1); // request one byte of data
	byte state = m_wire.read();

	return state;
}

void mcp23017::writeOneRegister(byte command, byte theByte)
{
	m_wire.beginTransmission(MCPADDR);
	m_wire.write(command); // GPIOB register
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

	writeOneRegister(MCP_IODIR_B, 0);// set all of port B to outputs

	writeOneRegister(MCP_GPINTE_B, 0);// NO signal interrupt
}

byte mcp23017::readAllSwitches()
{
	byte state = readOneRegister(MCP_GPIO_A);

	return state;
}


bool mcp23017::readSwitch(unsigned switchNumber)
{
	byte state = readAllSwitches();
	return state & (1 << switchNumber) ? true : false;
}

void mcp23017::SetRelay(unsigned relayNumber, bool relayState, bool forceSwitchToReflect)
{
	byte state = readOneRegister(MCP_GPIO_B);

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

		Serial.printf("port %d switchState = %02x\n\r",relayNumber,switchState);

		// thw switch does NOT mirror the request state
		if (switchState != relayState)
		{
			Serial.println("switch does NOT reflect request - asked to alter");

			// get polarity of A
			byte polarity = readOneRegister(MCP_IPOL_A);

			Serial.printf("%02x -> ", polarity);

			// flip the polarity bit for that switch
			polarity ^= (1 << relayNumber);

			Serial.printf("%02x ", polarity);
			
			// enabling this line causes an ISR storm
			//writeOneRegister(MCP_IPOL_A, polarity);


			Serial.println("written");
		}
	}

	writeOneRegister(MCP_GPIO_B, state);
}

int mcp23017::InterruptCauseAndCurrentState(bool justClearInterrupt)
{
	if (justClearInterrupt)
	{
		return readAllSwitches();
	}

	byte cause = readOneRegister(MCP_INTF_A);

	// then get current states
	byte state= readAllSwitches();

	Serial.printf("MCPInt cause %02x state %02x [%04x]\n\r", cause, state, (int)((cause & 0xff) << 8) | (state & 0xff));

	// then send that back
	return (int)((cause & 0xff) << 8) | (state & 0xff);
}