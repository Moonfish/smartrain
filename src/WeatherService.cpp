#include <boost/asio.hpp>
#include <string>
#include <sstream>
#include "WeatherService.h"
#include "json11.hpp"

using boost::asio::ip::tcp;

WeatherService::WeatherService() 
{
}

WeatherService::~WeatherService()
{
}

WeatherData WeatherService::GetData()
{
  WeatherData wdBad;

  try
  { 
    std::string requestSite("api.openweathermap.org");
    std::string requestQuery("/data/2.5/weather?zip=92399,us&APPID=e7d28320794a5994c8e10181cee49918");

    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);

    tcp::resolver::query query(requestSite.c_str(), "http");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  
    tcp::socket socket(io_service);
    boost::asio::connect(socket, endpoint_iterator);

    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << requestQuery.c_str() << " HTTP/1.0\r\n";
    request_stream << "Host: " << requestSite << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    boost::asio::write(socket, request);
  
    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n");
  
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);

    if (!response_stream || http_version.substr(0,5) != "HTTP/")
      return wdBad;

    if (status_code != 200)
      return wdBad;

    boost::asio::read_until(socket, response, "\r\n\r\n");

    // Process response header (unneeded in this case) 
    std::string header;
    while (std::getline(response_stream, header) && header != "\r");
 
    std::ostringstream ss;
  
    if (response.size() > 0)
      ss << &response;

    boost::system::error_code error;
    while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1),error))
      ss << &response;

    std::string err;
    json11::Json jo = json11::Json::parse(ss.str(), err);

    if (!err.empty())
      return wdBad;

    double temp = jo["main"]["temp"].number_value();
    double maxTemp = jo["main"]["temp_max"].number_value();
    double humidity = jo["main"]["humidity"].number_value();
    double pressure = jo["main"]["pressure"].number_value();
    double windSpeed = jo["wind"]["speed"].number_value();
    std::string status = jo["weather"][0]["main"].string_value();

    std::cout << "Current temperature = " << temp << std::endl;

    WeatherData wdGood(temp, maxTemp, humidity, pressure, windSpeed, status.c_str());
    return wdGood;
  }
  catch(...)
  {
  }

  return wdBad;
}

size_t WeatherService::write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  if (!userdata)
    return 0;

  WeatherService* pService = (WeatherService*)userdata;
  return pService->OnGetDataFromServer(ptr, size, nmemb);
}

size_t WeatherService::OnGetDataFromServer(char* ptr, size_t size, size_t nmemb)
{
  m_result = std::string(ptr, size * nmemb);

  return size * nmemb;
}

