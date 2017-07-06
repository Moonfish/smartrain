/*
 * States.cpp
 *
 *  Created on: Jan 10, 2015
 *      Author: russell
 */
#include "States.h"
#include <sstream>
#include "NetworkInfo.h"

//////////////////////////

StartingState::StartingState(SmartRain& smartRain) : ApplicationState(smartRain)
{
}

void StartingState::OnMessage(MSG& msg)
{
  if (msg.type == MessageTypes::sm_key)
  {
    if (::isdigit(msg.data))
    {
      int station = msg.data - 0x30;
      if (station >= 0 && station < 4)
        m_smartRain.MoveToState(std::make_shared<ValveControlState>(m_smartRain, station));
    }
    else if (msg.data == '*')
    {
      m_smartRain.Enable(!m_smartRain.IsEnabled());
    }
    else if (msg.data == '#')
    {
      m_smartRain.MoveToState(std::make_shared<ShowIPState>(m_smartRain));
    }
  }
}

std::string StartingState::GetLine1()
{
  return m_smartRain.GetFormatedTime();
}

std::string StartingState::GetLine2()
{
  return m_smartRain.GetStatusString();
}

/////////////////////////

ValveControlState::ValveControlState(SmartRain& sm, int station) : ApplicationState(sm),
  m_station(station)
{
}

void ValveControlState::OnMessage(MSG& msg)
{
  if (msg.type == MessageTypes::sm_key)
  {
    if (msg.data == '#')
      m_smartRain.MoveToPreviousState();
    else if (msg.data == 'A')
      m_smartRain.StartStation(m_station, 10);
    else if (msg.data == 'B')
      m_smartRain.StopAllStations();
  }
}

std::string ValveControlState::GetLine1()
{
  std::stringstream ss;
  ss << "Station: " << m_station;
  return ss.str();
}

std::string ValveControlState::GetLine2()
{
  if (m_smartRain.IsStationOn(m_station))
    return "B=Stop #=Back";
  else
    return "A=Run  #=Back";
}

///////////////////////////////////////////////////////////////////

ErrorState::ErrorState(SmartRain& smartRain, const char* message) : ApplicationState(smartRain),
    m_message(message)
{
}

void ErrorState::OnMessage(MSG& msg)
{
}

std::string ErrorState::GetLine1()
{
  return "Error";
}

std::string ErrorState::GetLine2()
{
  return m_message;
}

///////////////////////////////////////////////////////////////////

ShowIPState::ShowIPState(SmartRain& smartRain) : ApplicationState(smartRain)
{
  NetworkInfo ni;
  m_message = ni.GetIP(1);
}

void ShowIPState::OnMessage(MSG& msg)
{
  if (msg.type == MessageTypes::sm_key)
  {
    if (msg.data == '#')
      m_smartRain.MoveToPreviousState();
  }
}

std::string ShowIPState::GetLine1()
{
  return m_message;
}

std::string ShowIPState::GetLine2()
{
  return "          #=Back";
}
