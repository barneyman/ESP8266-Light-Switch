#include <Arduino.h>
#include <debugLogger.h>
#include <ArduinoJson.h>

#include "announce.h"

class baseThing : public StateAnnouncer
{

public:

	baseThing(debugBaseClass*dbg):StateAnnouncer(dbg),
		thingName("Default")
	{

	}

	String GetName()
	{
		return thingName;
	}

	virtual void DoWork(){}

protected:

	unsigned long GetMillis()
	{
		return micros() / 1000;
	}

    String thingName;
};

class baseSensor : public baseThing
{
public:

    baseSensor(debugBaseClass*dbg):baseThing(dbg),
        m_valid(false)
    {}

    virtual bool GetSensorValue(JsonObject &toHere)=0;
	virtual void GetSensorConfig(JsonObject &toHere)
	{
		// ask it to get sensor stuff
		JsonArray &sensorElement = toHere.createNestedArray("elements");
		GetSensorConfigElements(sensorElement);
	}

	virtual void GetSensorConfigElements(JsonArray &toHere)=0;

    protected:

    // does this sensor have any useful data?
    bool    m_valid;

};

#ifndef ARDUINO_ESP8266_GENERIC

#include <OneWire.h>

class OneWireSensor : public baseSensor
{

public:

    OneWireSensor(int dataPin,debugBaseClass*dbg):baseSensor(dbg),
         m_oneWire(dataPin)
    {

    }

protected:

    OneWire m_oneWire;

};


#include <Wire.h>

class I2CSensor : public baseSensor
{
protected:

	bool i2cAddressExists;


public:

	I2CSensor(int address, debugBaseClass*dbg):baseSensor(dbg)
	{
		i2cAddressExists=AddressExists(address);
	}

	bool AddressExists(int addr)
	{
		if(m_dblog)
			m_dblog->printf(debug::dbInfo, "I2C checking for 0x%x ", addr);

		TwoWire localWire;
		localWire.begin();
		localWire.beginTransmission(addr);
		bool retval= localWire.endTransmission()?false:true;

		if(m_dblog)
			m_dblog->println(debug::dbInfo, retval?"EXISTS":"nexist");

		return retval;
	}

	void i2cscan()
	{
		TwoWire localWire;
		localWire.begin();

		if(m_dblog)
			m_dblog->println(debug::dbAlways, "I2C Scanning...");
	
		unsigned nDevices = 0;
		for(int address = 1; address < 127; address++ )
		{
			// The i2c_scanner uses the return value of
			// the Write.endTransmisstion to see if
			// a device did acknowledge to the address.
			localWire.beginTransmission(address);
			int error = localWire.endTransmission();
		
			if (error == 0)
			{
				if(m_dblog)
					m_dblog->printf(debug::dbAlways,"I2C device found at address 0x%x\r", address);
			
				nDevices++;
			}
			else if (error==4)
			{
				if(m_dblog)
					m_dblog->printf(debug::dbAlways,"Unknown error at address 0x%x\r", address);
			}    
		}
		if (nDevices == 0)
		{
			if(m_dblog)
				m_dblog->println(debug::dbAlways,"No I2C devices found\n");
		}
	}

};




#include <DallasTemperature.h>

class DallasSingleSensor : public OneWireSensor
{

public: 

    DallasSingleSensor(int datapin,debugBaseClass*dbg):
        OneWireSensor(datapin,dbg),
        m_tempC(&m_oneWire)
    {
        m_tempC.begin();
        thingName="DS18B20";
    }

	virtual void GetSensorConfigElements(JsonArray &toHere)
	{
		JsonObject &sensorElement = toHere.createNestedObject();
		sensorElement["type"] = "temperature";
		sensorElement["uom"] = "°C";
		sensorElement["round"] = "1";
		
	}

    virtual bool GetSensorValue(JsonObject &toHere)
    {
        if(!m_tempC.getDS18Count())
        {
            toHere["temperature_error"] = "No DSs found";
            return false;
        }
            

        m_tempC.requestTemperatures();
        float temp= m_tempC.getTempCByIndex(0);

        if (temp > -10.0 && temp < 100.0)
        {
            toHere["temperature"] = temp;
        }
        else
        {
            toHere["temperature_error"] = "Exceeded bounds";
        }
        

        return true;
    }


protected:

    DallasTemperature m_tempC;

};



