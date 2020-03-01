#include <Arduino.h>
#include <debugLogger.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BME280I2C.h>
#include <MAX44009.h>


class baseThing
{

public:

	baseThing(debugBaseClass*dbg):dblog(dbg),
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

	debugBaseClass *dblog;
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

	virtual void AddSensorRecipient(IPAddress addr, unsigned port) 
	{
		dblog->println(debug::dbWarning,"base AddSensorRecipient called");
	}

	virtual void GetSensorConfigElements(JsonArray &toHere)=0;

    protected:

    // does this sensor have any useful data?
    bool    m_valid;

};

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
		dblog->printf(debug::dbInfo, "I2C checking for 0x%x ", addr);

		TwoWire localWire;
		localWire.begin();
		localWire.beginTransmission(addr);
		bool retval= localWire.endTransmission()?false:true;

		dblog->println(debug::dbInfo, retval?"EXISTS":"nexist");

		return retval;
	}

	void i2cscan()
	{
		TwoWire localWire;
		localWire.begin();

		dblog->println(debug::dbAlways, "I2C Scanning...");
	
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
				dblog->printf(debug::dbAlways,"I2C device found at address 0x%x\r", address);
			
				nDevices++;
			}
			else if (error==4)
			{
				dblog->printf(debug::dbAlways,"Unknown error at address 0x%x\r", address);
			}    
		}
		if (nDevices == 0)
			dblog->println(debug::dbAlways,"No I2C devices found\n");
	}

};





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
		sensorElement["impl"]="rest";
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
		sensorElement1["impl"]="rest";
		sensorElement1["type"] = "temperature";
		sensorElement1["uom"] = "°C";
		sensorElement1["round"] = "1";

		JsonObject &sensorElement2 = toHere.createNestedObject();
		sensorElement2["impl"]="rest";
		sensorElement2["type"] = "pressure";
		sensorElement2["uom"] = "hPA";
		sensorElement2["round"] = "1";

		JsonObject &sensorElement3 = toHere.createNestedObject();
		sensorElement3["impl"]="rest";
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

class MAX44009Sensor: public I2CSensor
{
protected:

public:

	MAX44009Sensor(debugBaseClass*dbg,int address=0x4a):I2CSensor(address, dbg),m_sensor(address)
	{
		thingName="MAX44009";
	}

	virtual void GetSensorConfigElements(JsonArray &toHere)
	{
		JsonObject &sensorElement = toHere.createNestedObject();
		sensorElement["impl"]="rest";
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

		toHere["illuminance"] = m_sensor.getLux();

        return true;
    }


protected:

	Max44009 m_sensor;

};

#define _USE_UDP

#ifdef _USE_UDP
#else
#include <WiFiClient.h>
#endif



class instantSensor : public baseSensor
{

protected:

	class recipient
	{
		public:
		recipient(IPAddress addr, unsigned port):m_addr(addr),m_port(port)
		{

		}

		// only checks host, deliberately
		// this allows the HA server to reboot, change ports, and we'll spot it
		bool operator==(const recipient &other)
		{
			return m_addr==other.m_addr;// && m_port==other.m_port;
		}


		IPAddress m_addr;
		unsigned m_port;
	};

	std::vector<recipient> m_HAhosts;

	unsigned m_port;

	StaticJsonBuffer<200> jsonBuffer;

	bool m_currentState;

	String deviceClass;

public:

	instantSensor(debugBaseClass*dbg):baseSensor(dbg),m_currentState(false)
	{
		// 
		// debug
		
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
#else
		sensorElement["impl"]="tcp";
#endif
		sensorElement["type"] = deviceClass;

	}

	virtual void AddSensorRecipient(IPAddress addr, unsigned port) 
	{
		dblog->printf(debug::dbInfo,"Adding recipient %s\r",addr.toString().c_str());
		recipient potential(addr,port);
		auto finder=std::find(m_HAhosts.begin(),m_HAhosts.end(),potential);
		if(finder==m_HAhosts.end())
		{
			m_HAhosts.push_back(potential);
			dblog->println(debug::dbInfo,"ADDED");
		}
		else
		{
			// cater for port change
			if(finder->m_port==port)
			{
				dblog->println(debug::dbInfo,"already exists");

			}
			else
			{
				dblog->println(debug::dbInfo,"port has changed, re-mapping");
				finder->m_port=port;
				//m_HAhosts.erase(finder);
				//m_HAhosts.push_back(potential);
			}

		}
	}


protected:


