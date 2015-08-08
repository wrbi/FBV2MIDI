/*!
	*  @file       VoxAd60Vt.h
	*  Project     Arduino Line6 FBV Longboard to MIDI Library
	*  @brief      Line6 FBV Library for the Arduino
	*  @version    0.1
	*  @author     Joachim Wrba
	*  @date       17/07/15
	*  @license    GPL v3.0
	*
	*  This Program is free software: you can redistribute it and/or modify
	*  it under the terms of the GNU General Public License as published by
	*  the Free Software Foundation, either version 3 of the License, or
	*  (at your option) any later version.
	*
	*  This Program is distributed in the hope that it will be useful,
	*  but WITHOUT ANY WARRANTY; without even the implied warranty of
	*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	*  GNU General Public License for more details.
	*
	*  You should have received a copy of the GNU General Public License
	*  along with this Program.  If not, see <http://www.gnu.org/licenses/>.
	*/


#ifndef VOXAD60VT_H
#define VOXAD60VT_H

#include <Arduino.h>

const byte VOXAD60VT_VOL = 0x0B;
const byte VOXAD60VT_WAH = 0x01;

class VoxAd60Vt {
public:

	// Definitions for callback functions
	typedef void FunctTypeCbStomp(byte, byte, byte, byte);
	typedef void FunctTypeCbDelayTime(int);
	typedef void FunctTypeCbTunerValue(byte, byte);
	typedef void FunctTypeCbTunerOnOff(byte);
	typedef void FunctTypeCbTunerSilent();
	typedef void FunctTypeCbPgmChanged(byte);
	typedef void FunctTypeCbReset();

	// just the constructor
	VoxAd60Vt();

	// assign the Serial to be used (Serial1, Serial2, Serial3, Serial)
	void begin(HardwareSerial* inSerial);

	// interpret incoming bytes and fire callback functions
	void read();

	// stompBox onOff status has to be sent in one Byte
	void switchStompBoxes(byte inPdl, byte inMod, byte inDly, byte inRev);

	// Switch tuner on/off or silent
	void switchTuner(byte inOnOff, byte inSilent);

	// send program change values 0 - 31
	void sendPgmChange(byte inPgmNum);

	// send pedal values. inCntl = VOXAD60VT_VOL or VOXAD60VT_WAH
	void sendCtlChange(byte inCtl, byte inValue);

	// send Delay Time. Tap Events must be converted to milliseconds
	void sendDelayTime(int inMs);

	// this tells the AD60VT that a pedal (normally the VC-12) is connected
	void sendReset();

	// set a callback Function for StompBox onOff status
	void setHandleStomp(FunctTypeCbStomp* cb);

	// set a callback Function for Delay-time information
	// can be used to flash an LED
	void setHandleDelayTime(FunctTypeCbDelayTime* cb);

	// set a callback Function for Tuner information
	void setHandleTunerValue(FunctTypeCbTunerValue* cb);

	// set a callback Function for 
	void setHandleTunerOnOff(FunctTypeCbTunerOnOff* cb);

	// set a callback Function for 
	void setHandleTunerSilent(FunctTypeCbTunerSilent* cb);

	// set a callback Function for Program Changes
	void setHandlePgmChanged(FunctTypeCbPgmChanged* cb);


	// set a callback Function for
	void setHandleReset(FunctTypeCbReset* cb);


private:

	FunctTypeCbStomp*        mCbStomp;
	FunctTypeCbDelayTime*    mCbDelayTime;
	FunctTypeCbTunerValue*   mCbTunerValue;
	FunctTypeCbTunerOnOff*   mCbTunerOnOff;
	FunctTypeCbTunerSilent*  mCbTunerSilent;
	FunctTypeCbPgmChanged*   mCbPgmChanged;
	FunctTypeCbReset*        mCbReset;


	HardwareSerial* mSerial;
	byte mDataBytes[3];
	int mByteCount;
	int mBytesExpected;

};
#endif
