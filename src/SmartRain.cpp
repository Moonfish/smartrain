
/*
 * SmartRain.cpp
 *
 *  Created on: Dec 17, 2014
 *      Author: russell
 */

#include <sstream>
#include <fstream>
#include <time.h>
#include <sys/inotify.h>
#include "SmartRain.h"
#include "States.h"
#include "util.h"
#include "json11.hpp"
#include "NetworkInfo.h"

int main(int argc, const char* argv[])
{
  SmartRain sm;
  sm.Run();

  return 0;
}

SmartRain::SmartRain() : m_log("./log.html"), m_lcd("/dev/i2c-2", 0x21), m_relay("/dev/i2c-2", 0x24),
    m_previousState(nullptr), m_currentState(nullptr), m_lastRain(0), m_manualMode(false)
{
  m_shutdown = false;
  m_status.runningStation = -1;
  m_status.on = true;
}

SmartRain::~SmartRain ()
{
}

void SmartRain::Run()
{
  if (!Init())
    return;
}

void SmartRain::WorkerProc(SmartRain* sr)
{
  while(!sr->m_shutdown)
  {
    sr->DrawLCD();
    sr->ProcessSchedule();
    sr->UpdateWeatherInfo();

    // Update status and check schedule once every second.
    util::msleep(300);
  }
}

bool SmartRain::Init()
{
  ::chdir("/home/russell/smartrain");

  std::cout << "Initializing SmartRain..." << std::endl;

  m_log.WriteEvent("Restarting Smartrain");

  std::cout << "Obtaining IP address." << std::endl;
  NetworkInfo ni;
  std::string ipMsg = "IP Address: ";
  ipMsg += ni.GetIP(1);
  m_log.WriteEvent(ipMsg.c_str());

  m_lcd.PrintLine(0, "SmartRain 1.1");

  LoadSettings();
  m_lcd.Clear();

  m_currentState =  std::make_shared<StartingState>(*this);

  // Start background thread that checks timer
  std::thread worker(WorkerProc, this);

  // Process messages
  MessageLoop();

  std::cout << "Shutting down...\n";
  m_shutdown = true;
  worker.join();

  return true;
}

