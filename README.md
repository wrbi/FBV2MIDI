UPDATE 08.10.2015

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






