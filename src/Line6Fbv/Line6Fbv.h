/*!
*  @file       Line6Fbv.h
*  Project     Arduino Line6 FBV Longboard to MIDI Library
*  @brief      Line6 FBV Library for the Arduino
*  @version    0.4
*  @author     Joachim Wrba
*  @date       09/08/15
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
*/

/*
====Data received from the FBV:

a heartbeat is sent every 7 seconds
F0 02 90 00
F0 02 30 08

Switches:
F0 03 81 <key-code><pressed=01/released=00>

Pedals:
F0 03 82 <00=Wah/01=Vol><Value 00 – 7F>

====Data sent to the FBV

LEDs on/off
F0 03 04 <key-code><00=off/01=on>

Display:
the 16 char title
F0 13 10 00 10 <16 char title>

the first 4 digits separate:
the first three must be numeric or space
F0 02 <numDigit = 0-3> <char>

the first 4 digits together
F0 05 08 + 4 characters (1-3 numeric)

clear title
F0 01 11

the flat sign
F0 02 20 <00=off/01=on>


*/

#ifndef LINE6FBV_H
#define LINE6FBV_H

#include <Arduino.h>

// Constants used for different switches and LEDs
// Switch + LED
const byte LINE6FBV_FXLOOP = 0x02;
const byte LINE6FBV_STOMP1 = 0x12;
const byte LINE6FBV_STOMP2 = 0x22;
const byte LINE6FBV_STOMP3 = 0x32;
const byte LINE6FBV_AMP1 = 0x01;
const byte LINE6FBV_AMP2 = 0x11;
const byte LINE6FBV_REVERB = 0x21;
const byte LINE6FBV_PITCH = 0x31;
const byte LINE6FBV_MOD = 0x41;
const byte LINE6FBV_DELAY = 0x51;
const byte LINE6FBV_TAP = 0x61;
const byte LINE6FBV_DOWN = 0x00;
const byte LINE6FBV_UP = 0x10;
const byte LINE6FBV_CHANNELA = 0x20;
const byte LINE6FBV_CHANNELB = 0x30;
const byte LINE6FBV_CHANNELC = 0x40;
const byte LINE6FBV_CHANNELD = 0x50;
const byte LINE6FBV_FAVORITE = 0x60;

// SWITCH Only
const byte LINE6FBV_PDL1_SW = 0x43;
const byte LINE6FBV_PDL2_SW = 0x53;

// LED ONLY
const byte LINE6FBV_PDL1_GRN = 0x03;
const byte LINE6FBV_PDL1_RED = 0x13;
const byte LINE6FBV_PDL2_GRN = 0x33;
const byte LINE6FBV_PDL2_RED = 0x23;
const byte LINE6FBV_DISPLAY = 0x0A;


const byte LINE6FBV_PDL1 = 0x00;
const byte LINE6FBV_PDL2 = 0x01;


// internal use only, don't know how to code a constant inside the class
const int LINE6FBV_NUM_LED_AND_SWITCH = 25;
const int LINE6FBV_FLASH_TIME = 50;
const int LINE6FBV_HOLD_TIME = 2000;


class Line6Fbv {
public:


	// Definitions for callback functions
	typedef void FunctTypeCbKeyPressed(byte);
	typedef void FunctTypeCbKeyReleased(byte, byte); // the second byte indicates if the key wad held before
	typedef void FunctTypeCbCtrlChanged(byte, byte);
	typedef void FunctTypeCbHeartbeat();
	typedef void FunctTypeCbKeyHeld(byte);

	// just the constructor
	Line6Fbv();

	void begin(HardwareSerial* inSerial);

	// interpret incoming bytes and fire callback functions
	void read();

	// switch status of a LED on or off --> updateUI must be called
	void setLedOnOff(byte inLed, byte inOnOff);

	// set staus of  a LED to flash --> updateUI must be called
	void setLedFlash(byte inLed, int inDelayTime);

	// process all LED changes on the FBV at once
	void updateUI();

	// set the 16 character Title --> updateUI must be called
	void setDisplayTitle(char* inTitle);

	// set one of the first 4 digits (inNumDigit = 0-3): --> updateUI must be called
	// the first 3 can be a character '0' - '9' or space
	// the 4th is used for channels A-D or note names
	void setDisplayDigit(int inNumDigit, char inDigit);

	// set the first 4 digits at once --> updateUI must be called
	void setDisplayDigits(char* inDigits);

	// display the flat sign (b) --> updateUI must be called
	void setDisplayFlat(byte inOnOff);

	// let the diusplay light flash --> updateUI must be called
	void setDisplayFlash(int inOnTime, int inOffTime);

	// set a callback Function for pressed Key
	void setHandleKeyPressed(FunctTypeCbKeyPressed* cb);

	// set a callback Function for released Key
	void setHandleKeyReleased(FunctTypeCbKeyReleased* cb);

	// set a callback Function for held Key
	void setHandleKeyHeld(FunctTypeCbKeyHeld* cb);

	// set a callback Function for pedal usage
	void setHandleCtrlChanged(FunctTypeCbCtrlChanged* cb);

	// set a callback Function for heartbeat
	// a heartbeat is sent every 7 seconds
	// this callback can be used to check the connection status
	void setHandleHeartbeat(FunctTypeCbHeartbeat* cb);




private:

	FunctTypeCbKeyPressed*  mCbKeyPressed;
	FunctTypeCbKeyReleased* mCbKeyReleased;
	FunctTypeCbKeyHeld*     mCbKeyHeld;
	FunctTypeCbCtrlChanged* mCbCtrlChanged;
	FunctTypeCbHeartbeat*   mCbHeartbeat;

	struct LedAndSwitch{
		byte key;
		int onTime;
		int offTime;
		int waitTime;
		unsigned long lastMillis;
		byte isOn;
		byte setOn;
		byte setOff;
		byte flash;
		int holdTime;
		unsigned long lastPressTime;
		byte isHeld;
		byte isPressed;
	};

	struct Display{
		char numDigits[3];
		char noteDigit;
		byte flat;
		char title[16];
		int onTime;
		int offTime;
		int waitTime;
		unsigned long lastMillis;
		byte isShown;
		byte show;
		byte hide;
		byte flash;

	};

	Display mDisplay;
	Display mDisplayEmpty;  // for flashing
	LedAndSwitch mLedAndSwitch[LINE6FBV_NUM_LED_AND_SWITCH];
	HardwareSerial * mSerial;
	byte mDataBytes[5];
	int mByteCount;
	int mBytesExpected;

	// send Number, Note, Title to the display
	void sendDisplayData(Display inDisplay);

	void mStartHold(byte inKey);
	byte mStopHold(byte inKey);
	void mCheckHold();
};
#endif
