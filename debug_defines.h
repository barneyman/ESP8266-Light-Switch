
#define DEBUG_VERBOSE	100		// verbose info, not really useful
#define DEBUG_INFO		200		// debug info, semi useful
#define DEBUG_IMPORTANT 250
#define DEBUG_WARN		300		// worthy of note
#define DEBUG_ERROR		400		// attention required

#define DEBUG_DEBUG		1000	// what i'm currently fixing!

#define DEBUG_LEVEL		DEBUG_VERBOSE

#define DEBUG(a,b)	if(a>=DEBUG_LEVEL)	b


class debugBase
{
public:

	enum dbLevel { dbVerbose, dbInfo, dbImportant, dbWarning, dbError} m_currentLevel;

	debugBase(enum dbLevel currentLevel=dbImportant):m_currentLevel(currentLevel)
	{

	}

	void Debug(enum dbLevel level, String output)
	{
		if (level >= m_currentLevel)
			internalDebug(output);
	}

	void DebugVerbose(String out) { Debug(dbVerbose, out); }
	void DebugInfo(String out) { Debug(dbVerbose, out); }
	void DebugImportant(String out) { Debug(dbVerbose, out); }
	void DebugWarning(String out) { Debug(dbVerbose, out); }
	void DebugError(String out) { Debug(dbVerbose, out); }

	void DebugDebug(String out) { Debug(dbVerbose, out); }

protected:

	virtual void internalDebug(String out) = 0;

};

class SerialDebug : public debugBase
{
public:

	SerialDebug():debugBase()
	{}

protected:

	void internalDebug(String out)
	{
		Serial.print(out);
	}
};


