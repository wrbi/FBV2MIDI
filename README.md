# FBV2MIDI
Use Line6 FBV to control different amps with the help of an Arduino Board.

Update!!!
now i'm able to control my VOX with the FBV!!!


==========================================================

Both units have a proprietary data protocol.
I alrady analysed the Vox some time ago. 
Now i've analysed the Line6 FBV. 
At the moment i can interpret each key an pedal informatin from the FBV.
Also i can set the status of each LED, and send Text to the display

I coded a class Line6Fbv you can use in an Aruino Project which reponds to the Switches an Pedals and switches LEDs.
Together with the Arduino MIDI Library and a little coding it is possible to Control any Unit that responds to MIDI.






