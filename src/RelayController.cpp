/*
 * RelayController.cpp
 *
 *  Created on: Dec 20, 2014
 *      Author: russell
 */

#include <iostream>
#include "RelayController.h"
#include "util.h"

RelayController::RelayController(const char* device, int i2cAddress) : m_mcp(device, i2cAddress),
  m_currentlyOnValve(-1)
{
  m_mcp.set_IODIR(0x00);
  RelaysOff();
}

RelayController::~RelayController()
{
  RelaysOff();
}

void RelayController::RelayOn(int relay)
{
  if (relay < 0 || relay > 3)
    return;

  std::cout << "Turning relay " << relay << " on\r\n";

  m_currentlyOnValve = relay;

  char val = (1 << relay) << 4;

  m_mcp.set_GPIO(val);
  util::msleep(50);
}

bool RelayController::IsRelayOn(int relay)
{
  return (relay == m_currentlyOnValve);
}

void RelayController::RelaysOff()
{
  if (m_currentlyOnValve == -1)
    return;

  std::cout << "Turning relays off\r\n";

  m_currentlyOnValve = -1;
  m_mcp.set_GPIO(0x0);
  util::msleep(50);
}
