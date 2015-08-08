/*!
 *  @file       Line6Fbv.cpp
 *  Project     Arduino Line6 FBV Longboard to MIDI Library
 *  @brief      Line6 FBV Library for the Arduino
 *  @version    0.1
 *  @author     Joachim Wrba
 *  @date       17/07/15
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


Line6Fbv::Line6Fbv() {

	mCbKeyPressed  = 0;
	mCbKeyReleased = 0;
	mCbCtrlChanged = 0;
	mCbHeartbeat   = 0;

	mDataBytes[0] = 0;
	mByteCount = 0;
	mBytesExpected = 0;

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


void Line6Fbv::setLed( byte inLed, byte inOnOff) {

	mSerial->write(0xF0);
	mSerial->write(0x03);
	mSerial->write(0x04);
	mSerial->write(inLed);
	mSerial->write(inOnOff);

}

void Line6Fbv::read() {

	byte inByte;

	if (mSerial->available() > 0 ) {
		while (mSerial->available() > 0 ) {
			inByte = mSerial->read( );
			switch (mByteCount) {
				case 0:
					if (inByte == 0xF0) {
						mDataBytes[0] = inByte;
						mByteCount = 1;
						mBytesExpected = 0;
					}
					break;
				case 1:
					switch(inByte) {
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
					switch(inByte) {
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
							mCbHeartbeat( );
						}
					}

					if (mBytesExpected == 5) {
						switch(mDataBytes[2]) {
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

