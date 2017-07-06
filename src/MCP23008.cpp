//
//  MCP23008.cpp
//  LCDTest
//
//  Created by Russell Louks on 6/21/14.
//  Copyright (c) 2014 Russell. All rights reserved.
//
#include <iostream> 
#include <string>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdexcept>
#include <iomanip>
#include "i2c-dev.h"
#include "MCP23008.h"

std::mutex MCP23008::m_mux;

MCP23008::MCP23008(const char* device, int i2cAddr) : 
  m_fd(-1), m_i2cAddr(i2cAddr)
{
  m_fd = open(device, O_RDWR);
  if (m_fd < 0)
  {
    std::string msg("Could not open device ");
    msg += device;
    throw std::runtime_error(msg.c_str());
  }

  int chk = ioctl(m_fd, I2C_SLAVE, m_i2cAddr);

  if (chk == -1)
    std::cout << "Error initializing MCP23008." << std::endl;
  else
    std::cout << "MCP23008 Initialized." << std::endl;
}

MCP23008::~MCP23008()
{
  if (m_fd >= 0)
    close(m_fd);
}

void MCP23008::set_IODIR(char value)
{
  WriteIO(0, value);
}
char MCP23008::get_IODIR()
{
  return ReadIO(0);
}

void MCP23008::set_IPOL(char value)
{
  WriteIO(1, value);
}
char MCP23008::get_IPOL()
{
  return ReadIO(1);
}

void MCP23008::set_GPINTEN(char value)
{
  WriteIO(2, value);
}
char MCP23008::get_GPINTEN()
{
  return ReadIO(2);
}

void MCP23008::set_DEFVAL(char value)
{
  WriteIO(3, value);
}
char MCP23008::get_DEFVAL()
{
  return ReadIO(3);
}

void MCP23008::set_INTCON(char value)
{
  WriteIO(4, value);
}
char MCP23008::get_INTCON()
{
  return ReadIO(4);
}

void MCP23008::set_IOCON(char value)
{
  WriteIO(5, value);
}
char MCP23008::get_IOCON()
{
  return ReadIO(5);
}

void MCP23008::set_GPPU(char value)
{
  WriteIO(6, value);
}
char MCP23008::get_GPPU()
{
  return ReadIO(6);
}

void MCP23008::set_INTF(char value)
{
  WriteIO(7, value);
}
char MCP23008::get_INTF()
{
  return ReadIO(7);
}

char MCP23008::get_INTCAP() // Read only
{
  return ReadIO(8);
}

void MCP23008::set_GPIO(char value)
{
  WriteIO(9, value);
}
char MCP23008::get_GPIO()
{
  return ReadIO(9);
}

void MCP23008::set_OLAT(char value)
{
  WriteIO(10, value);
}
char MCP23008::get_OLAT()
{
  return ReadIO(10);
}

void MCP23008::WriteIO(int reg, char value)
{
  m_mux.lock();
  i2c_smbus_write_byte_data(m_fd, reg, value);
  m_mux.unlock();
}

char MCP23008::ReadIO(int reg)
{
  m_mux.lock();
  char ret = i2c_smbus_read_byte_data(m_fd, reg) & 0xFF;
  m_mux.unlock();
  return ret;
}

void MCP23008::PrintRegisters()
{
  std::cout << "  IODIR:  " << std::hex << std::setw(3) << (int)get_IODIR();
  std::cout << "  IPOL:   " << std::hex << std::setw(3) << (int)get_IPOL();
  std::cout << "  GPINTEN:" << std::hex <<  std::setw(3) << (int)get_GPINTEN() << std::endl;
  std::cout << "  DEFVAL: " << std::hex <<  std::setw(3) << (int)get_DEFVAL();
  std::cout << "  INTCON: " << std::hex <<  std::setw(3) << (int)get_INTCON();
  std::cout << "  IOCON:  " << std::hex <<  std::setw(3) << (int)get_IOCON() << std::endl;
  std::cout << "  GPPU:   " << std::hex <<  std::setw(3) << (int)get_GPPU();
  std::cout << "  INTF:   " << std::hex <<  std::setw(3) << (int)get_INTF();
  std::cout << "  INTCAP: " << std::hex <<  std::setw(3) << (int)get_INTCAP() << std::endl;
  std::cout << "  GPIO:   " << std::hex <<  std::setw(3) << (int)get_GPIO();
  std::cout << "  OLAT:   " << std::hex <<  std::setw(3) << (int)get_OLAT() << std::endl;
}

