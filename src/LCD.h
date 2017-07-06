#ifndef _LCD_H
#define _LCD_H

#include "MCP23008.h"

class LCDPanel
{
public:
  LCDPanel(const char* device, int i2cAddress);
  ~LCDPanel();
  
  void Clear();
  void PrintLine(int lineNum, const char* text);
  
private:
  void PrintChar(char c);
  void SetLine(int line);

  void SetMCPBit(int b);
  void SetMCPBits(int bits);
  void ResetMCPBit(int b);
  void PulseMCPBit(int b);
  void ClearMCPBits();
  void Pulse();

  MCP23008 m_mcp;
  char     m_data;
};

#endif

