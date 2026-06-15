#ifndef OPENWEATHER_CLINT_H
#define OPENWEATHER_CLINT_H

#include <HTTPClient.h>
#include "secrets.h"

#define OPENWEATHER_REQUEST_TEMPLATE "http://api.openweathermap.org/data/2.5/weather?lat=%f&lon=%f&appid=%s"
#define KELVIN_ZERO 273.15

namespace openweather
{
    enum class weather_type
    {
        Thunderstorm = 200,
        Drizzle = 300,
        Rain = 500,
        Snow = 600,
        Fog = 700,
        Clear = 800,
        Clouds = 801
    };
    enum class result
    {
        Ok,
        Error
    };
    struct forecast
    {
        float temp;
        float temp_feels_like;
        float humidity;
        int clouds;
        float wind_speed;
        weather_type weather;
    };
    class client
    {
    private:
        HTTPClient *_http_client;
        float _latitude, _longitude;
        result _makeRequest();
        result _parseRequest(forecast *forecast);

    public:
        client(HTTPClient *http_client, float latitude, float longitude) : _http_client(http_client), _latitude(latitude), _longitude(longitude)
        {
        }
        ~client()
        {
        }
        result get_weather(forecast *forecast);
    };
}

#endif