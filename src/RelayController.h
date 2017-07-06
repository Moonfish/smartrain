/*
 * RelayController.h
 *
 *  Created on: Dec 20, 2014
 *      Author: russell
 */

#ifndef RELAYCONTROLLER_H_
#define RELAYCONTROLLER_H_

#include "MCP23008.h"

class RelayController
{
public:
  RelayController(const char* device, int i2cAddress);
  ~RelayController();

  void RelayOn(int relay);
  bool IsRelayOn(int relay);
  void RelaysOff();
  void Test();

private:
  MCP23008 m_mcp;
  volatile int m_currentlyOnValve;
};

#endif /* RELAYCONTROLLER_H_ */
