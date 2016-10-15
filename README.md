Update 15.10.2016 
=================
new source file FBV2KPA_V4_02.ino added

- scroll performances whenn holding UP or DOWN

- solved a bug with display flashing

==> updated Line6Fbv library needed


Update 10.10.2016
=================
Version 1 for Kemper OS 4.x

fixes a bug when trying to switch to slot number 5 in higer performance numbers 

also switches on an LED at Pin 13 just to see Arduino is switched on


UPDATE 21.11.2015
=================

New features for the Kemper:

in looper mode, digit 4 shows the Status:

L = Looper mode, but empty

R = Record

P = Play

O = Overdub

S = Stopped

T = Triggered




Tuning information is shown in the display, either when you select Tuner with the Chickehead or if you set the Volume pedal tu zero.


in Browser Mode: the rig name is displayed, rigs can be selected with Bank up/down (like rig left, right)


in Perfomance mode:
Performance slot names are displayed

With bank up/down the performance preview mode is selected and nhe new performance name is shown.
it switches back after a few seconds if you don't switch to a channel, as the KPA does.


for proper operation start teh KPA with performance mode and seitch to another Slot. after that you can use the Pedal.






UPDATE 08.10.2015
=================

since today i am able to use the FBV with my new Kemper Profiling Amplifier (FBV2KPA.ino)

A brief overview about the current functionallity:


Switches:

FX Loop + Stomp 1-3 control FX Slots A-D.

Reverb, Delay and Modulation control the correponding FX slots of the KPA

PITCH controls FX Slot "X"

Channel A-D + FAVORITE(=E) switch between the Prformance slots.

Bank up/down select performances, but does not call a new program

Channel Switches call the program in the selected Performance.



Holding down the active channel switch for 2 seconds activates the looper ("L" in the display behiund the Performance Number).

The middle row of the switches has now a different function.

AMP1: LOOP_REC_PLAY

AMP2: LOOP_STOP_ERASE

REVERB: KPA_LOOP_TRIGGER
PITCH:LOOP_REVERSE

MOD: LOOP_HALFTIME

DELAY: LOOP_UNDO



Holding down the actice Channel again: Switches have their normal function again ("L" disappears)


Pedals:

Left => Wah

Right => Vol



Use the latest version of the Line6Fbv class 

PLEASE contribute and let us get the rest out of the KPA




=====================================================================================
=====================================================================================
=====================================================================================
=====================================================================================
=====================================================================================

# FBV2MIDI
Use Line6 FBV to control different amps with the help of an Arduino Board.



==========================================================
first i want to control my VOX AD60VT
===> it works now

==========================================================

Both units have a proprietary data protocol.
I alrady analysed the Vox some time ago. 
Now i've analysed the Line6 FBV. 
At the moment i can interpret each key an pedal informatin from the FBV.
Also i can set the status of each LED, and send Text to the display

I coded a class Line6Fbv you can use in an Aruino Project which reponds to the Switches an Pedals and switches LEDs.
Together with the Arduino MIDI Library and a little coding it is possible to Control any Unit that responds to MIDI.






