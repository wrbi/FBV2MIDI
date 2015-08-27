/*
*  @file       line6Fbv2VoxAd60Vt.ino
*  Project     Arduino Line6 FBV Longboard to MIDI Library
*  @brief      Line6 FBV Library for the Arduino
*  @version    0.4
*  @author     Joachim Wrba
*  @date       11/08/15
*  @license    GPL v3.0
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
==========================================================================
Control the VOX AD 60/120 VT(X)
with a Line6 FBV instead of the VC-12
---------------------------------------------------------------------------
No switching between Stomp and Channel Mode necessary
as there are enough Switches :-)
- Amp1, Amp2 + Pdl1 Switches activate the Pedal Stomp Box
- Modulation, Reverb Delay and Tap use the corresponding switches
- FX Loop = Tuner
- Stomp 1 = Tuner silent
- Bank up/down and Channel A-D are used to select the Programs

- as a special feature i made a Wah auto on/off function
if the Effect is off by default, the auto mode is activated.
When the Pedal is moved, the effect is automatically switched on
and after 0,7 seconds of no movement it is turned off again
----------------------------------------------------------------------------
the update of LEDs ans display is programmes quick and dirty in different functions
this should be optimized
---------------------------------------------------------------------------

*/
#include "Line6Fbv.h"
#include "VoxAd60Vt.h"

Line6Fbv mFbv = Line6Fbv();
VoxAd60Vt mVox = VoxAd60Vt();

// actual status of Stomp Boxes
byte mActStatusPdl;
byte mActStatusMod;
byte mActStatusDly;
byte mActStatusRev;

// actual values of Pedals
byte mActValWah;
byte mActValVol;  // sent with every program change

// Program handling
byte mActBank;
byte mNextBank;   // for UP/DOWN events
byte mActChannel; // A B C D
byte mActPgmNum;

// Auto Wah on/off **************************************************
// if the Pedal StompBox is Off in a program,
// the Pedal StompBox will be activated with the movement of Pedal 1.	
byte mWaitForStompInfo = 0;
unsigned long mWahLastMove;
byte mWahAutoOnOff;
const int mWahOnOffInterval = 700; // after 0,7 seconds without movement Wah will be turned off

// Tap has to be converted to milliseconds
byte mTapModeActive = 0;
unsigned long mLastTap;
const int mMaxDelayTime = 2000;

// Tuner
byte mTunerIsOn = 0;
byte mTunerIsSilent = 0;

// switch wah in auto-on-off-mode off after mWahOnOffInterval ( i use 0,7s)
void fWahAutoOff(){
	unsigned long currentMillis;

	if (mActStatusPdl && mWahAutoOnOff){
		currentMillis = millis();
		if (currentMillis - mWahLastMove >= mWahOnOffInterval) {
			mActStatusPdl = 0;
			mVox.switchStompBoxes(mActStatusPdl, mActStatusMod, mActStatusDly, mActStatusRev);
			fSetStompLeds();
			Serial.print("APP Wah Auto Off");
		}
	}
}

// Deactivate TAP Mode after 2s not tapping (2s = max delay time)
void fDeactivateTapMode(){
	unsigned long currentMillis;

	if (mTapModeActive){
		currentMillis = millis();
		if (currentMillis - mLastTap > mMaxDelayTime) {
			mTapModeActive = 0;
			Serial.print("APP Tap Mode Off");
		}
	}
}


// respond to pressed keys on the FBV
void onFbvKeyPressed(byte inKey) {
	byte switchStomp = 0;
	byte switchBank = 0;
	byte switchChannel = 0;
	byte switchTuner = 0;

	Serial.print("FBV KeyPressed ");
	Serial.println(inKey, HEX);

	switch (inKey){
	case LINE6FBV_AMP1:   // Respond to Amp1 Amp2 and Wah Switch to switch on Stomp Pedal
	case LINE6FBV_AMP2:
	case LINE6FBV_PDL1_SW:
		mActStatusPdl = !mActStatusPdl;
		switchStomp = 1;
		break;
	case LINE6FBV_MOD:
		mActStatusMod = !mActStatusMod;
		switchStomp = 1;
		break;
	case LINE6FBV_DELAY:
		mActStatusDly = !mActStatusDly;
		switchStomp = 1;
		break;
	case LINE6FBV_REVERB:
		mActStatusRev = !mActStatusRev;
		switchStomp = 1;
		break;
	case LINE6FBV_UP:
		mFbv.setLedOnOff(LINE6FBV_UP, 1);  // light LED until released
		switchBank = 1;
		break;
	case LINE6FBV_DOWN:
		mFbv.setLedOnOff(LINE6FBV_DOWN, 1); // light LED until released
		switchBank = 1;
		break;
	case  LINE6FBV_CHANNELA:
	case  LINE6FBV_CHANNELB:
	case  LINE6FBV_CHANNELC:
	case  LINE6FBV_CHANNELD:
		switchChannel = 1;
		break;
	case LINE6FBV_TAP:
		fProcessTap();
		break;
	case LINE6FBV_FXLOOP:
	case LINE6FBV_STOMP1:
		fProcessTuner(inKey);
		break;
	case LINE6FBV_STOMP3:
		mWahAutoOnOff = !mWahAutoOnOff;
		mFbv.setLedOnOff(LINE6FBV_STOMP3, mWahAutoOnOff);
		break;
	}

	if (switchStomp){
		mVox.switchStompBoxes(mActStatusPdl, mActStatusMod, mActStatusDly, mActStatusRev);
		fSetStompLeds();
	}
	if (switchBank){
		fSetNewBankValue(inKey);
	}
	if (switchChannel){
		fChangeProgram(inKey);
	}
}

