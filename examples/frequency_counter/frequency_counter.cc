// Copyright 2011 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <avr/io.h>
#include <stdio.h>

#include "avrlibx/devices/hd44780_lcd.h"
#include "avrlibx/devices/buffered_display.h"
#include "avrlibx/io/gpio.h"
#include "avrlibx/io/parallel_io.h"
#include "avrlibx/system/init.h"
#include "avrlibx/system/timer.h"

using namespace avrlibx;

typedef Hd44780Lcd<
    Gpio<PortC, 2>,
    Gpio<PortC, 3>,
    ParallelPort<PortC, 4, 7>,
    16, 2> Lcd;

typedef Timer<PortD, 1> TuningTimer;
typedef InputCapture<PortD, 4> TuningCaptureChannel;

BufferedDisplay<Lcd> display;
Lcd lcd;
PWM<PortC, 1> contrast;
Timer<PortC, 0> lcd_refresh_timer;
TuningTimer tuning_timer;
TuningCaptureChannel tuning_capture_channel;

// 4.9 kHz
static uint8_t cycle = 0;
ISR(TCC0_OVF_vect, ISR_NOBLOCK) {
  if (!(cycle & 0x07)) {
    lcd.Tick();
  }
  ++cycle;
}

volatile uint32_t interval = 1;
volatile uint8_t num_overflows = 0;

ISR(TCD1_CCA_vect) {
  uint16_t current_time = TuningCaptureChannel::get_value();
  interval = current_time + \
      (static_cast<uint32_t>(num_overflows) << 16);
  num_overflows = 0;
}

ISR(TCD1_OVF_vect, ISR_NOBLOCK) {
  ++num_overflows;
}

int main(void) {
  SysInit();
  
  Gpio<PortE, 0>::set_direction(OUTPUT);
  
  lcd.Init();
  display.Init();
  
  lcd_refresh_timer.set_prescaler(TIMER_PRESCALER_CLK);
  lcd_refresh_timer.set_period(6528);
  lcd_refresh_timer.set_mode(TIMER_MODE_SINGLE_PWM);
  lcd_refresh_timer.set_pwm_resolution(8);
  lcd_refresh_timer.EnableOverflowInterrupt(1);
  
  tuning_timer.set_prescaler(TIMER_PRESCALER_CLK_8);
  tuning_timer.set_period(0xffff);
  tuning_timer.set_mode(TIMER_MODE_NORMAL);
  tuning_capture_channel.Init<1>(true);
  tuning_timer.EnableOverflowInterrupt(1);
  tuning_capture_channel.EnableInterrupt(1);
  
  contrast.Start();
  contrast.set_value(80);
  
  display.Init();
  display.Print(0, "frequency:");
  char buffer[16];
  float smoothed_frequency = 0.0;
  while (1) {
    display.Tick();
    if (interval) {
      float frequency = (F_CPU / 8) / static_cast<float>(interval);
      smoothed_frequency = 0.95 * smoothed_frequency + 0.05 * frequency;
      memset(buffer, ' ', sizeof(buffer));
      sprintf(buffer, "%.3f", smoothed_frequency);
      display.Print(1, buffer);
    }
  }
}
