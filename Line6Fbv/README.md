Class Line6Fbv is used for sending and receicing commands between the Line6 FBV Longboard and the arduino Board

it works with Hardware Serial Ports of the Arduino

currently supported:

- receiving KeyPressed and KeyReleased messages and new: keyHold message
- receiving Pedal Position Changes
- receiving a heartbeat (every 7 seconds)
- switching LEDs on and of
- switch Display Backlight on and off
- flash LEDs with separate intervals
- write Text to the Display


================================================

Update 15.10.2016

solved a problem with Display flashing
flashing now has to be turned off exlicitly with setDisplayFlash(0,0);

