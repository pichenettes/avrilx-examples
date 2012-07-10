// Copyright 2011 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
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

#include "avrlibx/devices/rotary_encoder.h"
#include "avrlibx/io/gpio.h"
#include "avrlibx/system/init.h"
#include "avrlibx/system/time.h"

using namespace avrlibx;

Gpio<PortF, 0> led_a;
Gpio<PortF, 1> led_b;

RotaryEncoder<Gpio<PortD, 0>, Gpio<PortD, 1>, Gpio<PortD, 2> > encoder;

static uint8_t decay = 0;

ISR(TCF0_OVF_vect) {
  int8_t v = encoder.Read();
  if (v == 1) {
    led_a.High();
    decay = 100;
  } else if (v == -1) {
    led_b.High();
    decay = 100;
  }
  if (decay) {
    --decay;
    if (decay == 0) {
      led_a.Low();
      led_b.Low();
    }
  }
}

int main(void) {
  SysInit();

  led_a.set_direction(OUTPUT);
  led_b.set_direction(OUTPUT);
  encoder.Init();
  
  SetupTickTimer();
  
  while (1) {
  }
}
