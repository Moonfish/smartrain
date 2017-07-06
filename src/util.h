/*
 * util.h
 *
 *  Created on: Jan 11, 2015
 *      Author: russell
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <unistd.h>
#include <time.h>

namespace util
{
  void msleep(int ms);

  time_t TimeFromHourMin(const char* hourMin);

  double FahrenheitFromKelvin(double kelvin);
  double KelvinFromFarenheit(double farenheit);
}




#endif /* UTIL_H_ */
