#include "button.h"

void button::begin()
{
    pinMode(pin, INPUT);
}
bool button::is_clicked()
{
    return !is_pressed_before && is_pressed;
}
bool button::is_long_pressed()
{
    bool value = !is_long_pressed_before && is_pressed && is_pressed_before && millis() - press_time_ms >= long_press_ms;
    if (value == true)
        is_long_pressed_before = true;
    return value;
}
bool button::is_double_clicked()
{
    return is_clicked() && millis() - release_time_ms <= double_click_time_ms;
}
void button::tick()
{
    is_pressed_before = is_pressed;
    is_pressed = get_current_state();
    if (!is_pressed_before && is_pressed)
    {
        press_time_ms = millis();
        if (click_handler)
            click_handler();
    }
    else if (is_pressed_before && !is_pressed)
    {
        is_long_pressed_before = false;
        release_time_ms = millis();
    }
    if (is_long_pressed() && long_press_handler)
    {
        long_press_handler();
    }
    else if (is_double_clicked() && double_click_handler)
    {
        double_click_handler();
    }
    else if (is_clicked() && click_handler)
    {
        click_handler();
    }
}