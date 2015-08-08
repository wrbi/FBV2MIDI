/* 
Demo Programm to test the Line6Fbv library 
with an Arduino Mega Board 

The program turns on the Display light and responds to all 
switches except Pedal Switches.
If a key is pressed, the corresponding LED is lit.
On release of the Switch, the LED is turend off again.

all events are written to Serial Monitor.

 */

#include "Line6Fbv.h"

Line6Fbv myFbv = Line6Fbv();

void onKeyPressed( byte inKey) {
  myFbv.setLed( inKey, 0x01);
  Serial.print("KeyPressed ");
  Serial.println(inKey,HEX);
}

void onKeyReleased( byte inKey) {
  myFbv.setLed( inKey, 0x00);
  Serial.print("KeyReleased ");
  Serial.println(inKey,HEX);
}

void onCtrlChanged( byte inCtrl, byte inValue) {
  if (inCtrl == LINE6FBV_PDL1)
    Serial.print("Pedal 1 ");
  else  
    Serial.print("Pedal 2 ");
  Serial.println(inValue,HEX);
}



void onHeartbeat( ) {

  Serial.println("Heartbeat");
}


void setup() {

  Serial.begin(38400);
  // use Serial1 Port on Arduino Mega
  myFbv.begin(&Serial1);
  myFbv.setHandleKeyPressed( &onKeyPressed );
  myFbv.setHandleKeyReleased( &onKeyReleased );
  myFbv.setHandleHeartbeat( &onHeartbeat );
  myFbv.setHandleCtrlChanged( &onCtrlChanged );
  myFbv.setLed( LINE6FBV_DISPLAY, 0x01);
    Serial.println("ready");

}

void loop() {
  // put your main code here, to run repeatedly:
  myFbv.read();

}