void fSetStompLeds(){
	mFbv.setLedOnOff(LINE6FBV_PDL1_GRN, mActStatusPdl);
	mFbv.setLedOnOff(LINE6FBV_AMP1, mActStatusPdl);

	mFbv.setLedOnOff(LINE6FBV_MOD, mActStatusMod);

	mFbv.setLedOnOff(LINE6FBV_REVERB, mActStatusRev);

	mFbv.setLedOnOff(LINE6FBV_DELAY, mActStatusDly);
}

// send delay time every tap starting with the second tap to get an interval
void fProcessTap(){
	int ms;

	if (mTapModeActive){
		ms = millis() - mLastTap;
		Serial.print("APP send ms ");
		Serial.println(ms);
		mVox.sendDelayTime(ms);
		mFbv.setLedFlash(LINE6FBV_TAP, ms);
	}
	mTapModeActive = 1;
	mLastTap = millis();

}

void fProcessTuner(byte inKey){

	if (mTunerIsOn){
		mVox.switchTuner(0, 0);
		mTunerIsSilent = 0;
		fDisplayPgmInfo();
	}
	else{
		onVoxTunerValue(0x3b, 0x20);
		if (inKey == LINE6FBV_STOMP1){
			mVox.switchTuner(1, 1);
			mTunerIsSilent = 1;
		}
		else{
			mVox.switchTuner(1, 0);
			mTunerIsSilent = 0;
		}
	}
	mTunerIsOn = !mTunerIsOn;
	fSetTunerLed();
}


void fChangeProgram(byte inKey){
	byte pgmNum;
	pgmNum = mNextBank * 4;

	switch (inKey){
	case  LINE6FBV_CHANNELA:
		break; // nothhing to add
	case  LINE6FBV_CHANNELB:
		pgmNum += 1;
		break;
	case  LINE6FBV_CHANNELC:
		pgmNum += 2;
		break;
	case  LINE6FBV_CHANNELD:
		pgmNum += 3;
		break;
	}

	Serial.print("APP pgmChange ");
	Serial.println(pgmNum);


	if (mActPgmNum != pgmNum){

		mActPgmNum = pgmNum;    // remember PgmNum
		mActBank = mNextBank;
		mActChannel = pgmNum % 4;

		mWaitForStompInfo = 1;

		mVox.switchTuner(0, 0);  // switch tuner off
		mVox.sendPgmChange(pgmNum);
		mVox.sendCtlChange(VOXAD60VT_VOL, mActValVol);  // send Vol Pedal Pos  

		fDisplayPgmInfo();      // update display

	}


}

void fSetNewBankValue(byte inKey){

	char title[16] = "               ";


	switch (inKey){
	case LINE6FBV_UP:
		if (mNextBank < 7)
			mNextBank++;
		break;
	case LINE6FBV_DOWN:
		if (mNextBank > 0)
			mNextBank--;
		break;
	}

	Serial.print("APP mNextBank");
	Serial.println(mNextBank);

	// Display new bank in the title field

	if (mNextBank != mActBank){
		/* unfortunately not displayable characters
		title[2] = '=';
		title[3] = '=';
		title[4] = '>';
		*/
		title[6] = 0x31 + mNextBank;
	}

	mFbv.setDisplayTitle(title);

}



void onFbvKeyReleased(byte inKey, byte inKeyHeld) {
	//	mFbv.setLedOnOff(inKey, 0x00);

	switch (inKey){
	case LINE6FBV_UP:
		mFbv.setLedOnOff(LINE6FBV_UP, 0);
		break;
	case LINE6FBV_DOWN:
		mFbv.setLedOnOff(LINE6FBV_DOWN, 0);
		break;
	}

}