#include <BME280I2C.h>

class BME280Sensor : public I2CSensor
{

public:

	BME280Sensor(debugBaseClass*dbg):I2CSensor(0x76, dbg)
	{
		m_sensor.begin();
		thingName="BME280";
	}

	virtual void GetSensorConfigElements(JsonArray &toHere)
	{
		JsonObject &sensorElement1 = toHere.createNestedObject();
		sensorElement1["type"] = "temperature";
		sensorElement1["uom"] = "°C";
		sensorElement1["round"] = "1";

		JsonObject &sensorElement2 = toHere.createNestedObject();
		sensorElement2["type"] = "pressure";
		sensorElement2["uom"] = "hPA";
		sensorElement2["round"] = "1";

		JsonObject &sensorElement3 = toHere.createNestedObject();
		sensorElement3["type"] = "humidity";
		sensorElement3["uom"] = "%";
		sensorElement3["round"] = "0";
	}


    virtual bool GetSensorValue(JsonObject &toHere)
    {
        if(!i2cAddressExists)
        {
            toHere["bme_error"] = "No BMEs found";
            return false;
        }

		// BME is consistently 1deg over
		toHere["temperature"] = m_sensor.temp()-1.0;
		toHere["pressure"] = m_sensor.pres();
		toHere["humidity"] = m_sensor.hum();

        return true;
    }


protected:

	BME280I2C m_sensor;

};

#include <MAX44009.h>

class MAX44009Sensor: public I2CSensor
{
protected:

public:

	MAX44009Sensor(debugBaseClass*dbg,int address=0x4a):I2CSensor(address, dbg)//,m_sensor(address)
	{
		thingName="MAX44009";
		m_sensor.begin();
	}

	virtual void GetSensorConfigElements(JsonArray &toHere)
	{
		JsonObject &sensorElement = toHere.createNestedObject();
		sensorElement["type"] = "illuminance";
		sensorElement["uom"] = "Lux";
		sensorElement["round"] = "0";
	}


    virtual bool GetSensorValue(JsonObject &toHere)
    {
        if(!i2cAddressExists)
        {
            toHere["light_error"] = "No MAX44009s found";
            return false;
        }

		toHere["illuminance"] = m_sensor.get_lux();

        return true;
    }


protected:

	MAX44009 m_sensor;

};

#endif ARDUINO_ESP8266_GENERIC


class instantSensor : public baseSensor
{

protected:


	bool m_currentState;

	String deviceClass;

public:

	instantSensor(debugBaseClass*dbg):baseSensor(dbg),m_currentState(false)
	{
	
	}

    virtual bool GetSensorValue(JsonObject &toHere)
    {
		toHere["state"] = m_currentState;

        return true;
    }

	virtual void GetSensorConfigElements(JsonArray &toHere)
	{
		JsonObject &sensorElement = toHere.createNestedObject();
#ifdef _USE_UDP
		sensorElement["impl"]="udp";
#elif defined(_USE_TCP)
		sensorElement["impl"]="tcp";
#elif defined(_USE_REST)		
		sensorElement["impl"]="rest";
#else
#endif
		sensorElement["type"] = deviceClass;

	}




};


class testInstantSensor : public instantSensor
{

protected:

	unsigned long m_millis, m_timeout;

public:
	testInstantSensor(debugBaseClass*dbg, unsigned long timeout=600000):instantSensor(dbg),m_timeout(timeout)
	{
		m_millis=millis();
		thingName="testSensor";
		deviceClass="test";
	}

	virtual void DoWork()
	{
		// if our state has changed send State
		unsigned long now=millis();
		if((now-m_millis) > m_timeout)
		{
			m_currentState=!m_currentState;
			SendState(m_currentState);
			m_millis=now;
		}

		instantSensor::DoWork();

	}

};


class GPIOInstantSensor : public instantSensor
{
protected:

	unsigned m_gpio, m_gpOut;
	bool m_invertedInput, m_invertedOutput;
	volatile bool m_ioChanged;

	static void ICACHE_RAM_ATTR static_isr()
	{
		m_singleton->m_ioChanged=true;
	}

	static GPIOInstantSensor* m_singleton;

public:

