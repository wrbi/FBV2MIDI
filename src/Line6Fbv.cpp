/*!
	*  @file       Line6Fbv.cpp
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
#include "Line6Fbv.h"
//#include <Arduino.h>

Line6Fbv::Line6Fbv() {

	
	
	mCbKeyPressed = 0;
	mCbKeyReleased = 0;
	mCbCtrlChanged = 0;
	mCbHeartbeat = 0;

	mDataBytes[0] = 0;
	mByteCount = 0;
	mBytesExpected = 0;

	mLedValues[0].key = LINE6FBV_FXLOOP;
	mLedValues[1].key = LINE6FBV_STOMP1;
	mLedValues[2].key = LINE6FBV_STOMP2;
	mLedValues[3].key = LINE6FBV_STOMP3;
	mLedValues[4].key = LINE6FBV_AMP1;
	mLedValues[5].key = LINE6FBV_AMP2;
	mLedValues[6].key = LINE6FBV_REVERB;
	mLedValues[7].key = LINE6FBV_PITCH;
	mLedValues[8].key = LINE6FBV_MOD;
	mLedValues[9].key = LINE6FBV_DELAY;
	mLedValues[10].key = LINE6FBV_TAP;
	mLedValues[11].key = LINE6FBV_UP;
	mLedValues[12].key = LINE6FBV_DOWN;
	mLedValues[13].key = LINE6FBV_CHANNELA;
	mLedValues[14].key = LINE6FBV_CHANNELB;
	mLedValues[15].key = LINE6FBV_CHANNELC;
	mLedValues[16].key = LINE6FBV_CHANNELD;
	mLedValues[17].key = LINE6FBV_FAVORITE;
	mLedValues[18].key = LINE6FBV_PDL1_GRN;
	mLedValues[19].key = LINE6FBV_PDL1_RED;
	mLedValues[20].key = LINE6FBV_PDL2_GRN;
	mLedValues[21].key = LINE6FBV_PDL2_RED;
	mLedValues[22].key = LINE6FBV_DISPLAY;

	for (int i = 0; i < LINE6FBV_NUM_LED; i++){
		mLedValues[i].isOn = 0;
		mLedValues[i].setOn = 0;
		mLedValues[i].setOff = 0;
		mLedValues[i].flash = 0;
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

void Line6Fbv::setHandleCtrlChanged(FunctTypeCbCtrlChanged* cb) {
	mCbCtrlChanged = cb;
}

void Line6Fbv::setHandleHeartbeat(FunctTypeCbHeartbeat* cb) {
	mCbHeartbeat = cb;
}


void Line6Fbv::setLedOnOff(byte inLed, byte inOnOff) {

	// Find the Switch in the array and set the value
	for (int i = 0; i < LINE6FBV_NUM_LED; i++){
		if (mLedValues[i].key == inLed){
			mLedValues[i].flash = 0;
			if (inOnOff){
				mLedValues[i].setOn = 1;
				mLedValues[i].setOff = 0;
			}
			else{
				mLedValues[i].setOff = 1;
				mLedValues[i].setOn = 0;
			}

			i = LINE6FBV_NUM_LED; // exit loop
		}
	}

}

void Line6Fbv::setLedFlash(byte inLed, int inDelayTime) {

	// Find the Switch in the array and set the value
	for (int i = 0; i < LINE6FBV_NUM_LED; i++){
		if (mLedValues[i].key == inLed){
			mLedValues[i].flash = 1;
			mLedValues[i].isOn = 0;
			mLedValues[i].lastMillis = 0;
			if (inDelayTime > LINE6FBV_FLASH_TIME){
				mLedValues[i].offTime = inDelayTime - LINE6FBV_FLASH_TIME; // ToDo intervals < 50 ms
				mLedValues[i].onTime = LINE6FBV_FLASH_TIME;
			}
			else{
				mLedValues[i].offTime = inDelayTime / 2;
				mLedValues[i].onTime = inDelayTime / 2;
			}
			i = LINE6FBV_NUM_LED; // exit loop
		}
	}

}
void Line6Fbv::updateUI(){

	unsigned long currentMillis = millis();


	// LEDs
	for (int i = 0; i < LINE6FBV_NUM_LED; i++){
		if (mLedValues[i].setOn){
			mLedValues[i].setOn = 0;
			mLedValues[i].isOn = 1;
			mLedValues[i].flash = 0;
			mSerial->write(0xF0);
			mSerial->write(0x03);
			mSerial->write(0x04);
			mSerial->write(mLedValues[i].key);
			mSerial->write(0x01);
			//			Serial.print("LED On  : ");
			//			Serial.println(mLedValues[i].key, HEX);
		}
		else 		if (mLedValues[i].setOff){
			//			Serial.print("LED Off  : ");
			//			Serial.println(mLedValues[i].key, HEX);
			mLedValues[i].setOff = 0;
			mLedValues[i].isOn = 0;
			mLedValues[i].flash = 0;
			mSerial->write(0xF0);
			mSerial->write(0x03);
			mSerial->write(0x04);
			mSerial->write(mLedValues[i].key);
			mSerial->write(0x00);
		}
		else if (mLedValues[i].flash){
			//			Serial.print("LED flash  : ");
			//			Serial.println(mLedValues[i].key, HEX);
			if (currentMillis - mLedValues[i].lastMillis >= mLedValues[i].waitTime) {
				mLedValues[i].lastMillis = currentMillis;
				if (!mLedValues[i].isOn)
					mLedValues[i].waitTime = mLedValues[i].onTime;
				else
					mLedValues[i].waitTime = mLedValues[i].offTime;
				mLedValues[i].isOn = !mLedValues[i].isOn;
				mSerial->write(0xF0);
				mSerial->write(0x03);
				mSerial->write(0x04);
				mSerial->write(mLedValues[i].key);
				mSerial->write(mLedValues[i].isOn);
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
								mCbKeyReleased(mDataBytes[3]);
							}

							if (mDataBytes[4] == 0x01 && mCbKeyPressed) {
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

void Line6Fbv::setDisplayFlat(byte inOnOff){
	mDisplay.flat = inOnOff;
}

void Line6Fbv::setDisplayFlash(int inOnTime, int inOffTime){
	mDisplay.flash = 1;
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