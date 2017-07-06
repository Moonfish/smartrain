/*
 * CKeypad.h
 *
 *  Created on: Jul 12, 2014
 *      Author: russell
 */

#pragma once
#include <functional>
#include <thread>
#include <memory>
#include "MCP23008.h"

class CKeypad
{
public:
  CKeypad(const char* i2cDev, int id);
  virtual ~CKeypad();

  // Waits for key to be pressed and returns character.
  char ReadKey();

  void Initialize(std::function<int(char)> callback);

private:
  void WaitKeyDown();
  void WaitKeyUp();
  unsigned char ReadColumn();
  unsigned char ReadRow();

  MCP23008 m_mcp;
  volatile bool m_bExit;

  std::shared_ptr<std::thread> m_keyboardThread;
};