	GPIOInstantSensor(debugBaseClass*dbg,unsigned gpio, unsigned displayPin=-1, bool invertedInput=false, bool invertedOutput=false):instantSensor(dbg),m_gpio(gpio),m_ioChanged(false),m_gpOut(displayPin)
	{
		m_singleton=this;
		pinMode(m_gpio, INPUT);
		attachInterrupt(m_gpio, static_isr, CHANGE);

		if(m_gpOut!=-1)
			pinMode(m_gpOut,OUTPUT);

		m_invertedInput=invertedInput;
		m_invertedOutput=invertedOutput;

	}

	virtual void DoWork()
	{
		// if our state has changed send State
		if(m_ioChanged)
		{

			m_currentState=(digitalRead(m_gpio)==(m_invertedInput?LOW:HIGH))?true:false;

			if(m_dblog)
				m_dblog->printf(debug::dbInfo,"sensor changed %s\r",m_currentState?"HIGH":"LOW");

			if(m_gpOut!=-1)
			{
				bool ledState=m_currentState;
				if(m_invertedOutput)
					ledState=!ledState;

				digitalWrite(m_gpOut,ledState);
			}	

			SendState(m_currentState);
			m_ioChanged=false;
		}

		instantSensor::DoWork();

	}


};

class PIRInstantSensor : public GPIOInstantSensor
{
public:
	// wemos is inverted LED
	PIRInstantSensor(debugBaseClass*dbg,unsigned gpio, unsigned displayPin=LED_BUILTIN, bool invertedLED=true):
		GPIOInstantSensor(dbg, gpio, displayPin,false,invertedLED)
	{
		thingName="PIR";
		deviceClass="motion";
	}

};


//////////////////////////////////////////

// TODO - have to distinguish between a 'SWITCH' and a 'RELAY' and a 'SWITCH mated to RELAY'
class baseSwitch : public baseThing
{

public:

	enum typeOfSwitch { stUndefined, stMomentary, stToggle, stVirtual };
	//enum switchState { swUnknown, swOn, swOff };
	
protected:	

	enum typeOfSwitch m_type;
	enum workToDo_Switch { w2dNone, w2dToggle, w2dQuery };
	volatile enum workToDo_Switch m_workToDo;

public:

	baseSwitch(debugBaseClass*dbg, unsigned long bounceThreshold):switchCount(0),last_seen_bounce(0),bounce_threshhold(bounceThreshold),
		baseThing(dbg), m_type(stUndefined),
		m_workToDo(w2dNone)
	{

	}

	virtual void DoRelay(bool on, bool forceSwitchToReflect=false)
	{
		if(forceSwitchToReflect)
		{
			SetSwitch(on);
		}	
		else
		{
			SetRelay(on);
		}
		
	}

	// switch and relay do not relate to exactly the same thing
	// switch is the user side, relay is the power side

	// in a simply situation switch==relay
	virtual bool GetSwitch()
	{
		return GetRelay();
	}

	virtual void SetSwitch(bool on)
	{
		SetRelay(on);
	}


	virtual void ToggleRelay()
	{
		DoRelay(!GetRelay());
	}

	virtual bool GetRelay()=0;
	virtual void SetRelay(bool){}

	// make the relay honour the switch
	virtual void HonourCurrentSwitch()=0;


	// to handle multiple switches 
	virtual unsigned ChildSwitchCount()
	{
		return 0;
	}

	virtual bool ICACHE_RAM_ATTR IsSwitchBounce()
	{
		unsigned long now=GetMillis();
		unsigned long bounceTime=now-last_seen_bounce;

		last_seen_bounce=now;

		if(bounceTime<bounce_threshhold)
			return true;

		return false;
	}

	void ResetTransitionCount()
	{
		switchCount=0;	
	}

	unsigned long GetTransitionCount()
	{
		return switchCount;
	}

	enum typeOfSwitch GetSwitchType()
	{
		return m_type;
	}


protected:

	unsigned long switchCount;	// TODO rename to switchTransitionCount
	unsigned long last_seen_bounce, bounce_threshhold;
	

};

#define _DEBOUNCE_WINDOW_MOMENTARY	100
#define _DEBOUNCE_WINDOW_TOGGLE		100

class momentarySwitch : public baseSwitch
{
public:
	momentarySwitch(debugBaseClass *m_dblog):baseSwitch(m_dblog,_DEBOUNCE_WINDOW_MOMENTARY)
	{
		m_type=stMomentary;
	}

	// do nothing
	virtual void HonourCurrentSwitch()
	{
	}



};

