// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "wavetables.h"

// Wavetable names - the names have no particular significance outside of describing what
// the wavetable actually is.
#define WAVETABLE_SINE        0
#define WAVETABLE_TRIANGLE    1
#define WAVETABLE_SAW         2
#define WAVETABLE_SQUARE      3
#define WAVETABLE_REVSAW       4
#define WAVETABLE_NOISE       5

// Pin definitions
#define CV_INPUT              A1  
#define WAVE_INPUT            A0  
#define PWM_OUT               11  

// Any variables which are used inside the ISR must be declared as volatile - not doing so could
// result in unpredictable behaviour or, more likely, no behaviour at all.
//
// DO NOT use compiler optimisation flags (-O0 etc.)
volatile uint16_t syncPhaseAcc, syncPhaseInc, baseFreq;
volatile uint8_t current_wavetable = 5;

// Wavetable definitions - the two NULL values are placeholders for the square and sawtooth
// waveforms which can be generated based on the current step. To replace these with a wavetable,
// declare the wavetable in wavetables.h and replace the appropriate NULL with a pointer to the
// beginning of the desired table.
//
// Doing this will also require the code within the ISR to be modified as well.
const uint8_t * const wavetables[] PROGMEM = { &sine[0], &triangle[0], NULL, NULL, NULL, NULL };

static inline void initTimer0(void) {
  pinMode(CV_INPUT, INPUT);
  pinMode(WAVE_INPUT,INPUT);
  pinMode(PWM_OUT,OUTPUT);
  TCCR2A = _BV(COM2A1) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  TIMSK2 = _BV(TOIE2);
  syncPhaseAcc = 0;
}

void setup() {
  initTimer0();
}

void loop() {
  uint16_t fAcc=0, tAcc=0;
  for (int i=0; i <= 15; i++) {
    fAcc += analogRead(CV_INPUT);
    tAcc += analogRead(WAVE_INPUT);
  }
  
  baseFreq = mapFreq(fAcc >> 4);
  current_wavetable = mapOsc(tAcc >> 4);
  syncPhaseInc = baseFreq;

}

// The interrupt service routine - this is where it all happens.
// Triggered when TIMER0 overflows.
//
// - Increment the phase accumulator
// - Convert the accumulator value to an 8-bit number corresponding to
//   the step in the wavetable
// - Read the appropriate wavetable value
// - Write it to the PWM pin
//
// In order to prevent overruns and glitching, code within the ISR should do
// as little as it can get away with. In this case, we're reading a value from
// wavetable and writing it out to a PWM pin - square and sawtooth waves are
// generated on the fly based on the current step; obviously this is much quicker.
ISR(TIMER2_OVF_vect) {
   uint8_t val, step;
   syncPhaseAcc += syncPhaseInc;
   step = syncPhaseAcc >> 8;
   
   switch (current_wavetable) {
     case WAVETABLE_SINE:
         val = pgm_read_byte_near((uint8_t *)wavetables[WAVETABLE_SINE] + step);
         break;
     case WAVETABLE_TRIANGLE:
         val = pgm_read_byte_near((uint8_t *)wavetables[WAVETABLE_TRIANGLE] + step);
         break;
     case WAVETABLE_SAW:
         val = step;
         break;
     case WAVETABLE_SQUARE:
         val = (step < 128) ? 0x00 : 0xff;
         break;
     case WAVETABLE_REVSAW:
         val = 255-step;
         break;
     default: val = (step < 32) ? 0xff : 0x00;
              break;
   }

   // Writing the current wavetable value directly to OCR2A is much quicker than using
   // digitalWrite() or its equivalent..
   OCR2A = val;
}
