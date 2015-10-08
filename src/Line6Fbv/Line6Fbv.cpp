/*!
*  @file       Line6Fbv.cpp
*  Project     Arduino Line6 FBV Longboard to MIDI Library
*  @brief      Line6 FBV Library for the Arduino
*  @version    1.0
*  @author     Joachim Wrba
*  @date       2015.10.08
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
#include "Line6Fbv.h"
//#include <Arduino.h>

Line6Fbv::Line6Fbv() {



	mCbKeyPressed = 0;
	mCbKeyReleased = 0;
	mCbKeyHeld = 0;
	mCbCtrlChanged = 0;
	mCbHeartbeat = 0;

	mDataBytes[0] = 0;
	mByteCount = 0;
	mBytesExpected = 0;

	mLedAndSwitch[0].key = LINE6FBV_FXLOOP;
	mLedAndSwitch[1].key = LINE6FBV_STOMP1;
	mLedAndSwitch[2].key = LINE6FBV_STOMP2;
	mLedAndSwitch[3].key = LINE6FBV_STOMP3;
	mLedAndSwitch[4].key = LINE6FBV_AMP1;
	mLedAndSwitch[5].key = LINE6FBV_AMP2;
	mLedAndSwitch[6].key = LINE6FBV_REVERB;
	mLedAndSwitch[7].key = LINE6FBV_PITCH;
	mLedAndSwitch[8].key = LINE6FBV_MOD;
	mLedAndSwitch[9].key = LINE6FBV_DELAY;
	mLedAndSwitch[10].key = LINE6FBV_TAP;
	mLedAndSwitch[11].key = LINE6FBV_UP;
	mLedAndSwitch[12].key = LINE6FBV_DOWN;
	mLedAndSwitch[13].key = LINE6FBV_CHANNELA;
	mLedAndSwitch[14].key = LINE6FBV_CHANNELB;
	mLedAndSwitch[15].key = LINE6FBV_CHANNELC;
	mLedAndSwitch[16].key = LINE6FBV_CHANNELD;
	mLedAndSwitch[17].key = LINE6FBV_FAVORITE;
	mLedAndSwitch[18].key = LINE6FBV_PDL1_GRN;
	mLedAndSwitch[19].key = LINE6FBV_PDL1_RED;
	mLedAndSwitch[20].key = LINE6FBV_PDL2_GRN;
	mLedAndSwitch[21].key = LINE6FBV_PDL2_RED;
	mLedAndSwitch[22].key = LINE6FBV_DISPLAY;
	mLedAndSwitch[23].key = LINE6FBV_PDL1;
	mLedAndSwitch[24].key = LINE6FBV_PDL2;


	for (int i = 0; i < LINE6FBV_NUM_LED_AND_SWITCH; i++){
		mLedAndSwitch[i].isOn = 0;
		mLedAndSwitch[i].setOn = 0;
		mLedAndSwitch[i].setOff = 0;
		mLedAndSwitch[i].flash = 0;
		mLedAndSwitch[i].holdTime = LINE6FBV_HOLD_TIME;
		mLedAndSwitch[i].isHeld = 0;
	}

	mDisplayEmpty.numDigits[0] = 0x20;
	mDisplayEmpty.numDigits[1] = 0x20;
	mDisplayEmpty.numDigits[2] = 0x20;
	mDisplayEmpty.noteDigit = 0x20;
	mDisplayEmpty.flat = 0;
	for (int i = 0; i < 16; i++){
		mDisplayEmpty.title[i] = 0x20;
	}

}

void Line6Fbv::begin(HardwareSerial * inSerial) {
	mSerial = inSerial;
	mSerial->begin(32150);

}


void Line6Fbv::setHandleKeyPressed(FunctTypeCbKeyPressed* cb) {
	mCbKeyPressed = cb;
}

void Line6Fbv::setHandleKeyReleased(FunctTypeCbKeyReleased* cb) {
	mCbKeyReleased = cb;
}

void Line6Fbv::setHandleKeyHeld(FunctTypeCbKeyHeld* cb) {
	mCbKeyHeld = cb;
}


void Line6Fbv::setHandleCtrlChanged(FunctTypeCbCtrlChanged* cb) {
	mCbCtrlChanged = cb;
}

void Line6Fbv::setHandleHeartbeat(FunctTypeCbHeartbeat* cb) {
	mCbHeartbeat = cb;
}


void Line6Fbv::setLedOnOff(byte inLed, byte inOnOff) {

	// Find the Switch in the array and set the value
	for (int i = 0; i < LINE6FBV_NUM_LED_AND_SWITCH; i++){
		if (mLedAndSwitch[i].key == inLed){
			mLedAndSwitch[i].flash = 0;
			if (inOnOff){
				mLedAndSwitch[i].setOn = 1;
				mLedAndSwitch[i].setOff = 0;
			}
			else{
				mLedAndSwitch[i].setOff = 1;
				mLedAndSwitch[i].setOn = 0;
			}

			i = LINE6FBV_NUM_LED_AND_SWITCH; // exit loop
		}
	}

}

void Line6Fbv::syncLedFlash() {
	for (int i = 0; i < LINE6FBV_NUM_LED_AND_SWITCH; i++){
		if (mLedAndSwitch[i].flash){
			mLedAndSwitch[i].waitTime = 0;
			mLedAndSwitch[i].isOn = false;
			mLedAndSwitch[i].lastMillis = 0;
		}
	}

}

void Line6Fbv::setLedFlash(byte inLed, int inDelayTime) {
	setLedFlash(inLed, inDelayTime, LINE6FBV_FLASH_TIME);
}

void Line6Fbv::setLedFlash(byte inLed, int inDelayTime, int inOnTime) {

	// Find the Switch in the array and set the value
	for (int i = 0; i < LINE6FBV_NUM_LED_AND_SWITCH; i++){
		if (mLedAndSwitch[i].key == inLed){
			mLedAndSwitch[i].flash = 1;
			mLedAndSwitch[i].isOn = 0;
			mLedAndSwitch[i].lastMillis = 0;
			if (inDelayTime > inOnTime){
				mLedAndSwitch[i].offTime = inDelayTime - inOnTime; // ToDo intervals < 50 ms
				mLedAndSwitch[i].onTime = inOnTime;
			}
			else{
				mLedAndSwitch[i].offTime = inDelayTime / 2;
				mLedAndSwitch[i].onTime = inDelayTime / 2;
			}
			i = LINE6FBV_NUM_LED_AND_SWITCH; // exit loop
		}
	}

}
void Line6Fbv::updateUI(){

	unsigned long currentMillis = millis();
	

	// LEDs
	for (int i = 0; i < LINE6FBV_NUM_LED_AND_SWITCH; i++){
		if (mLedAndSwitch[i].setOn){
			mLedAndSwitch[i].setOn = 0;
			mLedAndSwitch[i].isOn = 1;
			mLedAndSwitch[i].flash = 0;
			mSerial->write(0xF0);
			mSerial->write(0x03);
			mSerial->write(0x04);
			mSerial->write(mLedAndSwitch[i].key);
			mSerial->write(0x01);
			//			Serial.print("LED On  : ");
			//			Serial.println(mLedAndSwitch[i].key, HEX);
		}
		else 		if (mLedAndSwitch[i].setOff){
			//			Serial.print("LED Off  : ");
			//			Serial.println(mLedAndSwitch[i].key, HEX);
			mLedAndSwitch[i].setOff = 0;
			mLedAndSwitch[i].isOn = 0;
			mLedAndSwitch[i].flash = 0;
			mSerial->write(0xF0);
			mSerial->write(0x03);
			mSerial->write(0x04);
			mSerial->write(mLedAndSwitch[i].key);
			mSerial->write(0x00);
		}
		else if (mLedAndSwitch[i].flash){
			//			Serial.print("LED flash  : ");
			//			Serial.println(mLedAndSwitch[i].key, HEX);
			if (currentMillis - mLedAndSwitch[i].lastMillis >= mLedAndSwitch[i].waitTime) {
				mLedAndSwitch[i].lastMillis = currentMillis;
				if (!mLedAndSwitch[i].isOn)
					mLedAndSwitch[i].waitTime = mLedAndSwitch[i].onTime;
				else
					mLedAndSwitch[i].waitTime = mLedAndSwitch[i].offTime;
				mLedAndSwitch[i].isOn = !mLedAndSwitch[i].isOn;
				mSerial->write(0xF0);
				mSerial->write(0x03);
				mSerial->write(0x04);
				mSerial->write(mLedAndSwitch[i].key);
				mSerial->write(mLedAndSwitch[i].isOn);
			}
		}

	}

	// Display
	if (mDisplay.show){
		mDisplay.show = 0;
		mDisplay.isShown = 1;
		mDisplay.flash = 0;
		sendDisplayData(mDisplay);
	}
	else if (mDisplay.hide){
		mDisplay.hide = 0;
		mDisplay.isShown = 0;
		mDisplay.flash = 0;
		sendDisplayData(mDisplayEmpty);
	}
	else if (mDisplay.flash){
		
		if (currentMillis - mDisplay.lastMillis >= mDisplay.waitTime) {
			mDisplay.lastMillis = currentMillis;
			if (!mDisplay.isShown)
				mDisplay.waitTime = mDisplay.onTime;
			else
				mDisplay.waitTime = mDisplay.offTime;

			mDisplay.isShown = !mDisplay.isShown;
			
			if (mDisplay.isShown)
				sendDisplayData(mDisplay);
			else
				sendDisplayData(mDisplayEmpty);
		}
	}
}

void Line6Fbv::read() {

	byte inByte;

	if (mSerial->available() > 0) {
		while (mSerial->available() > 0) {
			inByte = mSerial->read();
			switch (mByteCount) {
			case 0:
				if (inByte == 0xF0) {
					mDataBytes[0] = inByte;
					mByteCount = 1;
					mBytesExpected = 0;
				}
				break;
			case 1:
				switch (inByte) {
				case 0x02:         // Heartbeat
					mDataBytes[1] = inByte;
					mBytesExpected = 4;
					mByteCount = 2;
					break;
				case 0x03:         // Switch / Pedal
					mDataBytes[1] = inByte;
					mBytesExpected = 5;
					mByteCount = 2;
					break;
				default:          // Nothing to do
					mByteCount = 0;
					mBytesExpected = 0;
				}
				break;
			case 2:
				switch (inByte) {
				case 0x81:         // Switch
				case 0x82:         // Pedal
				case 0x90:         // Heartbeat part 1
				case 0x30:         // Heartbeat part 2
					mDataBytes[2] = inByte;
					mByteCount = 3;
					break;
				default:          // Nothing to do
					mByteCount = 0;
					mBytesExpected = 0;
				}
				break;
			case 3:
				mDataBytes[3] = inByte;
				mByteCount = 4;
				break;
			case 4:
				mDataBytes[4] = inByte;
				mByteCount = 5;
				break;
			}
			if (mByteCount) {
				if (mByteCount == mBytesExpected) {
					if (mBytesExpected == 4 && mDataBytes[2] == 0x30) { // Heartbeat
						if (mCbHeartbeat) {
							mCbHeartbeat();
						}
					}

					if (mBytesExpected == 5) {
						switch (mDataBytes[2]) {
						case 0x82:
							if (mCbCtrlChanged) {
								mCbCtrlChanged(mDataBytes[3], mDataBytes[4]);
							}
							break;
						case 0x81:
							if (mDataBytes[4] == 0x00 && mCbKeyReleased) {

								mCbKeyReleased(mDataBytes[3], mStopHold(mDataBytes[3]));
							}

							if (mDataBytes[4] == 0x01 && mCbKeyPressed) {
								mStartHold(mDataBytes[3]);
								mCbKeyPressed(mDataBytes[3]);
							}

						}
					}
					mByteCount = 0;
					mBytesExpected = 0;
				}
			}
		}
	}
	mCheckHold();   // check elapsed time for "hold" state
}

void Line6Fbv::setDisplayTitle(byte* inTitle){

	
	char title[16];

	for (size_t i = 0; i < 16; i++)
	{
			title[i] = inTitle[i];
			if (title[i] == 0x00)
				i = 16;
	}

	setDisplayTitle(title);
}
void Line6Fbv::setDisplayTitle(char* inTitle){

	char title[16];

	strncpy(title, inTitle, 16);
	mDisplay.flash = 0;
	mDisplay.show = 1;

	for (int i = 0; i < 16; i++){
		mDisplay.title[i] = title[i];
	}
}
void Line6Fbv::setDisplayDigit(int inNum, char inDigit){
	mDisplay.flash = 0;
	mDisplay.show = 1;

	if (inNum < 3)
		mDisplay.numDigits[inNum] = inDigit;
	else
		mDisplay.noteDigit = inDigit;

}

void Line6Fbv::setDisplayDigits(char* inDigits){

	char digits[4];

	strncpy(digits, inDigits, 4);
	mDisplay.flash = 0;
	mDisplay.show = 1;

	for (int i = 0; i < 3; i++){
		mDisplay.numDigits[i] = digits[i];
	}
	mDisplay.noteDigit = digits[3];

}

void Line6Fbv::setDisplayNumber(int inNumber){
	int number;
	byte digit_100;
	byte digit_10;
	byte digit_1;

	number = inNumber % 1000; // only 3 digits possible

	digit_100 = number / 100;
	number = number % 100;
	digit_10 = number / 10;
	digit_1 = number % 10;

	// char value for int
	digit_100 += 0x30;
	digit_10 += 0x30;
	digit_1 += 0x30;

	// suppress leading zero
	if (digit_100 == 0x30){
		digit_100 = 0x20;
		if (digit_10 == 0x30){
			digit_10 = 0x20;
		}
	}

	setDisplayDigit(0, digit_100);
	setDisplayDigit(1, digit_10);
	setDisplayDigit(2, digit_1);

}


void Line6Fbv::setDisplayFlat(byte inOnOff){
	mDisplay.flat = inOnOff;
}

void Line6Fbv::setDisplayFlash(int inOnTime, int inOffTime){

	mDisplay.flash = true;
	mDisplay.show = false;
	mDisplay.hide = false; 
	mDisplay.onTime = inOnTime;
	mDisplay.offTime = inOffTime;
	mDisplay.lastMillis = 0;
}


void Line6Fbv::sendDisplayData(Display inDisplay){
	/* each of the first 4 digits:
	mSerial->write(0xF0);
	mSerial->write(0x02);
	mSerial->write(numDigit);
	mSerial->write(inDigit);
	*/

	/* clear display title
	Serial1.write(0xF0);
	Serial1.write(0x01);
	Serial1.write(0x11);
	*/

	// the first 4 digits together
	mSerial->write(0xF0);
	mSerial->write(0x05);
	mSerial->write(0x08);
	mSerial->write(inDisplay.numDigits[0]);
	mSerial->write(inDisplay.numDigits[1]);
	mSerial->write(inDisplay.numDigits[2]);
	mSerial->write(inDisplay.noteDigit);

	// flat sign
	mSerial->write(0xF0);
	mSerial->write(0x02);
	mSerial->write(0x20);
	mSerial->write(inDisplay.flat);

	// title
	mSerial->write(0xF0);
	mSerial->write(0x13);
	mSerial->write(0x10);
	mSerial->write(0x00);
	mSerial->write(0x10);
	for (int i = 0; i < 16; i++){
		mSerial->write(inDisplay.title[i]);
	}


}

