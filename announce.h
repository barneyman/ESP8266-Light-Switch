#ifndef _announce_h
#define _announce_h

#include <Arduino.h>

//#define _USE_UDP
//#define _USE_TCP
#define _USE_REST

#ifdef _USE_UDP
#elif defined(_USE_TCP)
#include <WiFiClient.h>
#else
#endif

#ifdef _USE_REST
#ifdef ARDUINO_ARCH_ESP32
#include <WiFiClient.h>
#include <HTTPClient.h>
#else // ARDUINO_ARCH_ESP8266??
#include <ESP8266HTTPClient.h>
#endif
#endif


class StateAnnouncer
{

protected:

	class recipient
	{
		public:
		recipient(IPAddress addr, unsigned port, String &extra):m_addr(addr),m_port(port)
		{
			DynamicJsonBuffer jsonRestInfoBuffer;
			JsonObject& restInfo = jsonRestInfoBuffer.parseObject(extra);

			m_endpoint=restInfo["endpoint"].as<String>();
			m_auth=restInfo["auth"].as<String>();
			if(restInfo.containsKey("instanceid"))
				m_instanceid=restInfo["instanceid"].as<String>();
			else
				m_instanceid=m_addr.toString();
			
		}

		// only checks host, deliberately
		// this allows the HA server to reboot, change ports, and we'll spot it
		bool operator==(const recipient &other)
		{
			return m_instanceid==other.m_instanceid;// && m_port==other.m_port;
		}

		void updateFrom(const recipient &other)
		{
			m_addr=other.m_addr;
			m_endpoint=other.m_endpoint;
			m_auth=other.m_auth;
		}	

		IPAddress m_addr;
		unsigned m_port;
		String m_endpoint, m_auth, m_instanceid;
	};

	std::vector<recipient> m_HAhosts;
	StaticJsonBuffer<200> jsonBuffer;
	String m_extra;

	//unsigned m_port;

	debugBaseClass*m_dblog;

public:

	StateAnnouncer(debugBaseClass*dbg):m_dblog(dbg)
	{}


	// TODO - handle string extra implications
	virtual void AddAnnounceRecipient(IPAddress addr, unsigned port, String extra=String()) 
	{
		recipient potential(addr,port,extra);
		if(m_dblog)
			m_dblog->printf(debug::dbVerbose,"Adding recipient %s\r",potential.m_instanceid.c_str());
		auto finder=std::find(m_HAhosts.begin(),m_HAhosts.end(),potential);
		if(finder==m_HAhosts.end())
		{
			m_HAhosts.push_back(potential);
			if(m_dblog)
				m_dblog->println(debug::dbInfo,"ADDED");
		}
		else
		{
			// cater for auth, port and endpoint change
			finder->updateFrom(potential);

		}
	}

	virtual String GetImpl()
	{
#ifdef _USE_UDP		
		return String("udp");
#elif defined(_USE_REST)		
		return String("rest");
#else
		return String("tcp");
#endif		
	}

public:


	void SendState(bool state)
	{
		// 
		jsonBuffer.clear();
		JsonObject& udproot = jsonBuffer.createObject();
		// on off for binarysensor
		udproot["state"]=state?"on":"off";
		String bodyText;
		udproot.printTo(bodyText);

#ifdef _USE_UDP
		WiFiUDP sender;
#elif defined(_USE_TCP)
		WiFiClient sender;
#elif defined(_USE_REST)

		HTTPClient thisClient;
		WiFiClient wfc;

#else		
#endif		
		if(m_dblog)
			m_dblog->printf(debug::dbInfo,"Starting IP yell ");

		if(m_dblog)
			m_dblog->printf(debug::dbInfo,"%d to yell to\r",m_HAhosts.size());

		for(auto eachHA=m_HAhosts.begin();eachHA!=m_HAhosts.end();eachHA++)
		{

#ifdef _USE_UDP

			sender.beginPacket(eachHA->m_addr, eachHA->m_port);

			if(m_dblog)
				m_dblog->printf(debug::dbInfo,"udp %s to %s:%u\r",bodyText.c_str(), eachHA->m_addr.toString().c_str(),eachHA->m_port);
			sender.write(bodyText.c_str(),bodyText.length());

			sender.endPacket();

#elif defined (_USE_TCP)

			if(sender.connect(eachHA->m_addr, eachHA->m_port)==1)
			{
				if(m_dblog)
					m_dblog->printf(debug::dbInfo,"tcp %s;%u to %s\r",bodyText.c_str(),eachHA->m_port, eachHA->m_addr.toString().c_str());
				sender.write(bodyText.c_str(),bodyText.length());
				sender.flush();
				sender.stop();
			}
			else
			{
				if(m_dblog)
					m_dblog->printf(debug::dbError,"tcpconnect failed %s:%u\r", eachHA->m_addr.toString().c_str(),eachHA->m_port);
			}
#elif defined(_USE_REST)

			if(m_dblog)
				m_dblog->printf(debug::dbInfo,"Posting %s:%u%s\r", eachHA->m_addr.toString().c_str(), eachHA->m_port, eachHA->m_endpoint.c_str());

#ifdef ARDUINO_ARCH_ESP32
			if (thisClient.begin(eachHA->m_addr.toString(), eachHA->m_port, eachHA->m_endpoint.c_str() )) 
#else
			if (thisClient.begin(wfc, eachHA->m_addr.toString(), eachHA->m_port, eachHA->m_endpoint.c_str() )) 
#endif
			{  


				//thisClient.setAuthorization(restInfo["auth"]);
				thisClient.addHeader("content-type", "application/json");

				String bearerToken("Bearer ");
				bearerToken+=eachHA->m_auth;
				thisClient.addHeader("Authorization", bearerToken);
				if(m_dblog)
					m_dblog->printf(debug::dbInfo,"Authorization %s\r", bearerToken.c_str());
				// possible fix for exception 9 on httpClient.end()
				thisClient.setReuse(false);

				// tell it we want the Date header back
				const char *retHeaders[]={"Date", NULL};
				thisClient.collectHeaders(retHeaders,1);

				int postresult=thisClient.POST(bodyText);
				if(m_dblog)
					m_dblog->printf(debug::dbInfo,"POST %s returned %d @ %s\r",bodyText.c_str(), postresult, thisClient.header((size_t)0).c_str());

				thisClient.end();
			}
			else
			{
				/* code */
				if(m_dblog)
					m_dblog->printf(debug::dbError,"httpbegin failed %s:%u:%s\r", eachHA->m_addr.toString().c_str(), eachHA->m_port,eachHA->m_endpoint.c_str());
			}
			


#else

#endif

		}		

	}

};


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

#endif // _announce_h