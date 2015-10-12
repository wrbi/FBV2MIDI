/*!
*  @file       KPA.h
*  Project     Arduino Library for MIDI Communication with Kemper Profiling Amplifier
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

#ifndef KPA_H
	#define KPA_H
	#include <Arduino.h>
	
	// Define Program Change and Control Change for MIDI Channel 1
	const byte KPA_MIDI_CMD_CC = 0xB0;  // MIDI Channel 1
	const byte KPA_MIDI_CMD_PC = 0xC0;  // MIDI Channel 1
	
	const int KPA_SYSEX_SIZE = 128;
	
	// continuous controller numbers
const byte KPA_CC_WAH = 0x01;
const byte KPA_CC_VOL = 0x07;
const byte KPA_CC_PITCH = 0x04;
const byte KPA_CC_GAIN = 0x48;

const byte KPA_CC_TAP = 0x1E;

const byte KPA_CC_FX_A =  0x11;
const byte KPA_CC_FX_B =  0x12;
const byte KPA_CC_FX_C =  0x13;
const byte KPA_CC_FX_D =  0x14;
const byte KPA_CC_FX_X =  0x16;
const byte KPA_CC_FX_MOD =  0x18;
const byte KPA_CC_FX_DLY = 0x1B;  // 27 keep tail (26 cut tail)
const byte KPA_CC_FX_REV =  0x1D;  // 29 keep tail (28 cut tail)
	

// address pages and parameter Numbers
const byte KPA_ADDR_RIG = 0x04; // 50

const byte KPA_ADDR_FX_A = 0x32; // 50
const byte KPA_ADDR_FX_B = 0x33; // 51
const byte KPA_ADDR_FX_C = 0x34; // 52
const byte KPA_ADDR_FX_D = 0x35; // 53
const byte KPA_ADDR_FX_X = 0x38; // 56
const byte KPA_ADDR_FX_MOD = 0x3A; // 58
const byte KPA_ADDR_FX_DLY = 0x4A; // 74
const byte KPA_ADDR_FX_REV = 0x4B; // 75

const byte KPA_PARAM_TYPE_SINGLE = 0x41;
const byte KPA_PARAM_TYPE_STRING = 0x43;
const byte KPA_PARAM_NUM_FX_ON = 0x03;
const byte KPA_PARAM_NUM_FX_ON2 = 0x02; // Delay + Reverb
const byte KPA_PARAM_NUM_FX_TYPE = 0x00;
const byte KPA_PARAM_NUM_RIG_TEMPO = 0x00;
const byte KPA_PARAM_NUM_RIG_NAME = 0x01;

// Looper Commands
const byte KPA_LOOP_REC_PLAY = 0x58;
const byte KPA_LOOP_STOP_ERASE = 0x59;
const byte KPA_LOOP_TRIGGER = 0x5A;
const byte KPA_LOOP_REVERSE = 0x5B;
const byte KPA_LOOP_HALFTIME = 0x5C;
const byte KPA_LOOP_UNDO = 0x5D;

	
	
	class KPA
	{
		
		
		public:
		
		// Callback Functions 
		typedef void FunctTypeCbCtlChanged(byte, byte);
		typedef void FunctTypeCbPgmChanged(byte);
		typedef void FunctTypeCbSysEx(byte* , unsigned);
		typedef void FunctTypeCbParamSingle(byte, byte,byte,byte);
		typedef void FunctTypeCbParamString(byte, byte, byte*,unsigned);
		typedef void FunctTypeCbParamStringX(byte, byte, byte, byte, byte, byte*, unsigned int);
		
		KPA();
		void begin(HardwareSerial * InSerial);
		void read();
        void sendPgmChange(unsigned inPgmNum);  // send Program Chage incl. Bank Change
        
		void sendCtlChange(byte inCtlNum, byte inCtlVal); // Expression Pedals
        
		void sendLooperCmd(byte inCmd, byte inKeyPress);  // Control the LOOPER
        void sendParamRequest(byte inRequType, byte inAddrPage, byte inParamNum); // Ask for Values to display    
		void sendParamRequestStringX(byte inXsb1, byte inXsb2, byte inXsb3, byte inXsb4, byte inXsb5); // Ask for Values to display    
		void sendHeartbeat(); // tell KPA that midi controller is connected. needed for bidirectional communication
        void sendSysEx(byte *inSysEx, unsigned inBytes);  // send MIDI Sysex       
        
		void setHandleCtlChanged(FunctTypeCbCtlChanged* cb);  
		void setHandlePgmChanged(FunctTypeCbPgmChanged* cb);
		void setHandleSysEx(FunctTypeCbSysEx* cb);
		void setHandleParamSingle(FunctTypeCbParamSingle* cb);
		void setHandleParamString(FunctTypeCbParamString* cb);
		void setHandleParamStringX(FunctTypeCbParamStringX* cb);
		
		private:
		HardwareSerial * mSerial = 0;
		byte mDataBytes[KPA_SYSEX_SIZE];
		int mByteCount = 0;
		int mBytesExpected = 0;
//		unsigned long mConnStrSent = 0;
		
		FunctTypeCbCtlChanged * mCbCtlChanged = 0;
		FunctTypeCbPgmChanged * mCbPgmChanged = 0;
		FunctTypeCbSysEx * mCbSysEx = 0;
		FunctTypeCbParamSingle * mCbParamSingle = 0;
		FunctTypeCbParamString * mCbParamString = 0;
		FunctTypeCbParamStringX * mCbParamStringX = 0;
		
		void processSysEx(void); 
		
		
	};
	
#endif