class toggleSwitch : public baseSwitch
{

protected:

	// a switch is bistable, if the SWITCH/RELAY is off and software sends a relayon, switch should be flipped
	// or the next time is physically pressed it'll turn it on, ie do nothing
	bool m_flipSwitchPolarity, m_currentSwitchState;

public:
	toggleSwitch(debugBaseClass *m_dblog):baseSwitch(m_dblog,_DEBOUNCE_WINDOW_TOGGLE),m_flipSwitchPolarity(false),m_currentSwitchState(false)
	{
		m_type=stToggle;
	}

	// get's the compound answer, what the physical thing is doing, and what we've done to it 
	virtual bool GetSwitch()
	{
		return m_flipSwitchPolarity?!m_currentSwitchState :m_currentSwitchState;
	}

	// resets, hard, the relationship between switch and relay
	virtual void SetSwitch(bool on)
	{
		baseSwitch::SetRelay(on);
		m_flipSwitchPolarity=false;
		m_currentSwitchState=on;
	}

	// changes the relay, and remembers you 'swapped' the switch
	virtual void SetRelay(bool on)
	{
		m_flipSwitchPolarity=!m_flipSwitchPolarity;
		baseSwitch::SetRelay(on);
	}

	virtual void HonourCurrentSwitch()
	{
		SetSwitch(GetSwitch());
	}


};




class RelayLEDandSwitch : public momentarySwitch
{

protected:


public:

	RelayLEDandSwitch(debugBaseClass *m_dblog,int digitalPinInput=0, int digitalPinOutput=12,int digitalPinLED=13, bool switchOnGND=true):
		m_ioPinIn(digitalPinInput), m_ioPinOut(digitalPinOutput), m_ioPinLED(digitalPinLED),
		ignoreISRrequests(false),
		momentarySwitch(m_dblog)
	{
		// and remember shit
		m_singleton=this;

		if(switchOnGND)
		{
			// pull up in
			pinMode(digitalPinInput, INPUT_PULLUP);
			// and attach to it, it's momentary, so get if falling low
			attachInterrupt(digitalPinInput, RelayLEDandSwitch::staticOnSwitchISR, ONLOW );
		}
		else
		{
			// and attach to it, it's momentary, so get if falling low
			attachInterrupt(digitalPinInput, RelayLEDandSwitch::staticOnSwitchISR, ONHIGH );
		}

		// relay is output
		pinMode(digitalPinOutput, OUTPUT);

		// LED is output
		if(m_ioPinLED!=-1)
			pinMode(digitalPinLED, OUTPUT);

	}

	static void ICACHE_RAM_ATTR staticOnSwitchISR()
	{
		m_singleton->OnSwitchISR();

	}

	void ICACHE_RAM_ATTR OnSwitchISR()
	{
		if(ignoreISRrequests)
			return;

		if(m_dblog)
			m_dblog->isr_println(debug::dbInfo,"RelayLEDandSwitch::OnSwitchISR in");			

		if(IsSwitchBounce())
			return;

		// just alternate
		m_workToDo=w2dToggle;

	}

	virtual void DoWork()
	{
		switch(m_workToDo)
		{
			case w2dToggle:
				ToggleRelay();
				break;
			default:
				break;	
		}

		m_workToDo=w2dNone;

		momentarySwitch::DoWork();

	}


	virtual bool GetRelay()
	{
		return digitalRead(m_ioPinOut)==HIGH;
	}


	virtual void SetLEDstate(bool on)
	{
		if(m_ioPinLED!=-1)
			digitalWrite(m_ioPinLED, on ? HIGH : LOW);
	}

	virtual void SetRelay(bool on)
	{
		// have seen instances where firing this relay contributes enough
		// noise to get detected as an input switch - so guard
		ignoreISRrequests=true;

		// closely tied
		digitalWrite(m_ioPinOut, on ? HIGH : LOW);

		if(m_ioPinLED!=-1)
			SetLEDstate(on);

		// and inc
		switchCount++;

		ignoreISRrequests=false;

		// and announce
		SendState(on);
	}

	virtual void HonourCurrentSwitch()
	{
		SetLEDstate(GetRelay());
	}

protected:

	unsigned m_ioPinIn, m_ioPinOut, m_ioPinLED;
	volatile bool ignoreISRrequests;

public:
  static RelayLEDandSwitch* m_singleton;

};