void onFbvCtlChanged(byte inCtrl, byte inValue) {
	Serial.print("FBV CtlChanged ");
	Serial.println(inValue, HEX);
	if (inCtrl == LINE6FBV_PDL1){
		mWahLastMove = millis();
		if (!mActStatusPdl && mWahAutoOnOff){
			mActStatusPdl = 1;
			mActValWah = inValue;
			mVox.switchStompBoxes(mActStatusPdl, mActStatusMod, mActStatusDly, mActStatusRev);
			fSetStompLeds();

		}
		mVox.sendCtlChange(VOXAD60VT_WAH, inValue);
	}
	else{
		mActValVol = inValue;
		mVox.sendCtlChange(VOXAD60VT_VOL, inValue);
	}

}

void onFbvHeartbeat() {
	mFbv.setLedOnOff(LINE6FBV_DISPLAY, 0x01);
	Serial.println("FBV: Heartbeat");
	mVox.sendCtlChange(VOXAD60VT_WAH, mActValWah);

}

void onVoxPgmChanged(byte inPgmNo){

	// take the following Stomp Info to set Wah auto On Off
	mWaitForStompInfo = 1;

	mActChannel = inPgmNo % 4;
	mActBank = inPgmNo / 4;
	mNextBank = mActBank;

	fDisplayPgmInfo();

	Serial.print("VOX: PgmChanged ");
	Serial.println(inPgmNo);
}

void fDisplayPgmInfo(){

	byte bankDigit;
	byte channels[4] = { 0, 0, 0, 0 };

	channels[mActChannel] = 1;

	bankDigit = mActBank + 0x31;

	mFbv.setDisplayDigit(0, 0x20);
	mFbv.setDisplayDigit(1, bankDigit);
	mFbv.setDisplayDigit(2, 0x20);
	mFbv.setDisplayDigit(3, 0x20);
	mFbv.setDisplayFlat(0);
	mFbv.setDisplayTitle("");
	mFbv.setLedOnOff(LINE6FBV_CHANNELA, channels[0]);
	mFbv.setLedOnOff(LINE6FBV_CHANNELB, channels[1]);
	mFbv.setLedOnOff(LINE6FBV_CHANNELC, channels[2]);
	mFbv.setLedOnOff(LINE6FBV_CHANNELD, channels[3]);

	Serial.print("fDisplayPgmInfo ");
	Serial.print(channels[0]);
	Serial.print(channels[1]);
	Serial.print(channels[2]);
	Serial.println(channels[3]);

}



void onVoxStomp(byte inPdl, byte inMod, byte inDly, byte inRev){

	Serial.print("VOX Stomp Pdl ");
	Serial.print(inPdl);
	Serial.print(" MOD ");
	Serial.print(inMod);
	Serial.print(" DLY ");
	Serial.print(inDly);
	Serial.print(" REV ");
	Serial.println(inRev);

	if (mWaitForStompInfo){
		if (inPdl){
			mWahAutoOnOff = 0;
		}
		else{
			mWahAutoOnOff = 1;
			Serial.println("APP Wah Auto=On");
		}
		mWaitForStompInfo = 0;
		mFbv.setLedOnOff(LINE6FBV_STOMP3, mWahAutoOnOff);
	}


	mActStatusPdl = inPdl;
	mActStatusMod = inMod;
	mActStatusDly = inDly;
	mActStatusRev = inRev;
	fSetStompLeds();
}

void onVoxDelayTime(int inDelayTime) {

	Serial.print("VOX: mDelayTime ");
	Serial.println(inDelayTime);
	mFbv.setLedFlash(LINE6FBV_TAP, inDelayTime);
}
void onVoxTunerOnOff(byte inOnOff){
	mTunerIsOn = inOnOff;
	if (!mTunerIsOn){
		mTunerIsSilent = 0;
		fDisplayPgmInfo();
	}
	else{
		onVoxTunerValue(0x3b, 0x20);
	}
	fSetTunerLed();
}

void onVoxTunerSilent(){
	mTunerIsSilent = 1;
	mTunerIsOn = 1;
	fSetTunerLed();
}

