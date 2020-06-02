#pragma once

#include "timers.h"

namespace {
    constexpr auto update_minutes        {60'000};
    constexpr auto update_every_remember {60};
}


// считает время работы лампы в минутах
template<class Flash_data>
struct Work_count : TickSubscriber {
    
    Flash_data& flash;
    int tick_cnt {0};
    uint16_t minutes;

    Work_count (Flash_data& flash) : flash{flash} {tick_subscribe(); minutes = flash.minutes;}

    uint16_t get_minutes() {return minutes;}

    void notify() override {
        if (++tick_cnt == update_minutes) {
            tick_cnt = 0;
            minutes++;
            if (minutes == update_every_remember) {
                minutes = 0;
                flash.hours++;
            }
        }
    }

    void reset () {
        minutes = 0;
        flash.minutes = 0;
        flash.hours = 0;
    }
    
};