void Line6Fbv::mStartHold(byte inKey){
	// Find the Switch in the array and set the value
	for (int i = 0; i < LINE6FBV_NUM_LED_AND_SWITCH; i++){
		if (mLedAndSwitch[i].key == inKey){
			mLedAndSwitch[i].isHeld = 0;
			mLedAndSwitch[i].isPressed = 1;
			mLedAndSwitch[i].lastPressTime = millis();
			i = LINE6FBV_NUM_LED_AND_SWITCH; // exit loop
		}
	}
};


// called when key is released, returns hold state, so the hold information needs not to be store in the calling application
byte Line6Fbv::mStopHold(byte inKey){

	byte retVal = 0;

	// Find the Switch in the array and set the value
	for (int i = 0; i < LINE6FBV_NUM_LED_AND_SWITCH; i++){
		if (mLedAndSwitch[i].key == inKey){
			retVal = mLedAndSwitch[i].isHeld;
			mLedAndSwitch[i].isHeld = 0;
			mLedAndSwitch[i].isPressed = 0;
			i = LINE6FBV_NUM_LED_AND_SWITCH; // exit loop
		}
	}
	return retVal;
};


// check if hold time is elapsed while key is pressed
void Line6Fbv::mCheckHold(){

	unsigned long currentMillis = millis();
	for (int i = 0; i < LINE6FBV_NUM_LED_AND_SWITCH; i++){
		if (mLedAndSwitch[i].isPressed && !mLedAndSwitch[i].isHeld){
			if (currentMillis - mLedAndSwitch[i].lastPressTime >= mLedAndSwitch[i].holdTime) {
				if (mCbKeyHeld){
					mLedAndSwitch[i].isHeld = 1;
					mCbKeyHeld(mLedAndSwitch[i].key);
				}
			}
		}
	}

};
