/*!
*  @file       Line6Fbv.cpp
*  Project     Arduino Line6 FBV Longboard to MIDI Library
*  @brief      Line6 FBV Library for the Arduino
*  @version    1.1
*  @author     Joachim Wrba
*  @date       15/10/16
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
	mCbDisconnected = 0;

	mDataBytes[0] = 0;
	mByteCount = 0;
	mBytesExpected = 0;

	


	mLedAndSwitch[LINE6FBV_KEY_NONE].key = LINE6FBV_CC_KEY_NONE;
	mLedAndSwitch[LINE6FBV_FXLOOP].key = LINE6FBV_CC_FXLOOP;
	mLedAndSwitch[LINE6FBV_STOMP1].key = LINE6FBV_CC_STOMP1;
	mLedAndSwitch[LINE6FBV_STOMP2].key = LINE6FBV_CC_STOMP2;
	mLedAndSwitch[LINE6FBV_STOMP3].key = LINE6FBV_CC_STOMP3;
	mLedAndSwitch[LINE6FBV_AMP1].key = LINE6FBV_CC_AMP1;
	mLedAndSwitch[LINE6FBV_AMP2].key = LINE6FBV_CC_AMP2;
	mLedAndSwitch[LINE6FBV_REVERB].key = LINE6FBV_CC_REVERB;
	mLedAndSwitch[LINE6FBV_PITCH].key = LINE6FBV_CC_PITCH;
	mLedAndSwitch[LINE6FBV_MOD].key = LINE6FBV_CC_MOD;
	mLedAndSwitch[LINE6FBV_DELAY].key = LINE6FBV_CC_DELAY;
	mLedAndSwitch[LINE6FBV_TAP].key = LINE6FBV_CC_TAP;
	mLedAndSwitch[LINE6FBV_UP].key = LINE6FBV_CC_UP;
	mLedAndSwitch[LINE6FBV_DOWN].key = LINE6FBV_CC_DOWN;
	mLedAndSwitch[LINE6FBV_CHANNELA].key = LINE6FBV_CC_CHANNELA;
	mLedAndSwitch[LINE6FBV_CHANNELB].key = LINE6FBV_CC_CHANNELB;
	mLedAndSwitch[LINE6FBV_CHANNELC].key = LINE6FBV_CC_CHANNELC;
	mLedAndSwitch[LINE6FBV_CHANNELD].key = LINE6FBV_CC_CHANNELD;
	mLedAndSwitch[LINE6FBV_FAVORITE].key = LINE6FBV_CC_FAVORITE;
	mLedAndSwitch[LINE6FBV_PDL1_GRN].key = LINE6FBV_CC_PDL1_GRN;
	mLedAndSwitch[LINE6FBV_PDL1_RED].key = LINE6FBV_CC_PDL1_RED;
	mLedAndSwitch[LINE6FBV_PDL2_GRN].key = LINE6FBV_CC_PDL2_GRN;
	mLedAndSwitch[LINE6FBV_PDL2_RED].key = LINE6FBV_CC_PDL2_RED;
	mLedAndSwitch[LINE6FBV_DISPLAY].key = LINE6FBV_CC_DISPLAY;
	mLedAndSwitch[LINE6FBV_PDL1_SW].key = LINE6FBV_CC_PDL1_SW;
	mLedAndSwitch[LINE6FBV_PDL2_SW].key = LINE6FBV_CC_PDL2_SW;


	for (int i = 0; i < LINE6FBV_NUM_LED_AND_SWITCH; i++){
		mLedAndSwitch[i].isOn = 0;
		mLedAndSwitch[i].setOn = 0;
		mLedAndSwitch[i].setOff = 0;
		mLedAndSwitch[i].flash = 0;
		mLedAndSwitch[i].holdTime = 0;
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

void Line6Fbv::requestBoardType(void) {
	mSerial->write(0xF0);
	mSerial->write(0x02);
	mSerial->write(0x01);
	mSerial->write(0x00);
}

void Line6Fbv::requestPedalPos(void) {
	mSerial->write(0xF0);
	mSerial->write(0x02);
	mSerial->write(0x01);
	mSerial->write(0x01);
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

void Line6Fbv::setHandleDisconnected(FunctTypeCbDisconnected* cb) {
	mCbDisconnected = cb;
}


    uint8_t Line6Fbv::mGetLedInArray(byte inCC){
	uint8_t retVal = LINE6FBV_KEY_NONE;

	// Find the Switch in the array and set the value
	for (uint8_t i = 0; i < LINE6FBV_NUM_LED_AND_SWITCH; i++){
		if (mLedAndSwitch[i].key == inCC){
			retVal = i;
			i = LINE6FBV_NUM_LED_AND_SWITCH; // exit loop
		}
	}
	return retVal;

};

void Line6Fbv::setLedOnOff(byte inLed, byte inOnOff) {

	// Find the Switch in the array and set the value
	mLedAndSwitch[inLed].flash = 0;
	if (inOnOff){
		mLedAndSwitch[inLed].setOn = 1;
		mLedAndSwitch[inLed].setOff = 0;
	}
	else{
		mLedAndSwitch[inLed].setOff = 1;
		mLedAndSwitch[inLed].setOn = 0;
	}

}

void Line6Fbv::setHoldTime(byte inBtn, unsigned int inHoldTime){
	mLedAndSwitch[inBtn].holdTime = inHoldTime;
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

	mLedAndSwitch[inLed].flash = 1;
	mLedAndSwitch[inLed].isOn = 0;
	mLedAndSwitch[inLed].lastMillis = 0;
	if (inDelayTime > inOnTime){
		mLedAndSwitch[inLed].offTime = inDelayTime - inOnTime; // ToDo intervals < 50 ms
		mLedAndSwitch[inLed].onTime = inOnTime;
	}
	else{
		mLedAndSwitch[inLed].offTime = inDelayTime / 2;
		mLedAndSwitch[inLed].onTime = inDelayTime / 2;
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
			//Serial.print("LED On  : ");
			//Serial.println(mLedAndSwitch[i].key, HEX);
		}
		else if (mLedAndSwitch[i].setOff){
			//Serial.print("LED Off  : ");
			//Serial.println(mLedAndSwitch[i].key, HEX);
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
			//Serial.print("LED flash  : ");
			//Serial.println(mLedAndSwitch[i].key, HEX);
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
	if (!mDisplay.flash){
		mDisplay.isShown = 1;
		if (mDisplayDataChanged){
			sendDisplayData(mDisplay);
			mDisplayDataChanged = 0;
		}
	}
	else{
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
	static unsigned long last_connection_check = 0;
	static bool connected;

	if (connected) {
		if (millis() - last_connection_check > LINE6FBV_CONNECTION_LOST_TIME){
			connected = false;
			if (mCbDisconnected)
				mCbDisconnected();
		}
	}



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
					connected = true;   // connection established
					last_connection_check = millis();
					if (mBytesExpected == 4 && mDataBytes[2] == 0x30) { // Heartbeat
						requestBoardType(); 
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

								mCbKeyReleased(mGetLedInArray(mDataBytes[3]), mStopHold(mDataBytes[3]));
							}

							if (mDataBytes[4] == 0x01 && mCbKeyPressed) {
								mStartHold(mDataBytes[3]);
								mCbKeyPressed(mGetLedInArray(mDataBytes[3]));
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


void Line6Fbv::setDisplayTitle(char* inTitle){

	char title[16];

	mDisplayDataChanged = 1;

	strncpy(title, inTitle, 16);

	for (int i = 0; i < 16; i++){
		mDisplay.title[i] = title[i];
	}
}
void Line6Fbv::setDisplayDigit(int inNum, char inDigit){
	mDisplayDataChanged = 1;

	if (inNum < 3)
		mDisplay.numDigits[inNum] = inDigit;
	else
		mDisplay.noteDigit = inDigit;

}

void Line6Fbv::setDisplayDigits(char* inDigits){

	char digits[4];

	mDisplayDataChanged = 1;

	strncpy(digits, inDigits, 4);
	

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

	mDisplayDataChanged = 1;

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
	mDisplayDataChanged = 1;
	mDisplay.flat = inOnOff;
}

void Line6Fbv::setDisplayFlash(int inOnTime, int inOffTime){
	if (inOnTime != 0){
	mDisplay.flash = 1;
	mDisplay.onTime = inOnTime;
	mDisplay.offTime = inOffTime;
	mDisplay.lastMillis = 0;
	}
	else{
		mDisplay.flash = 0;
		}
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
	uint8_t i = mGetLedInArray(inKey);

	mLedAndSwitch[i].isHeld = 0;
	mLedAndSwitch[i].isPressed = 1;
	mLedAndSwitch[i].lastPressTime = millis();
};


// called when key is released, returns hold state, so the hold information needs not to be stored in the calling application
byte Line6Fbv::mStopHold(byte inKey){

	byte retVal = 0;
	uint8_t i = mGetLedInArray(inKey);

	// Find the Switch in the array and set the value
			retVal = mLedAndSwitch[i].isHeld;
			mLedAndSwitch[i].isHeld = 0;
			mLedAndSwitch[i].isPressed = 0;
	return retVal;
};




// check if hold time is elapsed while key is pressed
void Line6Fbv::mCheckHold(){

	unsigned long currentMillis = millis();
	for (int i = 0; i < LINE6FBV_NUM_LED_AND_SWITCH; i++){
		if (mLedAndSwitch[i].holdTime){
			if (mLedAndSwitch[i].isPressed && !mLedAndSwitch[i].isHeld){
				if (currentMillis - mLedAndSwitch[i].lastPressTime >= mLedAndSwitch[i].holdTime) {
					if (mCbKeyHeld){
						mLedAndSwitch[i].isHeld = 1;
						mCbKeyHeld(mGetLedInArray(mLedAndSwitch[i].key));
					}
				}
			}
		}
	}

};
