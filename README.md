# clock-ctrl
Code to control a clock and its CRT display.  Inspired by the David Forbes CRT clock.  The general display architecture uses Lissajou curves to draw graphics on the CRT.  For a good overview, see http://oscilloclock.com/archives/333.

The clock by David Forbes (and subsequent versions by Aaron Stokes) were done on a 8 bit microprocessor plus discreet logic.  I decided to create my own version, and used a Cypress PSOC chip.  The generation of the required waveforms is done on the programmable logic fabric of the PSOC.  The generation of the blanking waveform (used to selectively blank each 45 degree segement of the ellipses) is also generated in the PSOC fabric, and aligned in time with the sinuoids.  The additional hardware on the controller board is mostly analog, except for the DS3231 real-time clock and the MAX509 multiplying DAC.

Because I chose to program in C, and because my hardware architecture differs from the original Forbes clock at the lowest levels, I have written 100% of the code from scratch, but I use the font design from the open source Forbes/Stokes project.  I also use a piece of Verilog code that instantiates a sin lookup table in the PSOC programmable logic fabric.  This was contributed by a member of the Cypress PSOC Developer Forum.

To be continued.
