#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>
#include "time.h"
#include <Adafruit_BMP280.h>
#include "captive_wifi_manager.h"
#include "openweather_client.h"
#include "button.h"
#include "bitmaps.h"
#include "FreeSansBold24pt7b.h"
#include "Petme8x8.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define BMP280_I2C_ADDRESS 0x76
#define OPENWEATHER_REQUEST_PERIOD_MS 3600000UL
#define NTP_REQUEST_PERIOD_MS 1800000UL
#define SENS_BTN_GPIO 3
const char *ntp_server = "pool.ntp.org";
const long gmt_offset_sec = 10800;
const int daylight_offset_sec = 0;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BMP280 temp_sensor;
RoboEyes<Adafruit_SSD1306> robo_eyes(display);
extern const char *connect_page_html;
captive_wifi::manager wifi_manager("tinyRobot", connect_page_html);
HTTPClient http_client;
openweather::client weather_client(&http_client, 53.9, 27.56);
button btn(SENS_BTN_GPIO, 1);
void get_datetime(char *date_buf, size_t date_size, char *time_buf, size_t time_size)
{
  tm time_info;
  getLocalTime(&time_info);
  strftime(date_buf, date_size, "%d %B", &time_info);
  strftime(time_buf, time_size, "%02H:%02M", &time_info);
}
void draw_text_center(int16_t xcenter, int16_t ycenter, const char *text,
                      int16_t *xcorner = nullptr, int16_t *ycorner = nullptr, uint16_t *w = nullptr, uint16_t *h = nullptr)
{
  display.setTextWrap(false);
  int16_t x1, y1;
  uint16_t _w, _h;
  display.getTextBounds(text, xcenter, ycenter, &x1, &y1, &_w, &_h);
  xcenter = xcenter - _w / 2;
  ycenter = ycenter - _h / 2;
  display.setCursor(xcenter, ycenter);
  display.print(text);
  if (xcorner != nullptr)
    *xcorner = xcenter;
  if (ycorner != nullptr)
    *ycorner = ycenter;
  if (w != nullptr)
    *w = _w;
  if (w != nullptr)
    *h = _h;
}
void draw_screen_1(void)
{
  char time_buf[6];
  char date_buf[32];
  static uint32_t ntp_request_timer = 0;
  if (ntp_request_timer == 0 || millis() - ntp_request_timer >= NTP_REQUEST_PERIOD_MS)
  {
    display.clearDisplay();
    display.setFont(NULL);
    display.setCursor(0, 0);
    display.setTextWrap(true);
    display.print("Obtaining current time...");
    display.display();
    configTime(gmt_offset_sec, daylight_offset_sec, ntp_server);
    ntp_request_timer = millis();
  }
  display.clearDisplay();
  display.setTextColor(1);
  display.setFont(&FreeSansBold24pt7b);
  display.setCursor(6, 48);
  get_datetime(date_buf, 32, time_buf, 6);
  display.print(time_buf);
  // display.drawBitmap(106, 2, image_battery_bits, 19, 9, 1);
  display.setFont(&Petme8x8);
  draw_text_center(64, 65, date_buf);
  display.display();
}
void draw_screen_2(void)
{
  robo_eyes.setPosition(DEFAULT);
  robo_eyes.setAutoblinker(true, 2, 3);
  robo_eyes.setIdleMode(ON, 2, 2);
}
void draw_screen_3(void)
{
  static openweather::forecast forecast = {0, 0, 0, 0, 0, openweather::weather_type::Clear};
  static uint32_t openweather_request_timer = 0;
  static bool is_request_success = false;
  display.clearDisplay();
  if (openweather_request_timer == 0 || !is_request_success || millis() - openweather_request_timer >= OPENWEATHER_REQUEST_PERIOD_MS)
  {
    openweather_request_timer = millis();
    display.setFont(NULL);
    display.setCursor(0, 0);
    display.setTextWrap(true);
    display.print("Obtaining weather forecast...");
    display.display();
    display.clearDisplay();
    openweather::result res = weather_client.get_weather(&forecast);
    is_request_success = res == openweather::result::Ok;
  }
  if (!is_request_success)
  {
    display.setFont(NULL);
    display.setTextColor(1);
    display.setCursor(0, 0);
    display.println("Failed to get forecast!");
    display.display();
    return;
  }
  switch (forecast.weather)
  {
  case openweather::weather_type::Rain:
    display.drawBitmap(4, 11, image_rain_bits, 40, 40, 1);
    break;
  case openweather::weather_type::Snow:
    display.drawBitmap(4, 11, image_snow_bits, 40, 42, 1);
    break;
  case openweather::weather_type::Thunderstorm:
    display.drawBitmap(4, 11, image_thunder_bits, 40, 42, 1);
    break;
  case openweather::weather_type::Fog:
    display.drawBitmap(4, 11, image_fog_bits, 40, 40, 1);
    break;
  case openweather::weather_type::Clear:
    display.drawBitmap(4, 11, image_clear_bits, 43, 44, 1);
    break;
  case openweather::weather_type::Clouds:
    display.drawBitmap(4, 11, image_cloud_bits, 43, 32, 1);
    break;
  }
  char temperature_string[16], cloudiness_string[16];
  sprintf(temperature_string, ":%.0f C", std::round(forecast.temp_feels_like));
  sprintf(cloudiness_string, ":%d%%", forecast.clouds);
  // рисуем температуру
  display.drawBitmap(56, 18, image_temperature_bits, 10, 20, 1);
  display.setTextWrap(false);
  display.setFont(&Petme8x8);
  display.setCursor(68, 32);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(temperature_string, 68, 26, &x1, &y1, &w, &h);
  display.print(temperature_string);
  display.drawCircle(68 + w - 11, 25, 2, 1);
  // рисуем облачность
  display.drawBitmap(50, 43, image_cloud_small_bits, 22, 16, 1);
  display.setCursor(72, 55);
  display.print(cloudiness_string);
  // рисуем надпись forecast
  display.setCursor(32, 10);
  display.print("Forecast");
  display.display();
}
void draw_screen_4(void)
{
  char temperature_string[16], pressure_string[16];
  float temp = temp_sensor.readTemperature();
  float pressure = temp_sensor.readPressure();
  sprintf(temperature_string, "%.1f C", temp);
  sprintf(pressure_string, "%.0fPa", pressure);
  display.clearDisplay();
  display.setFont(&Petme8x8);
  display.setCursor(40, 10);
  display.print("Sensor");
  // рисуем температуру
  int16_t x0, y0;
  uint16_t w, h;
  display.drawBitmap(19, 15, image_temperature_big_bits, 16, 32, 1);
  draw_text_center(28, 63, temperature_string, &x0, &y0, &w, &h);
  display.drawCircle(x0 + w - 11, y0 - 8, 2, 1);
  display.drawBitmap(80, 15, image_pressure_bits, 32, 32, 1);
  draw_text_center(96, 63, pressure_string);
  display.display();
}
void wifi_manager_connection_attempt_handler(const char *ssid)
{
  display.clearDisplay();
  display.setFont(nullptr);
  display.setCursor(0, 0);
  display.print("Connecting to: ");
  display.print(ssid);
  display.display();
}
void wifi_manager_connection_result_handler(bool connected)
{
  display.clearDisplay();
  display.setFont(nullptr);
  display.setCursor(0, 0);
  if (connected)
  {
    display.print("Connected successfully!");
  }
  else
  {
    display.print("Failed to connect!");
  }
  display.display();
}
void wifi_manager_ap_created_handler(const char *ssid, const char *ip)
{
  display.clearDisplay();
  display.setFont(nullptr);
  display.setCursor(0, 0);
  display.print("SSID: ");
  display.println(ssid);
  display.print("IP: ");
  display.println(ip);
  display.display();
}
void drawing_routine(int current_screen)
{
  static uint32_t drawing_timer = 0;
  static int previous_screen = -1;
  if (current_screen != 1 && current_screen == previous_screen && millis() - drawing_timer < 5000UL)
  {
    return;
  }
  switch (current_screen)
  {
  case 0:
    draw_screen_1();
    break;
  case 1:
    draw_screen_2();
    robo_eyes.update();
    break;
  case 2:
    draw_screen_3();
    break;
  case 3:
    draw_screen_4();
    break;
  }
  drawing_timer = millis();
  previous_screen = current_screen;
}
void setup()
{
  Serial.begin(115200);
  // Инициализация дисплея
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  if (!temp_sensor.begin(BMP280_I2C_ADDRESS))
  {
    Serial.println("BMP280 initalization failed");
    while (1)
      ;
  }
  temp_sensor.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                          Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                          Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                          Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                          Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  robo_eyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 60);
  display.clearDisplay();
  display.setTextColor(1);
  display.println("Starting AP...");
  display.display();
  wifi_manager.add_connection_attempt_handler(wifi_manager_connection_attempt_handler);
  wifi_manager.add_connection_result_handler(wifi_manager_connection_result_handler);
  wifi_manager.add_ap_created_handler(wifi_manager_ap_created_handler);
  wifi_manager.begin();
  btn.set_long_press_time(5000);
  btn.set_long_press_handler([]()
                             { wifi_manager.reset(); });
  btn.begin();
}
void loop()
{
  static int current_screen = 0;
  static bool btn_pressed_before = false;
  btn.tick();
  if (!wifi_manager.isConnected())
  {
    wifi_manager.handle();
    delay(5);
    return;
  }
  if (btn.is_clicked())
  {
    btn_pressed_before = true;
    current_screen = (current_screen + 1) % 4;
  }
  else
  {
    btn_pressed_before = static_cast<bool>(digitalRead(SENS_BTN_GPIO));
  }
  drawing_routine(current_screen);
}