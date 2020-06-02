#define STM32F051x8
#define F_OSC    8'000'000UL
#define F_CPU   48'000'000UL

#include "init_clock.h"
#include "periph_rcc.h"
#include "periph_flash.h"
#include "pin.h"
#include "literals.h"
#include "button.h"
#include "flash.h"
#include "work_count.h"
#include "pwr_control.h"

using Reset      = mcu::PC14;
using Led_work   = mcu::PC13;
using Led_red    = mcu::PB12;
using Led_yellow = mcu::PB13;

extern "C" void init_clock () { init_clock<F_OSC, F_CPU>(); }

int main()
{
   constexpr auto warning_hours {7'500};
   constexpr auto finish_hours  {8'000};
   
   struct Flash_data {
      
      uint16_t hours = 0;
      uint16_t minutes = 0; 
      
   }flash;
   
   [[maybe_unused]] auto _ = Flash_updater<
        mcu::FLASH::Sector::_26
      , mcu::FLASH::Sector::_25
   >::make (&flash);

   volatile decltype (auto) led_red    = Pin::make<Led_red,    mcu::PinMode::Output>();
   volatile decltype (auto) led_yellow = Pin::make<Led_yellow, mcu::PinMode::Output>();
   volatile decltype (auto) led_work   = Pin::make<Led_work,   mcu::PinMode::Output>();

   // подсчёт часов работы
   [[maybe_unused]] auto work_count = Work_count{flash};

   volatile decltype(auto) pwr_control = Pwr_control::make<mcu::PWR::Threshold::_2_9V>();
   pwr_control.set_callback([&]{
      flash.minutes = work_count.get_minutes();
   });

   Timer timer_reset;
   bool pause_reset{false};

   auto reset = Button<Reset>();
   reset.set_long_push_callback([&]{work_count.reset(); timer_reset.start(3_s); led_work = true; pause_reset = true;});

   Timer timer{1_s};

   while(1){
      
      // if (timer_reset.done()) {
      //    timer_reset.stop();
      //    pause_reset = false;
      //    led_work = false;
      // }

      led_work ^= (timer.event() & not pause_reset);

      // led_work ^= timer.event();

      if (flash.hours >= warning_hours and flash.hours < finish_hours) {
         led_yellow = true;
         led_red = false;
      } else if (flash.hours >= finish_hours) {
         led_yellow = false;
         led_red = true;
      } else {
         led_yellow = false;
         led_red = false;
      }

      __WFI();
   }
}
