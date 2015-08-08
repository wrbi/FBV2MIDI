/*!
 *  @file       Line6Fbv.h
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

#ifndef LINE6FBV_H
#define LINE6FBV_H

#include <Arduino.h>

// Constants used for different switches ans LEDs
// Switch + LED
const byte LINE6FBV_FXLOOP   = 0x02;
const byte LINE6FBV_STOMP1   = 0x12;
const byte LINE6FBV_STOMP2   = 0x22;
const byte LINE6FBV_STOMP3   = 0x32;
const byte LINE6FBV_AMP1     = 0x01;
const byte LINE6FBV_AMP2     = 0x11;
const byte LINE6FBV_REVERB   = 0x21;
const byte LINE6FBV_PITCH    = 0x31;
const byte LINE6FBV_MOD      = 0x41;
const byte LINE6FBV_DELAY    = 0x51;
const byte LINE6FBV_TAP      = 0x61;
const byte LINE6FBV_DOWN     = 0x00;
const byte LINE6FBV_UP       = 0x10;
const byte LINE6FBV_CHANNELA = 0x20;
const byte LINE6FBV_CHANNELB = 0x30;
const byte LINE6FBV_CHANNELC = 0x40;
const byte LINE6FBV_CHANNELD = 0x50;
const byte LINE6FBV_FAVORITE = 0x60;

// SWITCH Only
const byte LINE6FBV_PDL1_SW  = 0x43;
const byte LINE6FBV_PDL2_SW  = 0x53;

// LED ONLY
const byte LINE6FBV_PDL1_GRN = 0x03;
const byte LINE6FBV_PDL1_RED = 0x13;
const byte LINE6FBV_PDL2_GRN = 0x33;
const byte LINE6FBV_PDL2_RED = 0x23;
const byte LINE6FBV_DISPLAY  = 0x0A;

const byte LINE6FBV_PDL1     = 0x00;
const byte LINE6FBV_PDL2     = 0x01;


class Line6Fbv {
	public:
		Line6Fbv();
		void setLed( byte inLed, byte inOnOff );
		void read();
		void begin(HardwareSerial* inSerial);

		typedef void FunctTypeCbKeyPressed(byte);
		typedef void FunctTypeCbKeyReleased(byte);
		typedef void FunctTypeCbCtrlChanged(byte, byte);
		typedef void FunctTypeCbHeartbeat();


		void setHandleKeyPressed(FunctTypeCbKeyPressed* cb);
		void setHandleKeyReleased(FunctTypeCbKeyReleased* cb);
		void setHandleCtrlChanged(FunctTypeCbCtrlChanged* cb);
		void setHandleHeartbeat(FunctTypeCbHeartbeat* cb);


	private:

		FunctTypeCbKeyPressed*  mCbKeyPressed;
		FunctTypeCbKeyReleased* mCbKeyReleased;
		FunctTypeCbCtrlChanged* mCbCtrlChanged;
		FunctTypeCbHeartbeat*   mCbHeartbeat;


		HardwareSerial * mSerial;
		byte mDataBytes[5];
		int mByteCount;
		int mBytesExpected;

};
#endif
