#include "openweather_client.h"
#include <ArduinoJson.h>

using namespace openweather;

result client::get_weather(forecast* forecast) {
    result res = _makeRequest();
    if (res != result::Ok) return res;
    return _parseRequest(forecast);
}
result client::_makeRequest() {
    char request_buffer[256];
    int size = sprintf(request_buffer, OPENWEATHER_REQUEST_TEMPLATE, _latitude, _longitude, OPENWEATHER_API_KEY);
    if (size > 256) {
        log_e("Openweather client request URL is too big!");
        return result::Error;
    }
    _http_client->useHTTP10(true);
    _http_client->begin(request_buffer);
    log_i("request URL: %s", request_buffer);
    int httpCode = _http_client->GET();
    if (httpCode != 200) {
        log_e("Openweather client http code is %d instead of 200!", httpCode);
        return result::Error;
    }
    return result::Ok;
}
result client::_parseRequest(forecast* forecast) {
    JsonDocument doc;
    WiFiClient stream = _http_client->getStream();
    DeserializationError error = deserializeJson(doc, stream);
    if (error) {
        log_e("Deserialization failed with error: %d, %s", error.code(), error.c_str());
        return result::Error;
    }
    forecast->temp = doc["main"]["temp"].as<float>() - KELVIN_ZERO;
    forecast->temp_feels_like = doc["main"]["feels_like"].as<float>() - KELVIN_ZERO;
    forecast->humidity = doc["main"]["humidity"].as<float>();
    forecast->clouds = doc["clouds"]["all"].as<int>();
    forecast->wind_speed = doc["wind"]["speed"].as<float>();
    int weather = doc["weather"][0]["id"].as<int>();
    if (weather / 100 == 8 && weather != 800) {
        forecast->weather = static_cast<weather_type>(801);
    } else {
        forecast->weather = static_cast<weather_type>(weather / 100 * 100);
    }
    return result::Ok;
}