bool SmartRain::LoadSettings()
{
  static const char* fileName = "settings.txt";

  struct stat attr = {0};
  stat(fileName, &attr);

  static int sLastMod = 0;
  if (attr.st_mtime == sLastMod)
    return true;

  sLastMod = attr.st_mtime;

  std::cout << "Loading settings file..." << std::endl;

  if (sLastMod == 0)
    m_log.WriteEvent("Loading configuration..."); 
  else
    m_log.WriteEvent("Reloading configuration...");

  std::ifstream fs(fileName);

  if (fs.fail())
  {
    m_log.WriteEvent("Error reading configuration file.");
    std::cout << "Could not read settings file." << std::endl;
    return false;
  }

  std::string content((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());

  std::string err;
  auto jo = json11::Json::parse(content.c_str(), err);

  m_startTime = jo["startTimeT"].string_value();

  m_runDurations[0] = std::stoi(jo["runtime1T"].string_value().c_str());
  m_runDurations[1] = std::stoi(jo["runtime2T"].string_value().c_str());
  m_runDurations[2] = std::stoi(jo["runtime3T"].string_value().c_str());
  m_runDurations[3] = std::stoi(jo["runtime4T"].string_value().c_str());

  UpdateWeatherInfo();

  return true;
}

void SmartRain::Enable(bool enable)
{
  m_status.on = enable;

  if (!enable)
  {
    m_log.WriteEvent("System disabled.");

    StopAllStations();
  }
  else
    m_log.WriteEvent("System enabled.");
}

bool SmartRain::IsEnabled()
{
  return m_status.on;
}

void SmartRain::StartStation(int station)
{
  if (station < 0 || station > 3)
    return;

  StopAllStations();

  m_status.runningStation = station;

  time_t now;
  time(&now);
  m_status.stopTime = now + m_runDurations[station] * 60;

  std::stringstream logMsg;
  logMsg << "Running station " << station << ".";
  m_log.WriteEvent(logMsg.str().c_str());

  m_relay.RelayOn(station);

  std::stringstream msgStrm;
  msgStrm << "S" << station << " running";
  UpdateStatusFile(msgStrm.str().c_str());
}

void SmartRain::StartStation(int station, int forMinutes)
{
  if (station < 0 || station > 3)
    return;

  StopAllStations();

  m_manualMode = true;

  m_status.runningStation = station;

  time_t now;
  time(&now);
  m_status.stopTime = now + forMinutes * 60;

  m_relay.RelayOn(station);

  std::stringstream msgStrm;
  msgStrm << "S" << station << " running";
  UpdateStatusFile(msgStrm.str().c_str());
}

bool SmartRain::IsStationOn(int station)
{
  return m_relay.IsRelayOn(station);
}

void SmartRain::StopAllStations()
{
  m_manualMode = false;
  m_status.runningStation = -1;
  m_relay.RelaysOff();

  std::stringstream msgStrm;
  if (IsEnabled())
    msgStrm << "Ready";
  else
    msgStrm << "Disabled";

  UpdateStatusFile(msgStrm.str().c_str());
}

std::string SmartRain::GetFormatedTime()
{
  time_t rawTime;
  time(&rawTime);

  char formatedTime[64] = {0};
  ::strftime(formatedTime, 64, "%h %d %H:%M:%S", localtime(&rawTime));

  return formatedTime;
}

std::string SmartRain::GetStatusString()
{
  std::stringstream strm;

  if (m_status.connected)
    strm << "@ ";
  else
    strm << "  ";

  if (m_status.on)
    strm << "On ";
  else
    strm << "Off";

  static bool sBlinkPhase = true;

  if (m_status.runningStation != -1)
  {
    if (sBlinkPhase)
     strm << "   " << m_status.runningStation << " ";
    else
     strm << "     ";

    sBlinkPhase = !sBlinkPhase;

    // Get the current time
    time_t rawTime;
    time(&rawTime);

    if (rawTime >= m_status.stopTime && m_manualMode)
    {
      StopAllStations();
      return strm.str();
    }

    char minRemaining[32] = {0};
    time_t balance = m_status.stopTime - rawTime;

    ::strftime(minRemaining, 32, "%M:%S", localtime(&balance));
    strm << minRemaining;
  }
  else
  {
    strm << " " << m_weather.status;
  }

  return strm.str();
}

void SmartRain::DrawLCD()
{
  m_mux.lock();
  auto state = m_currentState;
  m_mux.unlock();

  m_lcd.PrintLine(0, state->GetLine1().c_str());
  m_lcd.PrintLine(1, state->GetLine2().c_str());
}

void SmartRain::UpdateWeatherInfo()
{
  static time_t lastCollected = 0;

  time_t now = time(0);

  // Collect every hour.
  if (lastCollected == 0 || (now - lastCollected) > 3600)
  {
    lastCollected = now;

    auto wd = m_ws.GetData();
    if (wd.IsGood())
    {
      m_weather.status = wd.GetStatus();
      m_weather.humidity = wd.GetHumidity();
      m_weather.pressure = wd.GetPressure();
      m_weather.windspeed = wd.GetWindSpeed();
      m_weather.temperature = wd.GetMaxTemp();

      std::stringstream ss;
      ss << m_weather.status;
      ss << " [H:" << m_weather.humidity << "%, P:";
      ss << m_weather.pressure << ", W:";
      ss << m_weather.windspeed << ", T:";
      ss << util::FahrenheitFromKelvin(m_weather.temperature) << "]";

      m_log.WriteEvent(ss.str());

      if (m_weather.IsRaining())
        m_lastRain = now;
    }
    else
      m_log.WriteEvent("Unable to update weather information.");
  }
}

bool SmartRain::AbortRun(std::string& reason)
{
  if (m_manualMode)
    return false;

  if (!m_status.on)
  {
    reason = "Sprinkler run aborted, system disabled.";
    return true;
  }

  // Disable sprinklers if raining
  if (m_weather.IsRaining())
  {
    reason = "Sprinkler run aborted due to parcipitation.";
    return true;
  }

  // Disable sprinklers if recent rain
  time_t now = time(0);
  if (now - m_lastRain < 3600 * 24)
  {
    reason = "Sprinkler run aborted due to parcipitation within last 24 hours.";
    return true;
  }

  // Disable sprinklers if too windy
  if (m_weather.IsWindy())
  {
    reason = "Sprinkler run aborted due to high winds.";
    return true;
  }

  // Disable sprinklers if too cold
  if (m_weather.IsCold())
  {
    reason = "Sprinkler run aborted due to low temperatures.";
    return true;
  }

  return false;
}

void SmartRain::ProcessSchedule()
{
  ProcessCommandFile();

  // Changes may have been made
  LoadSettings();

  time_t run0Start = util::TimeFromHourMin(m_startTime.c_str());
  time_t run1Start = run0Start + m_runDurations[0] * 60;
  time_t run2Start = run1Start + m_runDurations[1] * 60;
  time_t run3Start = run2Start + m_runDurations[2] * 60;
  time_t stop =      run3Start + m_runDurations[3] * 60;

  // Get the current time
  time_t rawTime;
  time(&rawTime);

  static std::string sReason = "";
  std::string reason;

  // Turn on sprinkler if necessary
  if (rawTime >= run0Start && rawTime < run1Start)
  {
    if (!IsStationOn(0))
    {
      if (AbortRun(reason))
      {
        if (reason != sReason)
        {
          sReason = reason;
          m_log.WriteEvent(reason);
        }
        return;
      }

      std::cout << "Running station 0\n";
      StartStation(0);
    }
  }
  else if (rawTime >= run1Start && rawTime < run2Start)
  {
    if (!IsStationOn(1))
    {
      if (AbortRun(reason))
      {
        if (reason != sReason)
        {
          sReason = reason;
          m_log.WriteEvent(reason);
        }
        return;
      }

      std::cout << "Running station 1\n";
      StartStation(1);
    }
  }
  else if (rawTime >= run2Start && rawTime < run3Start)
  {
    if (!IsStationOn(2))
    {
      if (AbortRun(reason))
      {
        if (reason != sReason)
        {
          sReason = reason;
          m_log.WriteEvent(reason);
        }
        return;
      }

      std::cout << "Running station 2\n";
      StartStation(2);
    }
  }
  else if (rawTime >= run3Start && rawTime < stop)
  {
    if (!IsStationOn(3))
    {
      if (AbortRun(reason))
      {
        if (reason != sReason)
        {
          sReason = reason;
          m_log.WriteEvent(reason);
        }
        return;
      }

      std::cout << "Running station 3\n";
      StartStation(3);
    }
  }
  else
  {
    if (!m_manualMode)
      StopAllStations(); // Stop all
  }
}

void SmartRain::MessageLoop()
{
  while(true)
  {
    MSG msg = {MessageTypes::sm_none, 0};
    if (!m_msgQueue.GetMessage(msg))
      break;

    m_mux.lock();
    auto state = m_currentState;
    m_mux.unlock();

    state->OnMessage(msg);
  }
}

void SmartRain::MoveToPreviousState()
{
  m_mux.lock();
  std::shared_ptr<ApplicationState> currentState = m_currentState;
  m_currentState = m_previousState;
  m_previousState = currentState;
  m_mux.unlock();
}

void SmartRain::MoveToState(std::shared_ptr<ApplicationState> newState)
{
  m_mux.lock();
  std::shared_ptr<ApplicationState> currentState = m_currentState;
  m_currentState = newState;
  m_previousState = currentState;
  m_mux.unlock();
}

bool SmartRain::ProcessCommandFile()
{
  const char* fileName = "command.txt";
  std::ifstream cmdFile(fileName);
  if (cmdFile.fail())
      return false;

  std::string command((std::istreambuf_iterator<char>(cmdFile)), std::istreambuf_iterator<char>());
  cmdFile.close();

  ::remove(fileName);

  if (0 == command.compare("R1"))
    StartStation(0, 15);
  else if (0 == command.compare("R2"))
    StartStation(1, 15);
  else if (0 == command.compare("R3"))
    StartStation(2, 15);
  else if (0 == command.compare("R4"))
    StartStation(3, 15);
  else if (0 == command.compare("S"))
    StopAllStations();
  else
  {
    std::cout << "Unknown command" << std::endl;
    return false;
  }

  return true;
}

void SmartRain::UpdateStatusFile(const char* statusMessage)
{
  std::string message(statusMessage);

  std::ofstream statusFile("status.txt");

  std::string line = "{\"statusT\": \"";
  line += message;
  line += "\"}";

  statusFile.write(line.c_str(), line.length());
  statusFile.flush();
  statusFile.close();
}
