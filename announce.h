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
#include <ESP8266HTTPClient.h>
#endif


class StateAnnouncer
{

protected:

	class recipient
	{
		public:
		recipient(IPAddress addr, unsigned port, String &extra):m_addr(addr),m_port(port),m_extra(extra)
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
		String m_extra;
	};

	std::vector<recipient> m_HAhosts;
	StaticJsonBuffer<200> jsonBuffer;
	String m_extra;

	//unsigned m_port;

	debugBaseClass*dblog;

public:

	StateAnnouncer(debugBaseClass*dbg):dblog(dbg)
	{}


	// TODO - handle string extra implications
	virtual void AddAnnounceRecipient(IPAddress addr, unsigned port, String extra=String()) 
	{
		dblog->printf(debug::dbVerbose,"Adding recipient %s\r",addr.toString().c_str());
		recipient potential(addr,port,extra);
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
				dblog->println(debug::dbVerbose,"already exists");
			}
			else
			{
				dblog->println(debug::dbInfo,"port/extra has changed, re-mapping");
				finder->m_port=port;
				finder->m_extra=extra;
			}

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
		udproot["state"]=state?"on":"off";
		String bodyText;
		udproot.printTo(bodyText);

#ifdef _USE_UDP
		WiFiUDP sender;
#elif defined(_USE_TCP)
		WiFiClient sender;
#elif defined(_USE_REST)

		HTTPClient thisClient;

#else		
#endif		

		dblog->println(debug::dbInfo,"Starting IP yell");

		for(auto eachHA=m_HAhosts.begin();eachHA!=m_HAhosts.end();eachHA++)
		{

#ifdef _USE_UDP

			sender.beginPacket(eachHA->m_addr, eachHA->m_port);

			dblog->printf(debug::dbInfo,"udp %s to %s:%u\r",bodyText.c_str(), eachHA->m_addr.toString().c_str(),eachHA->m_port);
			sender.write(bodyText.c_str(),bodyText.length());

			sender.endPacket();

#elif defined (_USE_TCP)

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
#elif defined(_USE_REST)

			// extra is json
			//StaticJsonBuffer<500> jsonRestInfoBuffer;
			DynamicJsonBuffer jsonRestInfoBuffer;
			JsonObject& restInfo = jsonRestInfoBuffer.parseObject(eachHA->m_extra);
			String endPoint=restInfo["endpoint"];

			dblog->printf(debug::dbImportant,"Posting %s:%u%s\n\r", eachHA->m_addr.toString().c_str(), eachHA->m_port, endPoint.c_str());

			if (thisClient.begin(eachHA->m_addr.toString(), eachHA->m_port, endPoint )) {  

				//thisClient.setAuthorization(restInfo["auth"]);
				thisClient.addHeader("content-type", "application/json");

				String bearerToken("Bearer ");
				bearerToken+=(const char*)restInfo["auth"];
				thisClient.addHeader("Authorization", bearerToken);

				int postresult=thisClient.POST(bodyText);
				dblog->printf(debug::dbImportant,"POST %s returned %d\r",bodyText.c_str(), postresult);

				thisClient.end();
			}
			else
			{
				/* code */
				dblog->printf(debug::dbError,"httpbegin failed %s:%u:%s\r", eachHA->m_addr.toString().c_str(), eachHA->m_port,restInfo["endpoint"]);
			}
			


#else

#endif

		}		

	}

};