	void SendState(bool state)
	{
		// 
		jsonBuffer.clear();
		JsonObject& udproot = jsonBuffer.createObject();
		udproot["state"]=m_currentState;
		String bodyText;
		udproot.printTo(bodyText);

#ifdef _USE_UDP
		WiFiUDP sender;
#else
		WiFiClient sender;
#endif		

		dblog->println(debug::dbInfo,"Starting IP yell");

		for(auto eachHA=m_HAhosts.begin();eachHA!=m_HAhosts.end();eachHA++)
		{

#ifdef _USE_UDP

			sender.beginPacket(eachHA->m_addr, eachHA->m_port);

			dblog->printf(debug::dbInfo,"udp %s to %s:%u\r",bodyText.c_str(), eachHA->m_addr.toString().c_str(),eachHA->m_port);
			sender.write(bodyText.c_str(),bodyText.length());

			sender.endPacket();

#else

			if(sender.connect(eachHA->m_addr, eachHA->m_port)==1)
			{
				dblog->printf(debug::dbInfo,"tcp %s;%u to %s\r",bodyText.c_str(),eachHA->m_port, eachHA->m_addr.toString().c_str());
				sender.write(bodyText.c_str(),bodyText.length());
				sender.flush();
				sender.stop();
			}
			else
			{
				dblog->printf(debug::dbError,"tcpconnect failed %s:%u\r", eachHA->m_addr.toString().c_str(),eachHA->m_port);
			}

#endif

		}		

	}



};


class testInstantSensor : public instantSensor
{

protected:

	unsigned long m_millis, m_timeout;

public:
	testInstantSensor(debugBaseClass*dbg, unsigned long timeout):instantSensor(dbg),m_timeout(timeout)
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

	unsigned m_gpio;
	volatile bool m_ioChanged;

	static void ICACHE_RAM_ATTR static_isr()
	{
		m_singleton->m_ioChanged=true;
	}

	static GPIOInstantSensor* m_singleton;

public:

	GPIOInstantSensor(debugBaseClass*dbg,unsigned gpio):instantSensor(dbg),m_gpio(gpio),m_ioChanged(false)
	{
		m_singleton=this;
		pinMode(m_gpio, INPUT);
		attachInterrupt(m_gpio, static_isr, CHANGE);
	}

	virtual void DoWork()
	{
		// if our state has changed send State
		if(m_ioChanged)
		{
			m_currentState=(digitalRead(m_gpio)==HIGH)?true:false;

			dblog->printf(debug::dbInfo,"sensor changed %s\r",m_currentState?"HIGH":"LOW");

			SendState(m_currentState);
			m_ioChanged=false;
		}

		instantSensor::DoWork();

	}


};

class PIRInstantSensor : public GPIOInstantSensor
{
public:
	PIRInstantSensor(debugBaseClass*dbg,unsigned gpio):GPIOInstantSensor(dbg, gpio)
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
		//dblog->isr_printf(debug::dbImportant, "DoRelay: %s %s\r\n", on ? "ON" : "off", forceSwitchToReflect ? "FORCE" : "");		
	}

	virtual void ToggleRelay()
	{
		DoRelay(!GetRelay());
	}

	virtual bool GetRelay()=0;

	// make the relay honour the switch
	virtual void HonourCurrentSwitch()
	{

	}

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

#define _DEBOUNCE_WINDOW_MOMENTARY	300
#define _DEBOUNCE_WINDOW_TOGGLE		100

class momentarySwitch : public baseSwitch
{
public:
	momentarySwitch(debugBaseClass *dblog):baseSwitch(dblog,_DEBOUNCE_WINDOW_MOMENTARY)
	{
		m_type=stMomentary;
	}
};

class toggleSwitch : public baseSwitch
{
public:
	toggleSwitch(debugBaseClass *dblog):baseSwitch(dblog,_DEBOUNCE_WINDOW_TOGGLE)
	{
		m_type=stToggle;
	}
};


class RelayLEDandSwitch : public momentarySwitch
{

protected:


public:

	RelayLEDandSwitch(debugBaseClass *dblog,int digitalPinInput=0, int digitalPinOutput=12,int digitalPinLED=13):
		m_ioPinIn(digitalPinInput), m_ioPinOut(digitalPinOutput), m_ioPinLED(digitalPinLED),
		ignoreISRrequests(false),
		momentarySwitch(dblog)
	{
		// and remember shit
		m_singleton=this;

		// pull up in
		pinMode(digitalPinInput, INPUT_PULLUP);
		// and attach to it, it's momentary, so get if falling low
		attachInterrupt(digitalPinInput, RelayLEDandSwitch::staticOnSwitchISR, FALLING );

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

		dblog->isr_println(debug::dbInfo,"RelayLEDandSwitch::OnSwitchISR in");			

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

	virtual void DoRelay(bool on, bool forceSwitchToReflect=false)
	{
		// have seen instances where firing this relay contributes enough
		// noise to get detected as an inpout switch - so guard
		ignoreISRrequests=true;

		// closely tied
		digitalWrite(m_ioPinOut, on ? HIGH : LOW);

		// LED is inverted on the sonoff
		if(m_ioPinLED!=-1)
			digitalWrite(m_ioPinLED, on ? LOW : HIGH);

		// and inc
		switchCount++;

		ignoreISRrequests=false;
	}



protected:

	unsigned m_ioPinIn, m_ioPinOut, m_ioPinLED;
	volatile bool ignoreISRrequests;

public:
  static RelayLEDandSwitch* m_singleton;

};

class SonoffBasicNoLED : public RelayLEDandSwitch
{
public:

