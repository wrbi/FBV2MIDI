/*!
	*  @file       VoxAd60Vt.cpp
	*  Project     Arduino Line6 FBV Longboard to MIDI Library
	*  @brief      Line6 FBV Library for the Arduino
	*  @version    0.2
	*  @author     Joachim Wrba
	*  @date       09/08/15
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
#include "VoxAd60Vt.h"


VoxAd60Vt::VoxAd60Vt() {

	mCbStomp = 0;
	mCbDelayTime = 0;
	mCbTunerValue = 0;
	mCbTunerOnOff = 0;
	mCbPgmChanged = 0;
	mCbReset = 0;

	mDataBytes[0] = 0;
	mByteCount = 0;
	mBytesExpected = 0;
    

}

void VoxAd60Vt::begin(HardwareSerial * inSerial){
	mSerial = inSerial;
	mSerial->begin(32150);	
}
void VoxAd60Vt::setHandleStomp(FunctTypeCbStomp* cb) {
	mCbStomp = cb;
}

void VoxAd60Vt::setHandleTunerValue(FunctTypeCbTunerValue* cb) {
	mCbTunerValue = cb;
}
void VoxAd60Vt::setHandleTunerOnOff(FunctTypeCbTunerOnOff* cb) {
	mCbTunerOnOff = cb;
}

void VoxAd60Vt::setHandleTunerSilent(FunctTypeCbTunerSilent* cb) {
	mCbTunerSilent = cb;
}

void VoxAd60Vt::setHandleDelayTime(FunctTypeCbDelayTime* cb) {
	mCbDelayTime = cb;
}

void VoxAd60Vt::setHandlePgmChanged(FunctTypeCbPgmChanged* cb) {
	mCbPgmChanged = cb;
}

void VoxAd60Vt::setHandleReset(FunctTypeCbReset* cb) {
	mCbReset = cb;
}

void VoxAd60Vt::read() {

	byte inByte;

	byte stompPdl;
	byte stompMod;
	byte stompDly;
	byte stompRev;
	int delayTime;

	if (mSerial->available() > 0) {
		while (mSerial->available() > 0) {
			inByte = mSerial->read();
			if (mByteCount == 0){
				switch (inByte){
				case 0xBF: // only the last one in reset sequence is taken into account   
					mDataBytes[0] = inByte;
					mByteCount = 1;
					mBytesExpected = 3;
					break;
				case 0xC0: // Pgm Change
					mDataBytes[0] = inByte;
					mByteCount = 1;
					mBytesExpected = 2;
					break;
				case 0xB0: // Stomp Status
					mDataBytes[0] = inByte;
					mByteCount = 1;
					mBytesExpected = 3;
					break;
				case 0xA0: // Tuner 
					mDataBytes[0] = inByte;
					mByteCount = 1;
					mBytesExpected = 3;
					break;
				case 0xD0: // Tuner silent
					mDataBytes[0] = inByte;
					mByteCount = 1;
					mBytesExpected = 2;
					break;
				case 0xE0: // Delay Time
					mDataBytes[0] = inByte;
					mByteCount = 1;
					mBytesExpected = 3;
					break;
				}
			}
			else{
				mDataBytes[mByteCount] = inByte;
				mByteCount++;
			}
			if (mByteCount) {
				if (mByteCount == mBytesExpected) {
					switch (mDataBytes[0]){
					case 0xBF:
						if (mDataBytes[1] == 0x79){ // only the last one in reset sequence is taken into account   
							if (mCbReset) {
								mCbReset();
							}
						}
						break;
					case 0xC0: // Pgm Change
						if (mCbPgmChanged) {
							mCbPgmChanged(mDataBytes[1]);
						}
						break;
					case 0xB0: // Stomp Status
						if (mDataBytes[1] == 0x5F){
							if (mCbStomp) {
								stompPdl = (mDataBytes[2] & 0x01);
								stompMod = (mDataBytes[2] & 0x02) / 2;
								stompDly = (mDataBytes[2] & 0x04) / 4;
								stompRev = (mDataBytes[2] & 0x08) / 8;
								mCbStomp(stompPdl, stompMod, stompDly, stompRev);
							}
						}
						break;
					case 0xA0: // Tuner 
						if (mDataBytes[1] == 0x00){
							if (mCbTunerOnOff) {
								mCbTunerOnOff(mDataBytes[2]);
							}
						}
						else{
							if (mCbTunerValue) {
								mCbTunerValue(mDataBytes[1], mDataBytes[2]);
							}
						}
						break;
					case 0xD0: // Tuner silent
						if (mDataBytes[1] == 0x7F){
							if (mCbTunerSilent) {
								mCbTunerSilent();
							}
						}
						break;

					case 0xE0: // Delay Time
						if (mCbDelayTime) {
							delayTime = mDataBytes[1] + (mDataBytes[2] * 128);
							mCbDelayTime(delayTime);
						}
						break;

					}

					// reset values 
					mByteCount = 0;
					mBytesExpected = 0;
				}
			}
		}
	}
}


void VoxAd60Vt::sendReset() {
	// All Notes off; All Sound Off; Reset Cntrl on all 16 Channels
	// would look nicer with ( for int i = 1;... :-) 
	
for (byte b = 0xB0; b <=0xBF; b++){
	mSerial->write(b);
	mSerial->write(0x7B);
	mSerial->write(0x00);
}
		
for (byte b = 0xB0; b <=0xBF; b++){
	mSerial->write(b);
	mSerial->write(0x78);
	mSerial->write(0x00);
}
	
for (byte b = 0xB0; b <=0xBF; b++){
	mSerial->write(b);
	mSerial->write(0x79);
	mSerial->write(0x00);
}
	

}
void VoxAd60Vt::sendPgmChange(byte inPgmNum){

	mSerial->write(0xB0); // Bank MSB   there is only one Bank used for 32 Programs
	mSerial->write(0x00); // Bank MSB   there is only one Bank used for 32 Programs
	mSerial->write(0x00); // Bank MSB   there is only one Bank used for 32 Programs
	mSerial->write(0xB0); // Bank LSB
	mSerial->write(0x20); // Bank LSB
	mSerial->write(0x00); // Bank LSB
	mSerial->write(0xC0);     // Pgm Change
	mSerial->write(inPgmNum);


}
void VoxAd60Vt::sendCtlChange(byte inCtl, byte inValue){

	mSerial->write(0xB0);
	mSerial->write(inCtl);
	mSerial->write(inValue);

}
void VoxAd60Vt::sendDelayTime(int inMs){
	byte b1;
	byte b2;
	b1 = inMs % 128;
	b2 = inMs / 128;
	mSerial->write(0xE0);
	mSerial->write(b1);
	mSerial->write(b2);



}

void VoxAd60Vt::switchStompBoxes(byte inPdl, byte inMod, byte inDly, byte inRev){

	byte stompBoxes = 0x70;

	if (inPdl){
		stompBoxes += 1;
	}
	if (inMod){
		stompBoxes += 2;
	}
	if (inDly){
		stompBoxes += 4;
	}
	if (inRev){
		stompBoxes += 8;
	}
	mSerial->write(0xB0);
	mSerial->write(0x5F);
	mSerial->write(stompBoxes);
}

void VoxAd60Vt::switchTuner(byte inOnOff, byte inSilent){

	mSerial->write(0xA0);
	mSerial->write(0x00);
	if (inOnOff){
		mSerial->write(0x7F);
		if (inSilent){
			mSerial->write(0xD0);
			mSerial->write(0x7F);
		}
	}
	else{
		mSerial->write(0x00);
	}
}

