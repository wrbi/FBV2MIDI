Update 2015.11.21 
=================
added a completely new version.

in this versions i use code pieces i got from several users.
Thanx a lot.

I copied the code snippets together and edited until it worked.
The code is not nice and has to be refactored,
but it works for now.
Maybe i will do in a few months.


Update 2015.10.17 
=================
FBV2KPA: "Solo" Function added

When playing a Solo, you might want to turn on Reverb, Delay and Boost at once.

The AMP2 Switch on the FBV is used to turn on all available Effects in the Post-FX Section at once.
Pressing again, it turns off the effects which are initial off in the Performance Slot.

=================================================================================================================

Class KPA is used for sending and receicing commands between the Kemper Profiling Amplifier and the arduino Board


it is used in my Arduino file FBV2KPA

it is nothing else but sending and receiving midi commands.
I wrote it, because my KPA dumped when using the Arduino MIDI library.