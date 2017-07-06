//
//  BoneIO.cpp
//  BoneIO
//
//  Created by Russell Louks on 2/16/14.
//  Copyright (c) 2014 Russell. All rights reserved.
//

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "BoneIO.h"

using namespace std;

namespace BoneIO
{

typedef map<int, Pin*> PINMAP;
static PINMAP sPinMap;

static const char sRoot[] = "/sys/class/gpio/";

Pin& CBoneIO::Pin11()
{
  return CreatePin(11, "45");
}

Pin& CBoneIO::Pin12()
{
  return CreatePin(12, "44");
}

Pin& CBoneIO::Pin15()
{
  return CreatePin(15, "47");
}

Pin& CBoneIO::Pin16()
{
  return CreatePin(16, "46");
}

Pin& CBoneIO::Pin17()
{
  return CreatePin(17, "27");
}

Pin& CBoneIO::Pin18()
{
  return CreatePin(18, "65");
}

Pin& CBoneIO::CreatePin(int pinNum, const char* pinName)
{
  PINMAP::iterator it = sPinMap.find(pinNum);
  if (it != sPinMap.end())
    return *(it->second);

  Pin* pin = new Pin(pinNum, pinName);
  sPinMap.insert(PINMAP::value_type(pinNum, pin));

  return *pin;
}

//////////////////////////////////////////////////

Pin::Pin(int pinNum, const char* pinName) : _pinNumber(pinNum)
{
  _pinName = pinName;
}

int Pin::PinNumber()
{
  return _pinNumber;
}

void Pin::Export()
{
  IOWrite("export", _pinName.c_str());
}

void Pin::Unexport()
{
  IOWrite("unexport", _pinName.c_str());
}

void Pin::SetMode(PinMode mode)
{
  string path("gpio");
  path += _pinName;
  path += "/direction";

  if (mode == read)
    IOWrite(path.c_str(), "in");
  else
    IOWrite(path.c_str(), "out");
}

void Pin::Set()
{
  string path("gpio");
  path += _pinName;
  path += "/value";

  IOWrite(path.c_str(), "1");
}

void Pin::Reset()
{
  string path("gpio");
  path += _pinName;
  path += "/value";

  IOWrite(path.c_str(), "0");
}

void Pin::Pulse()
{
  usleep(500);
  Set();
  usleep(500);
  Reset();
}

bool Pin::Read()
{
  string path("gpio");
  path += _pinName;
  path += "/value";

  string value = IORead(path.c_str());
  if (value == "0")
    return false;

  return true;
}

void Pin::IOWrite(const char* path, const char* value)
{
  string fullPath(sRoot);
  fullPath += path;
  ofstream of(fullPath.c_str());
  of << value << endl;
  of.close();
}

string Pin::IORead(const char* path)
{
  string fullPath(sRoot);
  fullPath += path;
  ifstream ifs(fullPath.c_str());

  string value;
  ifs >> value;

  ifs.close();

  return value;
}

} // namespace BoneIO