class SonoffBasic : public RelayLEDandSwitch
{
public:
	SonoffBasic(debugBaseClass *m_dblog,int digitalPinInput=0, int digitalPinOutput=12,int digitalPinLED=13):
		RelayLEDandSwitch(m_dblog,digitalPinInput,digitalPinOutput,digitalPinLED, true)
		{
			
		}


	virtual void SetLEDstate(bool on)
	{
		// LED is inverted on the sonoff
		if(m_ioPinLED!=-1)
			digitalWrite(m_ioPinLED, on ? LOW : HIGH);
	}


};

class SonoffBasicNoLED : public RelayLEDandSwitch
{
	public:
	SonoffBasicNoLED(debugBaseClass *m_dblog,int digitalPinInput=0, int digitalPinOutput=12):
		RelayLEDandSwitch(m_dblog,digitalPinInput,digitalPinOutput,-1)
		{

		}
};


#ifndef ARDUINO_ESP8266_GENERIC

// TODO - untested
class WemosRelayShield : public RelayLEDandSwitch
{
public:
	WemosRelayShield(debugBaseClass *m_dblog):RelayLEDandSwitch(m_dblog,D2,D1,LED_BUILTIN)
	{}
};

#endif ARDUINO_ESP8266_GENERIC

#define _DEBOUNCE_WINDOW_LOGIC_TTL	10

// acts as a 'gang' switch, but exposes the children
class MultiSwitch : public baseSwitch
{

protected:

	class childSwitch : public toggleSwitch
	{
	public:
		childSwitch(debugBaseClass *m_dblog, MultiSwitch *parent, unsigned ordinal):
			toggleSwitch(m_dblog),m_parent(parent),m_ordinal(ordinal)
		{
			thingName="ordinal "+String(ordinal);
		}

		virtual void SetRelay(bool on)
		{
			// ask dad
			if(m_parent)
				m_parent->SetChildRelay(m_ordinal, on);
		}

		virtual bool GetRelay()
		{
			// ask dad
			if(m_parent)
				return m_parent->GetChildRelay(m_ordinal);
			return false;
		}

	protected:
		MultiSwitch*m_parent;
		unsigned m_ordinal;
	};


public:
	MultiSwitch(debugBaseClass *m_dblog):
		baseSwitch(m_dblog,_DEBOUNCE_WINDOW_LOGIC_TTL)
	{
		m_type=stVirtual;
		thingName="Gang";
	}

	virtual unsigned ChildSwitchCount()
	{
		return m_children.size();
	}

	baseSwitch *GetChild(unsigned offset)
	{
		if(offset<m_children.size())
		{
			return m_children[offset];
		}

		return NULL;
	}

	virtual bool GetRelay()
	{
		// if all the kids are on, i'm on
		for(auto each=0;each<m_children.size();each++)
		{
			if(!GetChildRelay(each))
				return false;
		}
		return true;
	}

	// turn all the children on or off
	virtual void DoRelay(bool on, bool forceSwitchToReflect=false)
	{
		if(m_dblog)
			m_dblog->println(debug::dbVerbose,"DoRelay on Gang called");
		for(auto each=0;each<m_children.size();each++)
		{
			if(m_dblog)
				m_dblog->printf(debug::dbVerbose,"DoChildRelay %d called\n\r",each);
			DoChildRelay(each,on,forceSwitchToReflect);
		}
	}

	virtual void DoChildRelay(unsigned child,bool on, bool forceSwitchToReflect=false)=0;
	virtual void SetChildRelay(unsigned child, bool on)=0;
	virtual bool GetChildRelay(unsigned child)=0;

protected:

	std::vector<baseSwitch*> m_children;

	unsigned m_numSwitches;

};

#ifndef ARDUINO_ESP8266_GENERIC

#include "mcp23017.h"

class MCP23017MultiSwitch : public MultiSwitch
{


protected:

	// TODO - one of the bases should inherit from StateAnnouncer
	class MCP23071ChildSwitch : public MultiSwitch::childSwitch, public StateAnnouncer
	{
	public:
		MCP23071ChildSwitch(debugBaseClass *m_dblog, MultiSwitch *parent, unsigned ordinal):
			MultiSwitch::childSwitch(m_dblog,parent,ordinal),
			StateAnnouncer(m_dblog)
		{

		}





	};


public:

