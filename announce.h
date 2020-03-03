#include <Arduino.h>

#define _USE_UDP

#ifdef _USE_UDP
#else
#include <WiFiClient.h>
#endif


class StateAnnouncer
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
	StaticJsonBuffer<200> jsonBuffer;

	//unsigned m_port;

	debugBaseClass*dblog;

public:

	StateAnnouncer(debugBaseClass*dbg):dblog(dbg)
	{}


	virtual void AddAnnounceRecipient(IPAddress addr, unsigned port) 
	{
		dblog->printf(debug::dbVerbose,"Adding recipient %s\r",addr.toString().c_str());
		recipient potential(addr,port);
		auto finder=std::find(m_HAhosts.begin(),m_HAhosts.end(),potential);
		if(finder==m_HAhosts.end())
		{
			m_HAhosts.push_back(potential);
			dblog->println(debug::dbVerbose,"ADDED");
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
				dblog->println(debug::dbVerbose,"port has changed, re-mapping");
				finder->m_port=port;
			}

		}
	}


public:


	void SendState(bool state)
	{
		// 
		jsonBuffer.clear();
		JsonObject& udproot = jsonBuffer.createObject();
		udproot["state"]=state;
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
