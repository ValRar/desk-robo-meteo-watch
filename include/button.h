#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include <functional>

class button
{
private:
    int pin;
    bool active_state;
    bool is_pressed;
    uint32_t long_press_ms;
    uint32_t double_click_time_ms;
    uint32_t press_time_ms;
    uint32_t release_time_ms;
    bool is_pressed_before;
    bool is_long_pressed_before;
    std::function<void(void)> click_handler;
    std::function<void(void)> long_press_handler;
    std::function<void(void)> double_click_handler;
public:
    button(int pin, bool active_state)
        : pin(pin), active_state(active_state),is_pressed(false), long_press_ms(2000UL), double_click_time_ms(400UL), press_time_ms(0), 
        release_time_ms(0), is_pressed_before(false), is_long_pressed_before(false), click_handler(nullptr), 
        long_press_handler(nullptr), double_click_handler(nullptr) {}
    ~button() {}
    void begin();
    bool is_clicked();
    bool is_long_pressed();
    bool is_double_clicked();
    bool get_current_state()
    { // 1 - pressed
        return static_cast<bool>(digitalRead(pin)) == active_state;
    }
    void set_long_press_time(uint32_t ms)
    {
        long_press_ms = ms;
    }
    void set_double_click_time(uint32_t ms) {
        double_click_time_ms = ms;
    }
    uint32_t get_long_press_time()
    {
        return long_press_ms;
    }
    void set_click_handler(std::function<void(void)> handler) {
        click_handler = handler;
    }
    void set_long_press_handler(std::function<void(void)> handler) {
        long_press_handler = handler;
    }
    void set_double_click_handler(std::function<void(void)> handler) {
        double_click_handler = handler;
    }
    void tick();
};
#endif
