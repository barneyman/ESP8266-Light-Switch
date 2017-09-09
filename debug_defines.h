
#define DEBUG_VERBOSE	100		// verbose info, not really useful
#define DEBUG_INFO		200		// debug info, semi useful
#define DEBUG_IMPORTANT 250
#define DEBUG_WARN		300		// worthy of note
#define DEBUG_ERROR		400		// attention required

#define DEBUG_DEBUG		1000	// what i'm currently fixing!

#define DEBUG_LEVEL		DEBUG_IMPORTANT

#define DEBUG(a,b)	if(a>=DEBUG_LEVEL)	b
