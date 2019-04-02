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
#include <avr/pgmspace.h>
#include <stdio.h>

#include "avrlibx/system/init.h"
#include "avrlibx/system/timer.h"
#include "avrlibx/utils/op.h"
#include "avrlibx/system/time.h"

using namespace avrlibx;

typedef Gpio<PortD, 0> DcoOut;
typedef Timer<PortD, 0> DcoTimer;
typedef PWM<PortD, 0> Dco;

DcoTimer dco_timer;
Dco dco;
DcoOut dco_out;

static const uint16_t kOctave = 12 << 7;
static const uint16_t kFirstNote = 24 << 7;

const prog_uint16_t lut_res_dco_pitch[] PROGMEM = {
   61156,  60716,  60279,  59846,  59415,  58988,  58563,  58142,
   57724,  57308,  56896,  56487,  56080,  55677,  55276,  54879,
   54484,  54092,  53703,  53316,  52933,  52552,  52174,  51799,
   51426,  51056,  50689,  50324,  49962,  49603,  49246,  48891,
   48540,  48190,  47844,  47500,  47158,  46819,  46482,  46147,
   45815,  45486,  45158,  44834,  44511,  44191,  43873,  43557,
   43244,  42933,  42624,  42317,  42013,  41711,  41410,  41113,
   40817,  40523,  40232,  39942,  39655,  39370,  39086,  38805,
   38526,  38249,  37974,  37700,  37429,  37160,  36893,  36627,
   36364,  36102,  35842,  35584,  35328,  35074,  34822,  34571,
   34323,  34076,  33831,  33587,  33346,  33106,  32868,  32631,
   32396,  32163,  31932,  31702,  31474,  31248,  31023,  30800,
   30578,
};

uint16_t SetNote(int16_t note) {
  uint8_t overflows = 0;
  // Lowest note: E0.
  note -= kFirstNote;
  // Transpose the lowest octave up.
  while (note < 0) {
    note += kOctave;
    ++overflows;
  }
  uint8_t shifts = 0;
  while (note >= kOctave) {
    note -= kOctave;
    ++shifts;
  }
  uint16_t index_integral = U16ShiftRight4(note);
  uint16_t index_fractional = U8U8Mul(note & 0xf, 16);
  uint16_t count = pgm_read_word(lut_res_dco_pitch + index_integral);
  uint16_t next = pgm_read_word(lut_res_dco_pitch + index_integral + 1);
  count -= U16U8MulShift8(count - next, index_fractional);
  if (overflows) {
    shifts += 3 - overflows;
  }
  while (shifts--) {
    count >>= 1;
  }
  dco.Write(count);
  dco_timer.set_prescaler(
      overflows
      ? TIMER_PRESCALER_CLK_64
      : TIMER_PRESCALER_CLK_8);
}

int main(void) {
  SysInit();
  
  dco_timer.set_mode(TIMER_MODE_FREQUENCY_GENERATOR);
  dco.Start();
  uint16_t note = 0;
  while (1) {
    SetNote(note);
    note = (note + 4) & 0x3fff;
    ConstantDelay(1);
  }
}
