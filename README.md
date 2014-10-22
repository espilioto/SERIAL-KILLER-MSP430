SERIAL-KILLER-MSP430
====================

This is the MSP430 C code for the Serial Killer (TI CCS5 project)


This is a simple parser for the MSP430. 
The MSP430 receives commands through the UART, parses them, and sets the PWM values for the RGB led/strip or whatever. 

At first this was meant to be a room mood lighting - led strip..thing. 
Then I dropped that for no reason and decided to install it in my pc case.

I'll update this and link some gerbers when I finish designing the pcb.

Here is the link for the GUI project: https://github.com/espilioto/SERIAL-KILLER-beta

Many thanks to the 43oh forum for the help (even though I didn't even start a new thread) :P It's a very nice place if you're into this stuff.
Threads that helped me develop this: 

http://forum.43oh.com/topic/253-pwm-codes-and-timera/ PWM stuff 
http://forum.43oh.com/topic/1643-g2553-hardware-uart-hello-world-example/ Serial stuff 
http://forum.43oh.com/topic/1764-msp430g2553launchpad-uart-rx-example/ More serial stuff

http://www.msp430launchpad.com/2010/12/njcs-launchscope-launchpad-oscilloscope.html 
This project gave me the idea to make this.

Many thanks to all of you!

tl;dr: A UART led blinker for the MSP430

SouLSLayeR