	// ctor sig is the same as Sooff Basic, but LED pin is ignored
	SonoffBasicNoLED(debugBaseClass *dblog,int digitalPinInput=0, int digitalPinOutput=12,int digitalPinLED=-1):
		RelayLEDandSwitch(dblog,digitalPinInput,digitalPinOutput,-1)
	{

	}

};

// TODO - untested
class WemosRelayShield : public RelayLEDandSwitch
{
public:
	WemosRelayShield(debugBaseClass *dblog):RelayLEDandSwitch(dblog,D2,D1,LED_BUILTIN)
	{}
};


#define _DEBOUNCE_WINDOW_LOGIC_TTL	10

// acts as a 'gang' switch, but exposes the children
class MultiSwitch : public baseSwitch
{

protected:

	class childSwitch : public toggleSwitch
	{
	public:
		childSwitch(debugBaseClass *dblog, MultiSwitch *parent, unsigned port):
			toggleSwitch(dblog),m_parent(parent),m_port(port)
		{
			thingName="Port "+String(port);
		}

		virtual void DoRelay(bool on, bool forceSwitchToReflect=false)
		{
			// ask dad
			if(m_parent)
				m_parent->DoChildRelay(m_port, on, forceSwitchToReflect);
		}

		virtual bool GetRelay()
		{
			// ask dad
			if(m_parent)
				return m_parent->GetChildRelay(m_port);
			return false;
		}

	protected:
		MultiSwitch*m_parent;
		unsigned m_port;
	};


public:
	MultiSwitch(debugBaseClass *dblog):
		baseSwitch(dblog,_DEBOUNCE_WINDOW_LOGIC_TTL)
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
		return false;
	}

	// turn all the children on or off
	virtual void DoRelay(bool on, bool forceSwitchToReflect=false)
	{
		dblog->println(debug::dbVerbose,"DoRelay on Gang called");
		for(auto each=0;each<m_children.size();each++)
		{
			dblog->printf(debug::dbVerbose,"DoChildRelay %d called\n\r",each);
			DoChildRelay(each,on,forceSwitchToReflect);
		}
	}

	virtual void DoChildRelay(unsigned child,bool on, bool forceSwitchToReflect=false)=0;
	virtual bool GetChildRelay(unsigned child)=0;

protected:

	std::vector<baseSwitch*> m_children;

	unsigned m_numSwitches;

};

#include "mcp23017.h"

class MCP23017MultiSwitch : public MultiSwitch
{


public:

	MCP23017MultiSwitch(debugBaseClass *dblog, unsigned numSwitches, int sdaPin, int sclPin, int intPin):
		MultiSwitch(dblog),
#ifdef USE_MCP_RELAY		
		m_iochip(dblog, sdaPin,sclPin, D0, D3)
#else
		m_iochip(dblog, sdaPin, sclPin)
#endif		
	{
		pinMode(intPin, INPUT_PULLUP);
		m_singleton=this;
		attachInterrupt(intPin,staticOnSwitchISR, FALLING);


		m_iochip.Initialise();

		for(unsigned each=0;each<numSwitches;each++)
		{
			m_children.push_back(new childSwitch(dblog, this, each));
		}
	}

	virtual void DoChildRelay(unsigned child,bool on, bool forceSwitchToReflect=false)
	{
		m_iochip.SetRelay(child,on);
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
		dblog->isr_println(debug::dbInfo,"MCP23017MultiSwitch::OnSwitchISR in");			

		// just query
		// m_workToDo=w2dQuery;
		QueryStateAndAct();

	}

	virtual void DoWork()
	{
		switch(m_workToDo)
		{
			case w2dQuery:
				QueryStateAndAct();
				break;
			default:
				break;	

		}

		m_workToDo=w2dNone;

		// clear the port int flag (if set and missed)
		m_iochip.readAllSwitches();

		MultiSwitch::DoWork();

	}

	void QueryStateAndAct()
	{
		unsigned causeAndState=m_iochip.QueryInterruptCauseAndCurrentState(false);

		// just in case more than one fired
		for(int each=0;each<this->ChildSwitchCount();each++)
		{
			if(causeAndState & (1<<(each+8)))
			{
				dblog->isr_printf(debug::dbInfo,"Port %u triggered ... ", each);			

				if(m_children[each]->IsSwitchBounce())
				{
					dblog->isr_println(debug::dbInfo,"bounce");
					continue;
				}

				// get the state
				bool state=(causeAndState&(1<<each))?true:false;
				dblog->isr_printf(debug::dbInfo,"%s\r\n", state?"ON":"off");
				DoChildRelay(each,state,false);
			}
		}

	}

	virtual void HonourCurrentSwitch()
	{
		// get the switch, set the relay, save the girl
		byte allSwitchStates=m_iochip.readAllSwitches(false);
		dblog->printf(debug::dbInfo,"readAllSwitches %x\n\r",allSwitchStates);
		// then set all relays
		m_iochip.setAllRelays(allSwitchStates);
	}

protected:

//#define USE_MCP_RELAY
#ifdef USE_MCP_RELAY
	mcp23017AndRelay m_iochip;
#else	
	mcp23017 m_iochip;
#endif	

	static MCP23017MultiSwitch *m_singleton;

};