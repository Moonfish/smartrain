//
//  MCP23008.h
//  LCDTest
//
//  Created by Russell Louks on 6/21/14.
//  Copyright (c) 2014 Russell. All rights reserved.
//

#pragma once
#include <mutex>

/*
 
 **** MCP23008 PDIP/SOIC wiring ****
 
 P1       : SCL
 P2       : SDA
 P3-P5    : GND
 P6       : VDD
 P7       : N/C
 P8       : N/C
 P9       : GND
 P10-P17  : data lines
 P18      : VDD

*/

class MCP23008
{
  public:
  MCP23008(const char* device, int i2cAddr);
  ~MCP23008();
  
  void set_IODIR(char value);
  char get_IODIR();
  
  void set_IPOL(char value);
  char get_IPOL();
  
  void set_GPINTEN(char value);
  char get_GPINTEN();

  void set_DEFVAL(char value);
  char get_DEFVAL();

  void set_INTCON(char value);
  char get_INTCON();

  void set_IOCON(char value);
  char get_IOCON();

  void set_GPPU(char value);
  char get_GPPU();

  void set_INTF(char value);
  char get_INTF();

  char get_INTCAP(); // Read only

  void set_GPIO(char value);
  char get_GPIO();

  void set_OLAT(char value);
  char get_OLAT();
  
  // Prints registers
  void PrintRegisters();
  
  private:

  static std::mutex m_mux;

  void WriteIO(int reg, char value);
  char ReadIO(int reg);
  
  int m_fd;
  int m_i2cAddr;
};




