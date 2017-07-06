/*
 * util.cpp
 *
 *  Created on: Jan 11, 2015
 *      Author: russell
 */

#include "util.h"
#include <string>

namespace util
{
  void msleep(int ms)
  {
    usleep(1000*ms);
  }

  time_t TimeFromHourMin(const char* hourMin)
  {
    std::string sHourMin(hourMin);
    if (sHourMin.empty())
      return 0;

    int pos = sHourMin.find(':');
    int hour = std::stoi(sHourMin.substr(0, pos));
    int min = std::stoi(sHourMin.substr(pos + 1));

    time_t currentTime;
    time(&currentTime);
    tm* tm = localtime(&currentTime);
    tm->tm_hour = hour;
    tm->tm_min = min;

    return mktime(tm);
  }

  double FahrenheitFromKelvin(double kelvin)
  {
    return (kelvin - 273.15) * 1.8 + 32.0;
  }

  double KelvinFromFarenheit(double farenheit)
  {
    return (farenheit + 459.67)*(5.0/9.0);
  }
}

