Update 29.10.2016 
=================

 New in 4.4:
  Scanning Rig Tag "Comment" for pedal assignment
    Tags in Comment:
      P<pdlNum><request>=<value>
         <pdlNum>:
           1 or 2
         <request>:
           A = activeControl
           I = inactiveControl (activated via switch)
               <value>:
                 _ =  none
                 W = Wah
                 V = Vol
                 P = Pitch
                 M = Morph
                 G = Gain
           P = Pedal Position of the active contoller
               <value>:
                 H = Heel  (1 not 0)
                 T = Toe   (127)
                 1 - 9 = Volume Steps of 12
   If Volume is not within the active controllers, a volume of 127 is sent.

   Example:   Rig Comment "P1A=M;P1I=P;P2I=W"
              ==> Pedal 1 is Morph
              ==> Pedal 1 after switching is Pitch  
              ==> Pedal 2 stays deafault Volume as it is not in the comment as (P2A=<>) 
              ==> Pedal 2 after switching is Wah   



   Pedal LEDs show Controller Type
   |       |Green  |Red    |
   +++++++++++++++++++++++++
   | none  | off   | off   |
   | Wah   | off   | flash |
   | Gain  | off   | on    |
   | Vol   | on    | off   |
   | Pitch | flash | off   |
   | Morph | flash | flash |


* Known issue:
  controller Number for Pedal light for Pedal 1 has to be analyzed again


Update 17.10.2016 
=================
new source file FBV2KPA_V4_03.ino added

- Bug fixed when switching the same slot in the next performance


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



Update 21.11.2015
=================
added a completely new version.

in this versions i use code pieces i got from several users.
Thanx a lot.

I copied the code snippets together and edited until it worked.
The code is not nice and has to be refactored,
but it works for now.
Maybe i will do in a few months.


Update 17.10.2015 
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