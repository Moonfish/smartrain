/*
 * States.h
 *
 *  Created on: Jan 10, 2015
 *      Author: russell
 */

#ifndef STATES_H_
#define STATES_H_

#include <string>
#include "SmartRain.h"
#include "MessageQueue.h"

class ApplicationState
{
public:
  ApplicationState(SmartRain& smartRain): m_smartRain(smartRain)
  {
  }
  virtual ~ApplicationState(){}

  virtual void OnMessage(MSG& msg) = 0;
  virtual std::string GetLine1() = 0;
  virtual std::string GetLine2() {return "";}

protected:
  SmartRain&  m_smartRain;
};

class StartingState : public ApplicationState,
  public std::enable_shared_from_this<StartingState>
{
public:
  StartingState(SmartRain& smartRain);
  void OnMessage(MSG& msg);
  std::string GetLine1();
  std::string GetLine2();
};

class ValveControlState : public ApplicationState,
  public std::enable_shared_from_this<ValveControlState>
{
public:
  ValveControlState(SmartRain& smartRain, int station);
  void OnMessage(MSG& msg);
  std::string GetLine1();
  std::string GetLine2();

private:
  int m_station;
};

class ErrorState : public ApplicationState
{
public:
  ErrorState(SmartRain& smartRain, const char* message);
  void OnMessage(MSG& msg);
  std::string GetLine1();
  std::string GetLine2();
private:
  std::string m_message;
};

class ShowIPState : public ApplicationState
{
public:
  ShowIPState(SmartRain& smartRain);
  void OnMessage(MSG& msg);
  std::string GetLine1();
  std::string GetLine2();
private:
  std::string m_message;
};

#endif /* STATES_H_ */
