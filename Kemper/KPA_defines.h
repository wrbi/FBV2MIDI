/*!
*  @file       Kemper.h
*  Project     Arduino Library for MIDI Communication with Kemper Profiling Amplifier
*  @brief      Kemper Library for the Arduino
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
	

#define KPA_SYSEX_FN_REQUEST_PARAM            0x41
#define KPA_SYSEX_FN_REQUEST_M_PARAM          0x42
#define KPA_SYSEX_FN_REQUEST_STRING           0x43
#define KPA_SYSEX_FN_REQUEST_EXT_PARAM        0x46
#define KPA_SYSEX_FN_REQUEST_EXT_STRING       0x47
	
#define KPA_SYSEX_FN_RETURN_PARAM            0x01
#define KPA_SYSEX_FN_RETURN_M_PARAM          0x02
#define KPA_SYSEX_FN_RETURN_STRING           0x03
#define KPA_SYSEX_FN_RETURN_EXT_PARAM        0x06
#define KPA_SYSEX_FN_RETURN_EXT_STRING       0x07


#define KPA_PARAM_TAP_EVENT			0x3E00	// 7C 00

#define KPA_PARAM_CURR_TUNING		0x3E0F	// 7C 0F
#define KPA_PARAM_CURR_NOTE			0x3ED4	// 7D 54

#define KPA_PARAM_TUNER_STATE       0x3FFE
#define KPA_PARAM_MODE              0x3FFD

#define KPA_MODE_BROWSE				0x00
#define KPA_MODE_PERFORM			0x01

#define KPA_PARAM_STOMP_A_TYPE				0x1900
#define KPA_PARAM_STOMP_A_STATE				0x1903
#define KPA_PARAM_STOMP_B_TYPE				0x1980
#define KPA_PARAM_STOMP_B_STATE				0x1983
#define KPA_PARAM_STOMP_C_TYPE				0x1A00
#define KPA_PARAM_STOMP_C_STATE				0x1A03
#define KPA_PARAM_STOMP_D_TYPE				0x1A80
#define KPA_PARAM_STOMP_D_STATE				0x1A83

#define KPA_PARAM_STOMP_X_TYPE				0x1C00
#define KPA_PARAM_STOMP_X_STATE				0x1C03
#define KPA_PARAM_STOMP_MOD_TYPE			0x1D00
#define KPA_PARAM_STOMP_MOD_STATE			0x1D03
#define KPA_PARAM_DELAY_TYPE 				0x2500
#define KPA_PARAM_DELAY_STATE				0x2502
#define KPA_PARAM_REVERB_TYPE				0x2580
#define KPA_PARAM_REVERB_STATE				0x2582

#define KPA_PARAM_GAIN_VALUE                0x0504  

#define KPA_PARAM_LOOPER_STATE              0x3E2A

#define KPA_STRING_ID_RIG_NAME				0x0001
#define KPA_STRING_ID_PERF_NAME				0x4000
#define KPA_STRING_ID_PERF_NAME_PREVIEW  	0x4010
#define KPA_STRING_ID_SLOT1_NAME			0x4001
#define KPA_STRING_ID_SLOT2_NAME			0x4002
#define KPA_STRING_ID_SLOT3_NAME			0x4003
#define KPA_STRING_ID_SLOT4_NAME			0x4004
#define KPA_STRING_ID_SLOT5_NAME			0x4005 



// continuous controller numbers
#define KPA_CC_WAH    0x01
#define KPA_CC_VOL    0x07
#define KPA_CC_PITCH  0x04
#define KPA_CC_GAIN   0x48

#define KPA_CC_TAP    0x1E

#define KPA_CC_FX_A   0x11
#define KPA_CC_FX_B   0x12
#define KPA_CC_FX_C   0x13
#define KPA_CC_FX_D   0x14
#define KPA_CC_FX_X   0x16
#define KPA_CC_FX_MOD 0x18
#define KPA_CC_FX_DLY 0x1B  // 27 keep tail (26 cut tail)
#define KPA_CC_FX_REV 0x1D  // 29 keep tail (28 cut tail)
	


#define KPA_PARAM_TYPE_SINGLE  0x41
#define KPA_PARAM_TYPE_MULTI  0x42
#define KPA_PARAM_TYPE_STRING  0x43
#define KPA_PARAM_TYPE_EXT_STRING  0x47



#define KPA_LOOPER_CC_START			0x58
#define KPA_LOOPER_CC_STOP			0x59
#define KPA_LOOPER_CC_TRIGGER		0x5A
#define KPA_LOOPER_CC_REVERSE		0x5B
#define KPA_LOOPER_CC_HALFTIME		0x5C
#define KPA_LOOPER_CC_UNDOREDO		0x5D
#define KPA_LOOPER_CC_ERASE			0x5E


#define KPA_MIDI_CHANNEL  1
#define KPA_SYSEX_HEADER_SIZE				7 
#define KPA_SYSEX_FN_ACK					0x7E

	
	