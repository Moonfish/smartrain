//
//  BoneIO.h
//  BoneIO
//
//  Created by Russell Louks on 2/16/14.
//  Copyright (c) 2014 Russell. All rights reserved.
//

#ifndef __BoneIO__BoneIO__
#define __BoneIO__BoneIO__

#include <iostream>
#include <string>
#include <linux/i2c-dev.h>

namespace BoneIO
{

enum PinMode
{
  read,
  write
};

class Pin
{
public:
  int PinNumber();
  void Export();
  void Unexport();
  void SetMode(PinMode mode);
  void Set();
  void Reset();
  void Pulse();
  bool Read();
  
private:
  Pin(int pinNum, const char* pinName);
  void IOWrite(const char* path, const char* value);
  std::string IORead(const char* path);
  
  friend class CBoneIO;
  std::string _pinName;
  int _pinNumber;
};

class CBoneIO
{
public:
  static Pin& Pin11();
  static Pin& Pin12();
  static Pin& Pin15();
  static Pin& Pin16();
  static Pin& Pin17();
  static Pin& Pin18();
  
private:
  
  static Pin& CreatePin(int pinNum, const char* pinName);
};

} // namespace BoneIO

#endif /* defined(__BoneIO__BoneIO__) */