	MCP23017MultiSwitch(debugBaseClass *m_dblog, unsigned numSwitches, int sdaPin, int sclPin, int intPin):
		MultiSwitch(m_dblog),m_isrHits(0),
#ifdef USE_MCP_RELAY		
		m_iochip(m_dblog, sdaPin,sclPin, D0, D3)
#else
		m_iochip(m_dblog, sdaPin, sclPin)
#endif		
	{
		pinMode(intPin, INPUT_PULLUP);
		m_singleton=this;
		// it's pulled up, so get it low
		attachInterrupt(intPin,staticOnSwitchISR, FALLING);


		m_iochip.Initialise();

		for(unsigned each=0;each<numSwitches;each++)
		{
			m_children.push_back(new MCP23071ChildSwitch(m_dblog, this, each));
		}
	}

	virtual void DoChildRelay(unsigned child,bool on, bool forceSwitchToReflect=false)
	{
		SetChildRelay(child,on);
	}

	virtual void SetChildRelay(unsigned child,bool on)
	{
		m_iochip.SetRelay(child,on);
		// TODO - this should be in baseThing
		m_children[child]->SendState(on);
	}


	virtual bool GetChildRelay(unsigned child)
	{
		bool result=false;
		m_iochip.GetRelay(child,result);
		return result;
	}


	static void ICACHE_RAM_ATTR staticOnSwitchISR()
	{
		m_singleton->OnSwitchISR();
	}

	void ICACHE_RAM_ATTR OnSwitchISR()
	{
		if(m_dblog)
			m_dblog->isr_println(debug::dbInfo,"MCP23017MultiSwitch::OnSwitchISR in");			

		m_isrHits++;

		if(m_workToDo==w2dQuery)
		{
			// this is an error condition - we've rentered
			if(m_dblog)
				m_dblog->isr_println(debug::dbError,"MCP23017MultiSwitch::OnSwitchISR re-entered");			
			return;
		}

		// just query
		m_workToDo=w2dQuery;

	}

	virtual void DoWork()
	{
		switch(m_workToDo)
		{
			case w2dQuery:
				// clear my state early, otherwise reading the state,
				// clears the interrupt, allowing the ISR to fire,
				// which sets a state, i then set to None too late
				m_workToDo=w2dNone;
				QueryStateAndAct();
			default:
				break;	

		}

		// if(m_isrHits)
		// {
		// 	//m_dblog->printf(debug::dbInfo,"isr hits = %lu\r",m_isrHits);
		// }	
		

		

		MultiSwitch::DoWork();

	}

	void QueryStateAndAct()
	{
		unsigned causeAndState=m_iochip.QueryInterruptCauseAndCurrentState(false);

		if(!(causeAndState&0xff00))
		{
			if(m_dblog)
				m_dblog->println(debug::dbWarning,"QueryStateAndAct called with no cause");
			return;
		}

		// just in case more than one fired
		for(int each=0;each<this->ChildSwitchCount();each++)
		{
			if(causeAndState & (1<<(each+8)))
			{
				if(m_dblog)
					m_dblog->printf(debug::dbInfo,"Port %u triggered ... ", each);

				if(m_children[each]->IsSwitchBounce())
				{
					if(m_dblog)
						m_dblog->println(debug::dbInfo,"bounce");
					continue;
				}

				// get the state
				bool state=(causeAndState&(1<<each))?true:false;
				if(m_dblog)
					m_dblog->printf(debug::dbInfo,"%s\r\n", state?"ON":"off");
				SetChildRelay(each,state);
			}
		}

	}

	virtual void HonourCurrentSwitch()
	{
		// get the switch, set the relay, save the girl
		byte allSwitchStates=m_iochip.readAllSwitches(false);
		if(m_dblog)
			m_dblog->printf(debug::dbInfo,"readAllSwitches %x\n\r",allSwitchStates);
		// switches are the 'correct' way round, relays are inverted 
		byte allRelayStates=~allSwitchStates;
		// then set all relays
		m_iochip.setAllRelays(allRelayStates);
	}

protected:

//#define USE_MCP_RELAY
#ifdef USE_MCP_RELAY
	mcp23017AndRelay m_iochip;
#else	
	mcp23017 m_iochip;
#endif	

	static MCP23017MultiSwitch *m_singleton;

	volatile unsigned long m_isrHits;


};	

#endif ARDUINO_ESP8266_GENERIC