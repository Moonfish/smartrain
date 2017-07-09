/*
 * SmartRain.h
 *
 *  Created on: Dec 17, 2014
 *      Author: russell
 */

#ifndef SMARTRAIN_H_
#define SMARTRAIN_H_

#include <iostream>
#include <string>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <functional>
#include <thread>
#include <memory>
#include <mutex>

#include "LCD.h"
#include "RelayController.h"
#include "MessageQueue.h"
#include "WeatherService.h"
#include "EventLogger.h"
#include "util.h"

struct Status
{
  bool    connected;           // True if connected to Internet.
  bool    on;                  // True if on.
  int 		runningStation;      // The running station
  time_t 	stopTime;            // The time_t when the station should stop.
};

class ApplicationState;

struct Weather
{
  std::string status;
  double      pressure;
  double      humidity;
  double      temperature;
  double      windspeed;
  const double cNormalPressure = 27.3; // Inches Hg for Yucaipa

  Weather() : pressure(cNormalPressure), humidity(50.0), temperature(290.0), windspeed(0.0)
  {
    // Setup defaults (if no internet connection)
  }

  bool IsRaining()
  {
    if (status == "Rain" || status == "Snow" || status == "Hail" || status == "Sleet")
      return true;

    // High humidity + dropping pressure = likely rain.
    if (humidity >= 90 && pressure < cNormalPressure * 0.9)
      return true;

    return false;
  }

  bool IsWindy()
  {
    // Greater than 25 knot winds (too windy for sprinkler)
    return windspeed > 25.0;
  }

  bool IsCold()
  {
    return temperature < util::KelvinFromFarenheit(40.0);
  }
};

class SmartRain
{
public:
  SmartRain();
  ~SmartRain();

  void Run();

  void Enable(bool enable); // Enables or disables all valves.
  bool IsEnabled();         // Returns true if system is enabled.

  void StartStation(int station, int forMinutes);
  void StartStation(int station);
  void StopAllStations();
  bool IsStationOn(int station);
  bool IsStationDisabled(int station);
  void MoveToPreviousState();
  void MoveToState(std::shared_ptr<ApplicationState> newState);
  std::string GetStatusString();
  std::string GetFormatedTime();

  Weather GetWeather(){return m_weather;}

private:
  bool Init();
  bool LoadSettings();

  bool ProcessCommandFile();
  void UpdateStatusFile(const char* statusMessage);

  void DrawLCD();
  void ProcessSchedule();
  void UpdateWeatherInfo();
  bool AbortRun(std::string& reason);

  void MessageLoop();

  static void WorkerProc(SmartRain* sr);

  EventLogger       m_log;
  WeatherService    m_ws;
  Weather           m_weather;

  LCDPanel   	      m_lcd;
  RelayController   m_relay;

  Status            m_status;

  MessageQueue      m_msgQueue;
  std::shared_ptr<ApplicationState> m_previousState;
  std::shared_ptr<ApplicationState> m_currentState;
  std::mutex m_mux;

  std::string       m_startTime;
  int               m_runDurations[4];
  time_t            m_lastRain;

  volatile bool     m_shutdown;
  volatile bool     m_manualMode;
};

#endif /* SMARTRAIN_H_ */
