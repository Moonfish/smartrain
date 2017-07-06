/*
 * CKeypad.cpp
 *
 *  Created on: Jul 12, 2014
 *      Author: russell
 */

#include "Keypad.h"
#include <unistd.h>
#include <iostream>
#include <cmath>
#include <future>

static void sleepm(int milliseconds)
{
  usleep(milliseconds * 1000);
}

CKeypad::CKeypad(const char* i2cDev, int id): m_mcp(i2cDev, id),
  m_bExit(false)
{
  // Initialize MCP
  m_mcp.set_IODIR(0xF0);
  m_mcp.set_INTCON(0xF0);
  m_mcp.set_DEFVAL(0xF0);
  m_mcp.set_GPPU(0x0F);
  m_mcp.set_GPINTEN(0xF0);
}

CKeypad::~CKeypad()
{
  m_bExit = true;
}

void CKeypad::Initialize(std::function<int(char)> callback)
{
  m_keyboardThread = std::make_shared<std::thread>([this, callback]() ->int
  {
    while (!m_bExit)
    {
      char key = this->ReadKey();
      callback(key);
    }

    return 0;
  });
}

void CKeypad::WaitKeyDown()
{
  // Wait for keydown.
  while(m_mcp.get_INTF() == 0x0){sleepm(50);}
}

void CKeypad::WaitKeyUp()
{
  // Wait for keyup.
  while(m_mcp.get_INTF() != 0x0)
  {
    sleepm(50);

    // Attempt to clear interrupt (will not clear if key is still down).
    m_mcp.get_GPIO();
  };
}

char CKeypad::ReadKey()
{
  static const char charLUT[4][4] = {{'1', '2', '3', 'A'},
				     {'4', '5', '6', 'B'},
	                             {'7', '8', '9', 'C'},
		                     {'*', '0', '#', 'D'}};

  WaitKeyDown();

  unsigned char col = ReadColumn();
  unsigned char row = ReadRow();

  WaitKeyUp();

  return charLUT[row][col];
}

unsigned char CKeypad::ReadColumn()
{
  // Configure I/O for columns
  m_mcp.set_IODIR(0xF0);
  m_mcp.set_INTCON(0xF0);
  m_mcp.set_DEFVAL(0xF0);
  m_mcp.set_GPINTEN(0xF0);

  return (unsigned char)log2((~m_mcp.get_INTCAP() >> 4) & 0x0F);
}

unsigned char CKeypad::ReadRow()
{
  // Configure I/O for rows
  m_mcp.set_IODIR(0x0F);
  m_mcp.set_INTCON(0x0F);
  m_mcp.set_DEFVAL(0x0F);
  m_mcp.set_GPINTEN(0x0F);

  return (unsigned char)(log2(~m_mcp.get_GPIO() & 0xF));
}


