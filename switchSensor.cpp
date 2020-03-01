#include "switchSensor.h"
#include <onewire.h>
#include <DallasTemperature.h>



RelayLEDandSwitch* RelayLEDandSwitch::m_singleton=NULL;
GPIOInstantSensor* GPIOInstantSensor::m_singleton=NULL;
MCP23017MultiSwitch* MCP23017MultiSwitch::m_singleton=NULL;


