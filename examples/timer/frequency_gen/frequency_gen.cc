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

#include "avrlibx/io/gpio.h"
#include "avrlibx/system/init.h"
#include "avrlibx/system/timer.h"
#include "avrlibx/system/time.h"

using namespace avrlibx;

// Frequency sweep
Timer<PortF, 0> timer;
PWM<PortF, 0> pwm_a;

int main(void) {
  SysInit();
  timer.set_prescaler(TIMER_PRESCALER_CLK);
  timer.set_mode(TIMER_MODE_FREQUENCY_GENERATOR);

  pwm_a.Start();
  uint16_t i = 0;
  while (1) {
    pwm_a.Write(i);
    ConstantDelay(1);
    i += 10;
  }
}
