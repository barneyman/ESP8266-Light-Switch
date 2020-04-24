#include "switchSensor.h"



RelayLEDandSwitch* RelayLEDandSwitch::m_singleton=NULL;
GPIOInstantSensor* GPIOInstantSensor::m_singleton=NULL;

#ifndef ARDUINO_ESP8266_GENERIC

MCP23017MultiSwitch* MCP23017MultiSwitch::m_singleton=NULL;

#endif

