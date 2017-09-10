#include <string.h>
#include <unistd.h>
#include <iostream>
#include "LCD.h"

/******* MCP to LCD wiring map ********/
/*
 * GPIO 0 -> DB4
 * GPIO 1 -> DB5
 * GPIO 2 -> DB6
 * GPIO 3 -> DB7
 * GND    -> R/W
 * GPIO 4 -> RS
 * GPIO 5 -> E
 * GPIO 6 (not connected)
 * GPIO 7 (not connected)
 */



enum Pins
{
  DB4 = 1,
  DB5 = 2,
  DB6 = 4,
  DB7 = 8,
  RS  = 16,
  E   = 32
};

void LCDPanel::Pulse()
{
  this->PulseMCPBit(Pins::E);    // Pulse E
}

LCDPanel::LCDPanel(const char* device, int i2cAddress) : m_mcp(device, i2cAddress), m_data(0)
{
  // Initialize MCP
  m_mcp.set_IODIR(0x00);  // Set to output mode.

  usleep(1600);           // Wait for power up

  // Initialize LCD controller using Triple pulse reset
  this->SetMCPBits(Pins::DB4|Pins::DB5);
  Pulse();
  Pulse();
  Pulse();

  // Enter 4 bit mode.
  this->SetMCPBits(Pins::DB5);
  Pulse();

  // Enable double line.
  this->ClearMCPBits();
  Pulse();
  this->SetMCPBits(Pins::DB7|Pins::DB6);
  Pulse();

  Clear();
}

LCDPanel::~LCDPanel()
{
  Clear();

  // Go back to 8 bit mode.
  this->SetMCPBits(Pins::DB5);   // Set D5
  Pulse();
}

void LCDPanel::SetMCPBits(int bits)
{
  m_data = bits;
  m_mcp.set_GPIO(m_data);
}

void LCDPanel::SetMCPBit(int b)
{
  m_data = m_data | b;
  m_mcp.set_GPIO(m_data);
}

void LCDPanel::ResetMCPBit(int b)
{
  m_data = m_data & (~b);
  m_mcp.set_GPIO(m_data);
}

void LCDPanel::PulseMCPBit(int b)
{
  usleep(40);
  SetMCPBit(b);
  usleep(40);
  ResetMCPBit(b);
  usleep(40);
}

void LCDPanel::ClearMCPBits()
{
  m_data = 0;
  m_mcp.set_GPIO(m_data);
}

void LCDPanel::Clear()
{
  // Clear
  this->ClearMCPBits();
  Pulse();
  this->SetMCPBits(Pins::DB4);   // Set D4
  Pulse();
  usleep(1600);
}

void LCDPanel::PrintLine(int lineNum, const char* text)
{
  int n = lineNum;
  if (n < 0)
    n = 0;
  if (n > 1)
    n = 1;

  SetLine(n);
  
  size_t strLen = ::strlen(text);

  if (strLen <=0 || strLen > 16)
    return;
  
  for (size_t i = 0; i < strLen; i++)
    PrintChar(text[i]);

  for (size_t r = 0; r < 16-strLen; r++)
    PrintChar(' ');
}

void LCDPanel::SetLine(int line)
{
  this->ClearMCPBits();

  this->SetMCPBit(Pins::DB7);
 
  if (line == 1)
    this->SetMCPBit(Pins::DB6);

  Pulse();

  this->ClearMCPBits();
  Pulse();
}

void LCDPanel::PrintChar(char c)
{
  this->SetMCPBits(Pins::RS);

  if (c & 16)
    this->SetMCPBit(Pins::DB4);

  if (c & 32)
    this->SetMCPBit(Pins::DB5);

  if (c & 64)
    this->SetMCPBit(Pins::DB6);

  if (c & 128)
    this->SetMCPBit(Pins::DB7);

  Pulse();

  this->SetMCPBits(Pins::RS);

  if (c & 1)
    this->SetMCPBit(Pins::DB4);

  if (c & 2)
    this->SetMCPBit(Pins::DB5);

  if (c & 4)
    this->SetMCPBit(Pins::DB6);

  if (c & 8)
    this->SetMCPBit(Pins::DB7);

  Pulse();
}