void fSetTunerLed(){
	if (!mTunerIsOn){
		mFbv.setLedOnOff(LINE6FBV_FXLOOP, 0);
		mFbv.setLedOnOff(LINE6FBV_STOMP1, 0);
	}
	else if (mTunerIsSilent){
		mFbv.setLedOnOff(LINE6FBV_FXLOOP, 0);
		mFbv.setLedOnOff(LINE6FBV_STOMP1, 1);
	}
	else {
		mFbv.setLedOnOff(LINE6FBV_FXLOOP, 1);
		mFbv.setLedOnOff(LINE6FBV_STOMP1, 0);
	}

}

void onVoxTunerValue(byte inNote, byte inPrecission){

	char noteDigit;
	byte flat;
	char* tuneString;

	Serial.print("VOX Tuner ");
	Serial.print(inNote, HEX);
	Serial.print(" ");
	Serial.print(inPrecission, HEX);


	switch (inNote){
	case 0x3b: // no Sound
		noteDigit = ' ';
		flat = 0;
		break;
	case 0x3c:
		noteDigit = 'C';
		flat = 0;
		break;
	case 0x3d:
		noteDigit = 'D';
		flat = 1;
		break;
	case 0x3e:
		noteDigit = 'D';
		flat = 0;
		break;
	case 0x3f:
		noteDigit = 'E';
		flat = 1;
		break;
	case 0x40:
		noteDigit = 'E';
		flat = 0;
		break;
	case 0x41:
		noteDigit = 'F';
		flat = 0;
		break;
	case 0x42:
		noteDigit = 'G';
		flat = 1;
		break;
	case 0x43:
		noteDigit = 'G';
		flat = 0;
		break;
	case 0x44:
		noteDigit = 'A';
		flat = 1;
		break;
	case 0x45:
		noteDigit = 'A';
		flat = 0;
		break;
	case 0x46:
		noteDigit = 'B';
		flat = 1;
		break;
	case 0x47:
		noteDigit = 'B';
		flat = 0;
		break;
	}

	if (noteDigit == ' ')          tuneString = "I              I";
	else if (inPrecission <= 0x08) tuneString = "I        ( ( ( I";
	else if (inPrecission <= 0x0A) tuneString = "I        ( (   I";
	else if (inPrecission <= 0x0E) tuneString = "I        (     I";
	else if (inPrecission == 0x0F) tuneString = "I      **(     I";
	else if (inPrecission == 0x10) tuneString = "I      **      I";
	else if (inPrecission == 0x11) tuneString = "I     )**      I";
	else if (inPrecission <= 0x15) tuneString = "I     )        I";
	else if (inPrecission <= 0x18) tuneString = "I   ) )        I";
	else                           tuneString = "I ) ) )        I";

	Serial.print(" ");
	Serial.print(noteDigit);
	Serial.print(" ");
	Serial.println(tuneString);

	mFbv.setDisplayDigit(0, ' ');
	mFbv.setDisplayDigit(1, ' ');
	mFbv.setDisplayDigit(2, ' ');
	mFbv.setDisplayDigit(3, noteDigit);
	mFbv.setDisplayFlat(flat);
	mFbv.setDisplayTitle(tuneString);

}

void onVoxReset(){

}


void setup() {

	Serial.begin(38400);

	// use Serial1 Port on Arduino Mega for the FBV
	mFbv.begin(&Serial1); // open port

	// set callback functions for the FBV
	mFbv.setHandleKeyPressed(&onFbvKeyPressed);
	mFbv.setHandleKeyReleased(&onFbvKeyReleased);
	mFbv.setHandleHeartbeat(&onFbvHeartbeat);
	mFbv.setHandleCtrlChanged(&onFbvCtlChanged);

	// use Serial2 Port on Arduino Mega for the VOX
	mVox.begin(&Serial2); // open port

	// set callback functions for the VOX
	mVox.setHandleStomp(&onVoxStomp);
	mVox.setHandleDelayTime(&onVoxDelayTime);
	mVox.setHandleTunerValue(&onVoxTunerValue);
	mVox.setHandleTunerOnOff(&onVoxTunerOnOff);
	mVox.setHandleTunerSilent(&onVoxTunerSilent);
	mVox.setHandlePgmChanged(&onVoxPgmChanged);
	mVox.setHandleReset(&onVoxReset);

	// switch dispay light on and display "hello world"
	mFbv.setLedOnOff(LINE6FBV_DISPLAY, 1);
	mFbv.setDisplayTitle("HELLO WORLD");
	Serial.println("ready");

}

void loop() {

	mFbv.read();  // Receive Commands from FBV

	mVox.read(); // Receive Commands from VOX

	fWahAutoOff(); // switch auto-wah off if time expired

	fDeactivateTapMode(); // leave tap mode if time expired

	mFbv.updateUI();  // update Display and LEDs on the FBV to the values set in the setDisplay..... routines
}
