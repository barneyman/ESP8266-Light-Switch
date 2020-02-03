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
        thingName="TempSensor";
    }

    virtual bool GetSensorValue(JsonObject &toHere)
    {
        if(!m_tempC.getDS18Count())
        {
            toHere["tempC_error"] = "No DSs found";
            return false;
        }
            

        m_tempC.requestTemperatures();
        float temp= m_tempC.getTempCByIndex(0);

        if (temp > -10.0 && temp < 100.0)
        {
            toHere["temp_C"] = temp;
        }
        else
        {
            toHere["tempC_error"] = "Exceeded bounds";
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

    virtual bool GetSensorValue(JsonObject &toHere)
    {
        if(!i2cAddressExists)
        {
            toHere["bme_error"] = "No BMEs found";
            return false;
        }

		toHere["temp_C"] = m_sensor.temp();
		toHere["pres_hPA"] = m_sensor.pres();
		toHere["humid_%"] = m_sensor.hum();

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

    virtual bool GetSensorValue(JsonObject &toHere)
    {
        if(!i2cAddressExists)
        {
            toHere["lux_error"] = "No MAX44009s found";
            return false;
        }

		toHere["light_Lux"] = m_sensor.getLux();

        return true;
    }


protected:

	Max44009 m_sensor;

};


//////////////////////////////////////////


class baseSwitch : public baseThing
{

public:

	enum typeOfSwitch { stUndefined, stMomentary, stToggle, stVirtual };
	//enum switchState { swUnknown, swOn, swOff };
	
protected:	

	enum typeOfSwitch m_type;

public:

	baseSwitch(unsigned long bounceThreshold, debugBaseClass*dbg):switchCount(0),last_seen_bounce(0),bounce_threshhold(bounceThreshold),
		baseThing(dbg), m_type(stUndefined)
	{

	}

	// TODO rename DoRelay
	virtual void ICACHE_RAM_ATTR DoSwitch(bool on, bool forceSwitchToReflect=false)
	{
		dblog->isr_printf(debug::dbImportant, "DoSwitch: %s %s\r\n", on ? "ON" : "off", forceSwitchToReflect ? "FORCE" : "");		
	}

	virtual void ICACHE_RAM_ATTR ToggleRelay()
	{
		DoSwitch(!GetSwitch());
	}

	// TODO rename GetRelay
	virtual bool ICACHE_RAM_ATTR GetSwitch()=0;

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
	momentarySwitch(debugBaseClass *dblog):baseSwitch(_DEBOUNCE_WINDOW_MOMENTARY,dblog)
	{
		m_type=stMomentary;
	}
};

class toggleSwitch : public baseSwitch
{
public:
	toggleSwitch(debugBaseClass *dblog):baseSwitch(_DEBOUNCE_WINDOW_TOGGLE,dblog)
	{
		m_type=stToggle;
	}
};


class SonoffBasic : public momentarySwitch
{

public:

	SonoffBasic(debugBaseClass *dblog,int digitalPinInput=0, int digitalPinOutput=12,int digitalPinLED=13):
		m_ioPinIn(digitalPinInput), m_ioPinOut(digitalPinOutput), m_ioPinLED(digitalPinLED),
		ignoreISRrequests(false),
		momentarySwitch(dblog)
	{
		// and remember shit
		m_singleton=this;

		// pull up in
		pinMode(digitalPinInput, INPUT_PULLUP);
		// and attach to it, it's momentary, so get if falling low
		attachInterrupt(digitalPinInput, SonoffBasic::staticOnSwitchISR, FALLING );

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

		//dblog->isr_println(debug::dbInfo,"OnSwitchISR in");			

		if(IsSwitchBounce())
			return;

		// just alternate
		ToggleRelay();

	}

	virtual bool ICACHE_RAM_ATTR GetSwitch()
	{
		return digitalRead(m_ioPinOut)==HIGH;
	}

	virtual void ICACHE_RAM_ATTR DoSwitch(bool on, bool forceSwitchToReflect=false)
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
  static SonoffBasic* m_singleton;

};

class SonoffBasicNoLED : public SonoffBasic
{
public:

	// ctor sig is the same as Sooff Basic, but LED pin is ignored
	SonoffBasicNoLED(debugBaseClass *dblog,int digitalPinInput=0, int digitalPinOutput=12,int digitalPinLED=-1):
		SonoffBasic(dblog,digitalPinInput,digitalPinOutput,-1)
	{

	}

};