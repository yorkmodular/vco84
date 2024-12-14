# vco84

A while ago, I developed a microcontroller-based VCO based around the ATTiny85 microcontroller - the resulting module was a wavetable VCO and, generally, did the job well with very few components, although there were obviously some deficiencies; using the internal PLL as a clock source meant that the tracking was definitely a bit iffy at higher frequencies. If you want perfect to-the-cent tracking then you're looking in the wrong place - if you want to make an approximately tuneful noise then read on.

Fast forward to here and now and there's an updated version.

Originally, the updated design used an ATTiny84 but for some reason it had a distressing tendency to run hot. Plus, unless you had the appropriate gear it was a pain in the backside to reflash the firmware, so as I had a bunch of Arduino Nano boards lying around I rejigged the circuit to use those instead - now you can reprogram the firmware over USB. This isn't actually the 84th iteration of the module - it just felt like it sometimes.

Other than that, it's much the same as its predecessor - CV input with a range good for around 5 octaves; using the default firmware, the lowest output frequency is around 32Hz (C1). Waveform is CV-selectable as well and the usual suspects are present: sine, triangle, sawtooth, reverse sawtooth, pulse and 50% duty cycle square - if you're confident about hacking the firmware, you can change these to whatever you like, add your own wavetables or just do something completely different.

There's a certain degree of extensibility as well - although only two analogue (A0, A1) and one PWM (D11) pins are used, there are pads for the remaining analogue pins (A2-A7) and a couple of digital pins (D9, D10 and D12) as well. This means that if you're confident with a soldering iron and want go have a good old hack, you can.

But wait, there's more!

One major addition over the original AVR VCO is that of a PWM output. The microcontroller generates a PWM signal which is then filtered into the sound that you eventually hear. The PWM signal is now broken out so that you can do things with it, for example feed it into a clock divider and then a filter to generate suboctaves of the main waveform.

The firmware is pretty simple - most of the heavy lifting is done by the ISR, with the main loop simply polling the analogue inputs. The compiled binary is around 5k, meaning that if you're running an Atmega328-based Nano there's a load of space available for things like higher resolution wavetables, more wavetables etc. Even if you're using a board with a 168 on board you've still got a good 8-9kb free if you want to hack around.

**NOTE**: if you're using an older Nano board (or perhaps some of the clone boards) you may find it necessary to select "Atmega328P (Old Bootloader)" as the target processor, rather than the default of "Atmega328P". I haven't done any testing with 168-based Nano boards, as I don't currently have any, but there's no technical reason why they shouldn't work.
