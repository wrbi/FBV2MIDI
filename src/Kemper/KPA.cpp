/*!
*  @file       KPA.cpp
*  Project     Arduino Library for Kemper Profiling Amplifier
*  @brief      KPA Library for the Arduino
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

#include "KPA.h"


KPA::KPA()
{

}

void KPA::begin(HardwareSerial * inSerial) {
	mSerial = inSerial;
	mSerial->begin(31250);

}

void KPA::setHandleCtlChanged(FunctTypeCbCtlChanged* cb)
{
	mCbCtlChanged = cb;
}

void KPA::setHandlePgmChanged(FunctTypeCbPgmChanged* cb)
{
	mCbPgmChanged = cb;
}

void KPA::setHandleSysEx(FunctTypeCbSysEx* cb)
{
	mCbSysEx = cb;
}

void KPA::setHandleParamSingle(FunctTypeCbParamSingle* cb)
{
	mCbParamSingle = cb;
}

void KPA::setHandleParamString(FunctTypeCbParamString* cb)
{
	mCbParamString = cb;
}


void KPA::sendHeartbeat(void){
	byte cnnStr[] = { 0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, 0x7E, 0x00, 0x40, 0x01, 0x36, 0x04, 0xF7 };

		sendSysEx(cnnStr, 13);

}

void KPA::sendPgmChange(unsigned inPgmNum){

	byte midiBankNum = inPgmNum / 127;
	byte midiPgmNum = inPgmNum % 127;

	sendCtlChange(0x00, 0x00);
	sendCtlChange(0x20, midiBankNum);
	mSerial->write(KPA_MIDI_CMD_PC);
	mSerial->write(midiPgmNum);

}
void KPA::sendCtlChange(byte inCtlNum, byte inCtlVal){
	mSerial->write(KPA_MIDI_CMD_CC);
	mSerial->write(inCtlNum);
	mSerial->write(inCtlVal);


}
void KPA::sendSysEx(byte *inSysEx, unsigned inBytes){
	for (size_t i; i < inBytes; i++){
		mSerial->write(inSysEx[i]);
	}
}

void KPA::sendLooperCmd(byte inCmd, byte inKeyPress){
	sendCtlChange(0x63, 0x7D);
	sendCtlChange(0x62, inCmd);
	sendCtlChange(0x06, 0x00);
	sendCtlChange(0x26, inKeyPress);
}
void KPA::sendParamRequest(byte inReqType, byte inAddrPage, byte inParamNum){

	byte request[] = { 0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, 0 /*Request Type*/, 0x00, 0 /*Addr Page*/, 0 /*Parm Number*/, 0xF7 };

	request[6] = inReqType;
	request[8] = inAddrPage;
	request[9] = inParamNum;

	sendSysEx(request, 11);
}






void KPA::read(){

	byte inByte;

	if (mSerial->available() == 0)
		return;

	while (mSerial->available() > 0) {
		inByte = mSerial->read();

		if (inByte >= 0x80){
			switch (inByte){
			case KPA_MIDI_CMD_CC:
				mBytesExpected = 3;
				mByteCount = 1;
				mDataBytes[0] = inByte;
				break;
			case KPA_MIDI_CMD_PC:
				mBytesExpected = 2;
				mByteCount = 1;
				mDataBytes[0] = inByte;
				break;
			case 0xF0:
				//Serial.println("Start Sysex");
				mBytesExpected = KPA_SYSEX_SIZE;
				mByteCount = 1;
				mDataBytes[0] = inByte;
				break;
			case 0xF7:
				mDataBytes[mByteCount] = inByte;
				mByteCount++;
				mBytesExpected = mByteCount;
				/*Serial.print("End Sysex ");
				Serial.print(mBytesExpected);
				Serial.print(" Count ");
				Serial.println(mByteCount);*/
			}
		}
		else{
			if (mBytesExpected){
				mDataBytes[mByteCount] = inByte;
				mByteCount++;
			}
		}

		if (mByteCount > mBytesExpected) // something went wrong
		{
			Serial.print("Hilfeeeeeee ");
			Serial.print(mByteCount);
			Serial.print(" - ");
			Serial.print(mBytesExpected);
			Serial.println(" ");
			mByteCount = 0;
			mBytesExpected = 0;
		}

		if (mBytesExpected){
			/*Serial.print("Bytes exp ");
			Serial.print(mBytesExpected);
			Serial.print(" Count ");
			Serial.println(mByteCount);*/
			if (mBytesExpected == mByteCount){
			//	Serial.println("passt");
				switch (mDataBytes[0])
				{
				case KPA_MIDI_CMD_CC:
					if (mCbCtlChanged)
						mCbCtlChanged(mDataBytes[1], mDataBytes[2]);
					break;
				case KPA_MIDI_CMD_PC:
					if (mCbPgmChanged)
						mCbPgmChanged(mDataBytes[1]);
					break;
				case 0xF0:
					processSysEx();
					break;
				}
				mByteCount = 0;
				mBytesExpected = 0;
			}
		}
	}
}
			
			


void KPA::processSysEx(){

	byte requestType = 0;
	byte addressPage = 0;
	byte paramNumber = 0;
	byte msb = 0;
	byte lsb = 0;
	byte stringValue[KPA_SYSEX_SIZE];
	int stringSize = 0;
	bool cbFound = false;

	if (mByteCount >= 13){
		requestType = mDataBytes[6];
		addressPage = mDataBytes[8];
		paramNumber = mDataBytes[9];
		msb = mDataBytes[10];
		lsb = mDataBytes[11];
	}
	switch (requestType) {
	case 0x01:  // answer to single parameter request
		if (mCbParamSingle) {
			mCbParamSingle(addressPage, paramNumber, msb, lsb);
			cbFound = true;
		}
		break;
	case 0x03:  // answer to single parameter request
//	case 0x07:
		if (mCbParamString) {
			for (size_t i = 0; i < KPA_SYSEX_SIZE; i++){
				stringValue[i] = mDataBytes[i + 10];
				if (stringValue[i] == 0x00)
					i = KPA_SYSEX_SIZE;
				else
					stringSize = i + 1;
			}
			mCbParamString(addressPage, paramNumber, stringValue, stringSize);
			cbFound = true;
		}


	}

	// if no special callback is found => call universal SysEx callback
	if (!cbFound){
		if (mCbSysEx)
			mCbSysEx(mDataBytes, mByteCount);
	}
}
