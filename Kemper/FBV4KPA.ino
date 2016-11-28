/*!
*  @file       FBV4KPA.ino  
*  Project     Control the Kemper Profiling Amplifier with a Line6 FBV Longboard
*  @brief      Control KPA with FBV
*  @version    5.0
*  @author     Joachim Wrba
*  @date       2016.11.28
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
//=========================================================================
#include "Line6Fbv.h"
#include "KPA_defines.h"
#include <MIDI.h>
//using namespace midi;
//============= Serial Ports of the Arduino Mega ==========================
#define SERIAL_FBV Serial1
#define SERIAL_KPA Serial3
//=========================================================================
#define FLASH_TIME 1000
#define HOLD_TIME_SWITCH_LOOPER 1000
#define HOLD_TIME_RESET 5000
// Looper State has to be requested repeatedly, as the actual state
//   is not available immideatelly after switching
#define LOOPER_STATE_REQUEST_INTERVAL 500
#define CONNECTION_INTERVAL 5000
//=========================================================================
//=========================================================================
// Match FBV Switches  
#define SWTCH_BANK_DOWN     LINE6FBV_CHANNELD
#define SWTCH_BANK_UP       LINE6FBV_FAVORITE

#define SWTCH_PRF_SLOT_1    LINE6FBV_DOWN 
#define SWTCH_PRF_SLOT_2    LINE6FBV_UP
#define SWTCH_PRF_SLOT_3    LINE6FBV_CHANNELA
#define SWTCH_PRF_SLOT_4    LINE6FBV_CHANNELB 
#define SWTCH_PRF_SLOT_5    LINE6FBV_CHANNELC

#define SWTCH_FX_SLOT_A     LINE6FBV_FXLOOP
#define SWTCH_FX_SLOT_B     LINE6FBV_STOMP1
#define SWTCH_FX_SLOT_C     LINE6FBV_STOMP2
#define SWTCH_FX_SLOT_D     LINE6FBV_STOMP3

#define SWTCH_FX_SLOT_X     LINE6FBV_AMP1
#define SWTCH_FX_SLOT_MOD   LINE6FBV_AMP2
#define SWTCH_FX_SLOT_DLY   LINE6FBV_REVERB
#define SWTCH_FX_SLOT_REV   LINE6FBV_PITCH

#define SWTCH_TAP           LINE6FBV_TAP
#define SWTCH_SOLO          LINE6FBV_MOD
#define SWTCH_LOOPER        LINE6FBV_DELAY

#define SWTCH_LOOPER_START        LINE6FBV_AMP1
#define SWTCH_LOOPER_STOP         LINE6FBV_AMP2
#define SWTCH_LOOPER_TRIGGER      LINE6FBV_REVERB
#define SWTCH_LOOPER_UNDO         LINE6FBV_PITCH
#define SWTCH_LOOPER_HALFTIME	  0xff  // currently not used, undefined value never sent by FBV
#define SWTCH_LOOPER_REVERSE      0xfe  // currently not used, undefined value never sent by FBV

#define SWTCH_RESET        LINE6FBV_FAVORITE





//=========================================================================
//=========================================================================
//=========================================================================

#define NAME_LENGTH 32 // Perfomance, Performance Slot and Rig name

#define CC_BANK_MSB  0x00 
#define CC_BANK_LSB  0x20


MIDI_CREATE_INSTANCE(HardwareSerial, SERIAL_KPA, kpa);
Line6Fbv fbv = Line6Fbv();

struct SysEx {                          // sysex message container
	char header[5];
	unsigned char fn;
	char id;
	unsigned char data[64];
} sysexBuffer = { { 0x00, 0x20, 0x33, 0x02, 0x7f }, 0, 0, { 0 } };

#define CNN_STATE_WAIT_SENSE         0
#define CNN_STATE_CONNECT            1 
#define CNN_STATE_WAIT_INITIAL_DATA  2
#define CNN_STATE_RUN                3

struct Connection {
	uint8_t  ackReceived;
	uint8_t  senseReceived;
	uint32_t lastAck;
	uint8_t  state;

} connection = { 0, 0, 0, CNN_STATE_WAIT_SENSE };



// positions in the array
#define FX_SLOT_POS_A  0
#define FX_SLOT_POS_B  1
#define FX_SLOT_POS_C  2
#define FX_SLOT_POS_D  3
#define FX_SLOT_POS_X  4
#define FX_SLOT_POS_MOD  5
#define FX_SLOT_POS_DLY  6
#define FX_SLOT_POS_REV  7

struct FxSlot{
	byte fbv;             // corresponding Switch on FBV
	int paramType;
	int paramState;
	byte contCtl;         // Midi CC Number to send
	bool isEnabled;       // Slot is not empty
	bool isInitialOn;     // On at program change
	bool isOn;            // actual status 
	bool received;

};

struct KpaState {
	int tune;                /* Holds the current tune value */
	uint8_t noteNum;             /* Holds the current tuner note */
	uint8_t octave;
	uint8_t mode;                                  // Holds the mode KPA is running (TUNER, BROWSE, PERFORMANCE)
	bool preview;
	uint8_t bankNum;
	uint16_t pgmNum;     // combination Bank + Pgm
	uint8_t actSlot;
	uint8_t actPerformance;
	bool tunerIsOn;
	char performanceSlotNames[5][NAME_LENGTH + 1];
	char rigName[NAME_LENGTH + 1];
	bool looperIsOn;
	uint32_t lastSent;
};

	unsigned long nextLooperStateRequest = 0;

struct KpaState kpaState = {
	0,
	0,
	0,
	0xff,  // mode undefinded, as a change is deeded to set the looper position
	false,
	0,
	0,
	0,
	0,
	false,
	{ { 0x00 } },
	{ 0x00 },
	false,
	0
};

struct FbvPedal{
	//     byte ctlNumOff;
	//     byte ctlNumOn;
	byte ctlNum;
	bool onOff;
	byte actPos;
	byte cmpPos;
	byte ledNumGrn;
	byte ledNumRed;
};


FbvPedal fbvPdls[2];

#define FX_SLOTS 8
FxSlot fxSlots[FX_SLOTS];

bool soloModePostFx = false;

void(*resetFunc) (void) = 0; //declare reset function @ address 0, call to this invalid address results in reatrting the arduino

void initFbvPdlValues(){
	fbvPdls[0].ledNumGrn = LINE6FBV_PDL1_GRN;
	fbvPdls[0].ledNumRed = LINE6FBV_PDL1_RED;
	fbvPdls[0].actPos = 0;
	fbvPdls[0].cmpPos = 0;
	fbvPdls[0].onOff = 0;
	//     fbvPdls[0].ctlNumOff = KPA_CC_WAH;  // maybe used later to assign 2 Values to each pedal
	//     fbvPdls[0].ctlNumOn = KPA_CC_GAIN;
	fbvPdls[0].ctlNum = KPA_CC_WAH;


	fbvPdls[1].ledNumGrn = LINE6FBV_PDL2_GRN;
	fbvPdls[1].ledNumRed = LINE6FBV_PDL2_RED;
	fbvPdls[1].actPos = 127;
	fbvPdls[1].cmpPos = 0;
	fbvPdls[1].onOff = 0;
	//     fbvPdls[1].ctlNumOff = KPA_CC_VOL;
	//     fbvPdls[1].ctlNumOn = KPA_CC_MORPH;
	fbvPdls[1].ctlNum = KPA_CC_VOL;

	setFbvPdlLeds(0);
	setFbvPdlLeds(1);

}

// set Addresspage, CC number, corresponding FBV Switch for each FX slot
void initFxSlots(){

	fxSlots[FX_SLOT_POS_A].fbv = SWTCH_FX_SLOT_A;
	fxSlots[FX_SLOT_POS_A].paramType = KPA_PARAM_STOMP_A_TYPE;
	fxSlots[FX_SLOT_POS_A].paramState = KPA_PARAM_STOMP_A_STATE;
	fxSlots[FX_SLOT_POS_A].contCtl = KPA_CC_FX_A;   // 17

	fxSlots[FX_SLOT_POS_B].fbv = SWTCH_FX_SLOT_B;
	fxSlots[FX_SLOT_POS_B].paramType = KPA_PARAM_STOMP_B_TYPE;
	fxSlots[FX_SLOT_POS_B].paramState = KPA_PARAM_STOMP_B_STATE;
	fxSlots[FX_SLOT_POS_B].contCtl = KPA_CC_FX_B;  // 18

	fxSlots[FX_SLOT_POS_C].fbv = SWTCH_FX_SLOT_C;
	fxSlots[FX_SLOT_POS_C].paramType = KPA_PARAM_STOMP_C_TYPE;
	fxSlots[FX_SLOT_POS_C].paramState = KPA_PARAM_STOMP_C_STATE;
	fxSlots[FX_SLOT_POS_C].contCtl = KPA_CC_FX_C;  // 19

	fxSlots[FX_SLOT_POS_D].fbv = SWTCH_FX_SLOT_D;
	fxSlots[FX_SLOT_POS_D].paramType = KPA_PARAM_STOMP_D_TYPE;
	fxSlots[FX_SLOT_POS_D].paramState = KPA_PARAM_STOMP_D_STATE;
	fxSlots[FX_SLOT_POS_D].contCtl = KPA_CC_FX_D;  // 20

	fxSlots[FX_SLOT_POS_X].fbv = SWTCH_FX_SLOT_X;
	fxSlots[FX_SLOT_POS_X].paramType = KPA_PARAM_STOMP_X_TYPE;
	fxSlots[FX_SLOT_POS_X].paramState = KPA_PARAM_STOMP_X_STATE;
	fxSlots[FX_SLOT_POS_X].contCtl = KPA_CC_FX_X;  // 22

	fxSlots[FX_SLOT_POS_MOD].fbv = SWTCH_FX_SLOT_MOD;
	fxSlots[FX_SLOT_POS_MOD].paramType = KPA_PARAM_STOMP_MOD_TYPE;
	fxSlots[FX_SLOT_POS_MOD].paramState = KPA_PARAM_STOMP_MOD_STATE;
	fxSlots[FX_SLOT_POS_MOD].contCtl = KPA_CC_FX_MOD;  // 24

	fxSlots[FX_SLOT_POS_DLY].fbv = SWTCH_FX_SLOT_DLY;
	fxSlots[FX_SLOT_POS_DLY].paramType = KPA_PARAM_DELAY_TYPE;
	fxSlots[FX_SLOT_POS_DLY].paramState = KPA_PARAM_DELAY_STATE;
	fxSlots[FX_SLOT_POS_DLY].contCtl = KPA_CC_FX_DLY;  // 27 keep tail (26 cut tail)

	fxSlots[FX_SLOT_POS_REV].fbv = SWTCH_FX_SLOT_REV;
	fxSlots[FX_SLOT_POS_REV].paramType = KPA_PARAM_REVERB_TYPE;
	fxSlots[FX_SLOT_POS_REV].paramState = KPA_PARAM_REVERB_STATE;
	fxSlots[FX_SLOT_POS_REV].contCtl = KPA_CC_FX_REV;  // 29 keep tail (28 cut tail)

}



void setFbvPdlLeds(byte _pdlNum){

	/*
	*Pedal LEDs show Controller Type
	* | | Green | Red |
	*++++++++++++++++++++++++ +
	*| none | off | off |
	*| Wah | off | flash |
	*| Gain | off | on |
	*| Vol | on | off |
	*| Pitch | flash | off |
	*| Morph | flash | flash |
	*/

	switch (fbvPdls[_pdlNum].ctlNum){
	case KPA_CC_VOL:
		fbv.setLedOnOff(fbvPdls[_pdlNum].ledNumGrn, true);
		fbv.setLedOnOff(fbvPdls[_pdlNum].ledNumRed, false);
		break;
	case KPA_CC_WAH:
		fbv.setLedOnOff(fbvPdls[_pdlNum].ledNumGrn, false);
		fbv.setLedFlash(fbvPdls[_pdlNum].ledNumRed, 2000, 1000);
		break;
	case KPA_CC_GAIN:
		fbv.setLedOnOff(fbvPdls[_pdlNum].ledNumGrn, false);
		fbv.setLedOnOff(fbvPdls[_pdlNum].ledNumRed, true);
		break;
	case KPA_CC_PITCH:
		fbv.setLedFlash(fbvPdls[_pdlNum].ledNumGrn, 2000, 1000);
		fbv.setLedOnOff(fbvPdls[_pdlNum].ledNumRed, false);
		break;
	case KPA_CC_MORPH:
		fbv.setLedFlash(fbvPdls[_pdlNum].ledNumGrn, 2000, 1000);
		fbv.setLedFlash(fbvPdls[_pdlNum].ledNumRed, 2000, 1000);
		break;
	default:
		fbv.setLedOnOff(fbvPdls[_pdlNum].ledNumGrn, false);
		fbv.setLedOnOff(fbvPdls[_pdlNum].ledNumRed, false);

	}

}



void onKpaCtlChange(byte chan, byte inCtlNum, byte inCtlVal){

  switch (inCtlNum)
	{
	case KPA_CC_PERFORMANCE_NUM_PREVIEW:
		if (inCtlVal != 0x7f){
			fbv.setDisplayNumber(inCtlVal + 1);
			kpaState.preview = true;
			fbv.setDisplayFlash((FLASH_TIME / 2), (FLASH_TIME / 4));
		}
		break;
	case CC_BANK_MSB:
		// nothing as a maximum of 5 banks is possible
		break;
	case CC_BANK_LSB:
		kpaState.bankNum = inCtlVal;
		break;
	default:
		Serial.print("onKpaCtlChanged ");
		Serial.print(inCtlNum, HEX);
		Serial.print(" - ");
		Serial.print(inCtlVal, HEX);
		Serial.println(" ");
		break;
	}


}

void onKpaPgmChange(byte chan, byte inMidiPgmNum)
{

	uint16_t pgmNum;
	byte channels[5] = { 0, 0, 0, 0, 0 };


	pgmNum = (kpaState.bankNum * 128) + inMidiPgmNum;
	if (pgmNum != kpaState.pgmNum){
		kpaState.preview = false;

		kpaState.pgmNum = pgmNum;

		kpaState.actSlot = kpaState.pgmNum % 5;
		kpaState.actPerformance = kpaState.pgmNum / 5;
		channels[kpaState.actSlot] = 1;
		fbv.setLedOnOff(SWTCH_PRF_SLOT_1, channels[0]);
		fbv.setLedOnOff(SWTCH_PRF_SLOT_2, channels[1]);
		fbv.setLedOnOff(SWTCH_PRF_SLOT_3, channels[2]);
		fbv.setLedOnOff(SWTCH_PRF_SLOT_4, channels[3]);
		fbv.setLedOnOff(SWTCH_PRF_SLOT_5, channels[4]);
		fbv.setDisplayFlash(0, 1);
		fbv.setLedOnOff(LINE6FBV_DISPLAY, 1);
		fbv.setDisplayNumber(kpaState.actPerformance + 1);

        // initializations  
		soloModePostFx = false;
		fbv.setLedOnOff(SWTCH_SOLO,false);
		initPerformanceSlotNames();

		if (millis() - kpaState.lastSent < 500)
			kpaState.lastSent = millis() - CONNECTION_INTERVAL + 500; // BiConn sent in 500 ms as it was just sent. Sending necessary to receive Slot Names
		else
			sendBiConn();
	}
}


void switchSoloModePostFx(){
//
// save actual on/off status when switched on
//   ==> switch all FX on
// when switched off, restore on/off status to the values before switching on
     
	soloModePostFx = !soloModePostFx;
	fbv.setLedOnOff(SWTCH_SOLO, soloModePostFx);

	if (soloModePostFx){
		for (size_t i = 4; i < 8; i++)
		{
			if (fxSlots[i].isEnabled){
				fxSlots[i].isInitialOn = fxSlots[i].isOn;
				if (!fxSlots[i].isOn)
				{
					switchFx(fxSlots[i].fbv);
				}
			}
		}
	}
	else
	{
		for (size_t i = 4; i < 8; i++)
		{
			if (fxSlots[i].isOn){
				if (!fxSlots[i].isInitialOn){
					switchFx(fxSlots[i].fbv);
				}
			}
		}
	}
}




void processKpaModeChanged(uint16_t newMode){

	//Serial.print("New Mode: ");
	//Serial.println(newMode, HEX);
	kpaState.mode = newMode;
		if (kpaState.mode == KPA_MODE_PERFORM){
			kpaSetLooperPrePost(1);
		}
		else{
			kpaSetLooperPrePost(0);
	}

	
	refreshDisplay();

}


void refreshDisplay(void){
	
	//Serial.println("refreshDisplay");
	if (kpaState.tunerIsOn){
		fbv.setDisplayDigit(0, ' ');
		fbv.setDisplayDigit(1, ' ');
		fbv.setDisplayDigit(2, ' ');
	}
	else if (kpaState.mode == KPA_MODE_BROWSE)
	{
		fbv.setDisplayDigits("   ");
		fbv.setDisplayFlat(false);
          
	}
	else if (kpaState.mode == KPA_MODE_PERFORM)
	{
		fbv.setDisplayNumber(kpaState.actPerformance + 1);
		fbv.setDisplayFlat(false);
		fbv.setDisplayDigit(3, ' ');

	}
}

void displayTuner(){
	char noteName;
	uint8_t noteNum;
	bool flat;
	char* tuneString;
	int tuneValue;

	if ((!kpaState.noteNum) && (!kpaState.octave)){
		//Serial.println("Tuner Note: <keine>");
		noteName = ' ';
		flat = false;
	}
	else{
		switch (kpaState.noteNum)
		{
		case 0:
			noteName = 'C';
			flat = false;
			break;
		case 1:
			noteName = 'D';
			flat = true;
			break;
		case 2:
			noteName = 'D';
			flat = false;
			break;
		case 3:
			noteName = 'E';
			flat = true;
			break;
		case 4:
			noteName = 'E';
			flat = false;
			break;
		case 5:
			noteName = 'F';
			flat = false;
			break;
		case 6:
			noteName = 'G';
			flat = true;
			break;
		case 7:
			noteName = 'G';
			flat = false;
			break;
		case 8:
			noteName = 'A';
			flat = true;
			break;
		case 9:
			noteName = 'A';
			flat = false;
			break;
		case 10:
			noteName = 'B';
			flat = true;
			break;
		case 11:
			noteName = 'B';
			flat = false;
			break;

		default:
			break;
		}
	}


	tuneValue = kpaState.tune / 128 - 63;  // tuned =  0

	
	if (noteName == ' ')          tuneString = "I              I";
	else if (tuneValue > 40)    tuneString = "I        ( ( ( I";
	else if (tuneValue > 25)     tuneString = "I        ( (   I";
	else if (tuneValue > 10)     tuneString = "I        (     I";
	else if (tuneValue > 2)      tuneString = "I      **(     I";
	else if ((tuneValue <= 2)
		&& (tuneValue >= -2))  tuneString = "I      **      I";
	else if (tuneValue >= -10) tuneString = "I     )**      I";
	else if (tuneValue >= -25) tuneString = "I   ) )        I";
	else                        tuneString = "I ) ) )        I";


	fbv.setDisplayDigit(3, noteName);
	fbv.setDisplayFlat(flat);
	fbv.setDisplayTitle(tuneString);
	/*
	#if DEBUG_displayTuner
	Serial.print("Tuner Note_: ");
	Serial.print(noteName);
	if (flat)
	Serial.print("b >>");
	else
	Serial.print(" >>");
	Serial.print(tuneValue);
	Serial.println(tuneString);
	#endif
	*/
}
void processKpaParamSingle(uint16_t param, uint16_t value){

   // Delay on /off is sent twice
   // one of them dioes not represent the acual on/off status
   //    the wrong one is sent after the parameter 0x1e1e which i can't say what it is
   //    so the delay status after this value will be ignored
   static bool ignoreDelayOnOff = false;


	switch (param){
	case KPA_PARAM_CURR_TUNING:
		kpaState.tune = value;
		break;
	case KPA_PARAM_CURR_NOTE:
		kpaState.noteNum = value % 12;
		kpaState.octave = value / 12;
		if (kpaState.tunerIsOn)
			displayTuner();
		break;
	case KPA_PARAM_TAP_EVENT:
		fbv.setLedOnOff(LINE6FBV_TAP, value);
		break;
	case KPA_PARAM_TUNER_STATE:
		kpaState.tunerIsOn = (value == 1);
		break;
	case KPA_PARAM_MODE:
		if (value != kpaState.mode)
			processKpaModeChanged(value);
		break;
	case KPA_PARAM_LOOPER_STATE:
		setLooperDigit(value);
		break;
	case 0x1e1e:  // ignore <------------------------------------------------------
		ignoreDelayOnOff = true;
		break;

	case KPA_PARAM_STOMP_A_TYPE:
		fxSlots[FX_SLOT_POS_A].isEnabled = (value);
		break;
	case KPA_PARAM_STOMP_A_STATE:
		fxSlots[FX_SLOT_POS_A].isOn = (value);
		setLedForFxSlot(FX_SLOT_POS_A);
		break;
	case KPA_PARAM_STOMP_B_TYPE:
		fxSlots[FX_SLOT_POS_B].isEnabled = (value);
		break;
	case KPA_PARAM_STOMP_B_STATE:
		fxSlots[FX_SLOT_POS_B].isOn = (value);
		setLedForFxSlot(FX_SLOT_POS_B);
		break;
	case KPA_PARAM_STOMP_C_TYPE:
		fxSlots[FX_SLOT_POS_C].isEnabled = (value);
		break;
	case KPA_PARAM_STOMP_C_STATE:
		fxSlots[FX_SLOT_POS_C].isOn = (value);
		setLedForFxSlot(FX_SLOT_POS_C);
		break;
	case KPA_PARAM_STOMP_D_TYPE:
		fxSlots[FX_SLOT_POS_D].isEnabled = (value);
		break;
	case KPA_PARAM_STOMP_D_STATE:
		fxSlots[FX_SLOT_POS_D].isOn = (value);
		setLedForFxSlot(FX_SLOT_POS_D);
		break;
	case KPA_PARAM_STOMP_X_TYPE:
		fxSlots[FX_SLOT_POS_X].isEnabled = (value);
		break;
	case KPA_PARAM_STOMP_X_STATE:
		fxSlots[FX_SLOT_POS_X].isOn = (value);
		setLedForFxSlot(FX_SLOT_POS_X);
		break;
	case KPA_PARAM_STOMP_MOD_TYPE:
		fxSlots[FX_SLOT_POS_MOD].isEnabled = (value);
		break;
	case KPA_PARAM_STOMP_MOD_STATE:
		fxSlots[FX_SLOT_POS_MOD].isOn = (value);
		setLedForFxSlot(FX_SLOT_POS_MOD);
		break;
	case KPA_PARAM_DELAY_TYPE:
		fxSlots[FX_SLOT_POS_DLY].isEnabled = (value);
		break;
	case KPA_PARAM_DELAY_STATE:
		if (!ignoreDelayOnOff){
			fxSlots[FX_SLOT_POS_DLY].isOn = (value);
			setLedForFxSlot(FX_SLOT_POS_DLY);
		}
		ignoreDelayOnOff = false;
		break;
 case KPA_PARAM_REVERB_TYPE:
		//  this parameter is sent, but always 0.
		//  as a workaround the slot is always handled as enabled.
	case KPA_PARAM_REVERB_STATE:
		fxSlots[FX_SLOT_POS_REV].isOn = (value);
		setLedForFxSlot(FX_SLOT_POS_REV);
		fxSlots[FX_SLOT_POS_REV].isEnabled = true;
		break;
		/*
		default:

		Serial.print("processKpaParamSingle: ");
		Serial.print(param, HEX);
		Serial.print(" - ");
		Serial.println(value, HEX);


		*/
	}
	
}

void setLedForFxSlot(byte slotNum){
    
	if (fxSlots[slotNum].isEnabled){
		if (fxSlots[slotNum].isOn){
			fbv.setLedOnOff(fxSlots[slotNum].fbv, true);
		}
		else{
			fbv.setLedFlash(fxSlots[slotNum].fbv, FLASH_TIME);
		}
	}
	else{
		fbv.setLedOnOff(fxSlots[slotNum].fbv, false);
	}
}


void parseRigNameForPdlAssignment(void){
	// the last two chars in the rig name are missused for pedal assignment
	fbvPdls[0].ctlNum = getPdlCtlNum(kpaState.rigName[30], KPA_CC_WAH);
	fbvPdls[1].ctlNum = getPdlCtlNum(kpaState.rigName[31], KPA_CC_VOL);
	setFbvPdlLeds(0);
	setFbvPdlLeds(1);

	// if neither padal is the volume pedal, send Volume = 127
	if (fbvPdls[0].ctlNum != KPA_CC_VOL && fbvPdls[1].ctlNum != KPA_CC_VOL)
		kpaSendCtlChange(KPA_CC_VOL, 127);
}

uint8_t getPdlCtlNum(char pdlChar, uint8_t defVal){

	//Serial.print("Pdl Char: ");
	//Serial.println(pdlChar, HEX);
	//Serial.print("-default: ");
	//Serial.println(defVal);
	uint8_t retval;

	switch (pdlChar){
	case 'V':
		retval = KPA_CC_VOL;
		break;
	case 'W':
		retval = KPA_CC_WAH;
		break;
	case 'P':
		retval = KPA_CC_PITCH;
		break;
	case 'M':
		retval = KPA_CC_MORPH;
		break;
	case 'G':
		retval = KPA_CC_GAIN;
		break;
	default:
		retval = defVal;
		break;
	}
	return retval;
}


void processKpaParamString(uint32_t param, char * data, unsigned int len){

	switch (param)
	{

	case KPA_STRING_ID_RIG_NAME:
		data[NAME_LENGTH] = 0x00; // prevent overflow
		for (int i = 0; i < NAME_LENGTH; i++){
			kpaState.rigName[i] = 0x00;
		}

		strcpy(kpaState.rigName, data);
		if (kpaState.mode == KPA_MODE_BROWSE){
			fbv.setDisplayTitle(kpaState.rigName);
		}
		if (!kpaState.preview){
			//Serial.print("RIG Name ");
			//     Serial.println(kpaState.rigName);

			parseRigNameForPdlAssignment();
		}
		fbv.syncLedFlash(); // rig name is received after all Stomps
		break;
	case KPA_STRING_ID_PERF_NAME:
		// preview always contains actual name if not in preview mode
		break;
	case KPA_STRING_ID_PERF_NAME_PREVIEW:
		if (kpaState.mode == KPA_MODE_PERFORM){
			if (kpaState.preview)
				fbv.setDisplayTitle((char*)data);
		}
		break;
	case KPA_STRING_ID_SLOT1_NAME:
	case KPA_STRING_ID_SLOT2_NAME:
	case KPA_STRING_ID_SLOT3_NAME:
	case KPA_STRING_ID_SLOT4_NAME:
	case KPA_STRING_ID_SLOT5_NAME:
		if (!kpaState.preview){
			data[NAME_LENGTH] = 0x00; // prevent overflow
			handleSlotNameReceived(data, (param - KPA_STRING_ID_SLOT1_NAME));
		}
		break;
		//recognized but not used
	case KPA_STRING_ID_SLOT1_NAME_PREVIEW:
	case KPA_STRING_ID_SLOT2_NAME_PREVIEW:
	case KPA_STRING_ID_SLOT3_NAME_PREVIEW:
	case KPA_STRING_ID_SLOT4_NAME_PREVIEW:
	case KPA_STRING_ID_SLOT5_NAME_PREVIEW:
		break;
	default:
	/*
		Serial.print("processKpaParamString: ");
		Serial.print(param, HEX);
		Serial.print(" - ");
		Serial.print(len);
		Serial.print(" - ");
		Serial.println(data);
	*/
			break;
	}
	
}

void initPerformanceSlotNames(){
	for (size_t i = 0; i < 5; i++){
		for (size_t j = 0; j < NAME_LENGTH; j++){
			kpaState.performanceSlotNames[i][j] = 0x00;
		}
	}
}

void handleSlotNameReceived(char * data, uint8_t slotNum){
	if (kpaState.mode == KPA_MODE_PERFORM){
		strcpy(kpaState.performanceSlotNames[slotNum], data);
		if (kpaState.actSlot == slotNum){
			//Serial.print("Slot Num: ");
			//Serial.println(slotNum);

			//Serial.print(" SetDisplayTitle: ");
			//Serial.println((char*)kpaState.performanceSlotNames[slotNum]);
			fbv.setDisplayTitle((char*)kpaState.performanceSlotNames[slotNum]);
		}
	}
}

void onKpaSysEx(byte* data, unsigned len)
{

	static uint8_t lastAck_value;

	static struct SysEx * s = (struct SysEx *) (data + 1);
	static int param;
	static int value;
	static int stringSize;


	switch (s->fn) {
	case KPA_SYSEX_FN_RETURN_PARAM:
		param = (s->data[0] << 7) | s->data[1];
		value = (s->data[2] << 7) | s->data[3];
		processKpaParamSingle(param, value);
		break;
	case KPA_SYSEX_FN_RETURN_STRING:
		param = (s->data[0] << 7 | s->data[1]);
		stringSize = strlen((char*)&s->data[2]) + 1; // incl 0x00
		processKpaParamString(param, (char*)&s->data[2], stringSize);
		break;
	case KPA_SYSEX_FN_RETURN_EXT_STRING:
		param = (s->data[0] << 28) | (s->data[1] << 21) | (s->data[2] << 14) | s->data[3] << 7 | s->data[4];
		stringSize = strlen((char*)&s->data[5]) + 1; // incl 0x00
		processKpaParamString(param, (char*)&s->data[5], stringSize);
		break;
	case KPA_SYSEX_FN_ACK:
		if (s->data[0] == 0x7F) {
			if (connection.ackReceived && (lastAck_value + 1 != s->data[1])) {
				connection.lastAck = 0;
				connection.ackReceived = 0;
			}
			else {
				connection.ackReceived = 1;
			}
			connection.lastAck = millis();
			lastAck_value = s->data[1];
		}
		break;
	default:
		/*
		Serial.print(s->fn, HEX);
		Serial.print("===");
		for (size_t i = 0; i < len; i++){
		Serial.print(s->data[i], HEX);
		Serial.print("-");
		}
		Serial.println(" ");
		*/
		break;
	}

}

void onKpaSense(void){
	connection.senseReceived = true;
}

void sendBiConn(void){
   // sending 0x2f (position 11) says the kemper to send al slot and namer information each time the 
   //    connection string is sent.
   // sending 0x2f once and 0x2e every other time lets the Kemper send only changed values.
   //    i tried this, but it didn't work after switching between modes
   // ==> 0x2f every time

	byte cnnStr[] = { 0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, 0x7E, 0x00, 0x40, 0x03, 0x2f, 0x05, 0xF7 };
	Serial.println("APP: SendBiConn");

	//cnnStr[10] = 0x2e | (connection.ackReceived ? 0 : 1); // flags

	kpa.sendSysEx(13, cnnStr, 1);
	
	kpaState.lastSent = millis();
	
}

void kpaSendCtlChange(byte inCtlNum, byte inCtlVal){
	kpa.sendControlChange(inCtlNum, inCtlVal, KPA_MIDI_CHANNEL);
}


// respond to pressed keys on the FBV
void onFbvKeyPressed(byte inKey) {

    // check first if the switch is missused for the looper
	if (keyPressUsedByLooper(inKey))
		return;

	switch (inKey){

	case SWTCH_BANK_UP:
		kpaSendCtlChange(48, 1);
		break;
	case SWTCH_BANK_DOWN:
		kpaSendCtlChange(49, 1);
		break;
	case SWTCH_PRF_SLOT_1:
		kpaSendCtlChange(50, 1);
		break;
	case SWTCH_PRF_SLOT_2:
		kpaSendCtlChange(51, 1);
		break;
	case SWTCH_PRF_SLOT_3:
		kpaSendCtlChange(52, 1);
		break;
	case SWTCH_PRF_SLOT_4:
		kpaSendCtlChange(53, 1);
		break;
	case SWTCH_PRF_SLOT_5:
		kpaSendCtlChange(54, 1);
		break;
	case SWTCH_FX_SLOT_A:
	case SWTCH_FX_SLOT_B:
	case SWTCH_FX_SLOT_C:
	case SWTCH_FX_SLOT_D:
	case SWTCH_FX_SLOT_X:
	case SWTCH_FX_SLOT_MOD:
	case SWTCH_FX_SLOT_DLY:
	case SWTCH_FX_SLOT_REV:
		switchFx(inKey);
		break;
	case SWTCH_SOLO:
		switchSoloModePostFx();
		break;
	case SWTCH_TAP:
		kpaSendCtlChange(KPA_CC_TAP, true);
		break;
	}
}

bool keyPressUsedByLooper(byte inKey){

	bool retVal = true;

	if (kpaState.looperIsOn){
		switch (inKey){
		case SWTCH_LOOPER_START: kpaSendLooperCmd(KPA_LOOPER_CC_START, true);  break;
		case SWTCH_LOOPER_STOP: kpaSendLooperCmd(KPA_LOOPER_CC_STOP, true);  break;
		case SWTCH_LOOPER_TRIGGER: kpaSendLooperCmd(KPA_LOOPER_CC_TRIGGER, true);  break; 
		case SWTCH_LOOPER_UNDO: kpaSendLooperCmd(KPA_LOOPER_CC_UNDOREDO, true);  break;
		case SWTCH_LOOPER_HALFTIME: kpaSendLooperCmd(KPA_LOOPER_CC_HALFTIME, true);  break;
		case SWTCH_LOOPER_REVERSE: kpaSendLooperCmd(KPA_LOOPER_CC_REVERSE, true);  break;
		default: retVal = false;

		}
	}
	else{
		retVal = false;
	}
	return retVal;
}


void onFbvKeyReleased(byte inKey, byte inKeyHeld) {

    // check first if the switch is missused for the looper
	if (keyReleaseUsedByLooper(inKey))
		return;

	switch (inKey){

	case SWTCH_BANK_UP:
		kpaSendCtlChange(48, 0);
		break;
	case SWTCH_BANK_DOWN:
		kpaSendCtlChange(49, 0);
		break;
	case SWTCH_PRF_SLOT_1:
		kpaSendCtlChange(50, 0);
		break;
	case SWTCH_PRF_SLOT_2:
		kpaSendCtlChange(51, 0);
		break;
	case SWTCH_PRF_SLOT_3:
		kpaSendCtlChange(52, 0);
		break;
	case SWTCH_PRF_SLOT_4:
		kpaSendCtlChange(53, 0);
		break;
	case SWTCH_PRF_SLOT_5:
		kpaSendCtlChange(54, 0);
		break; case SWTCH_TAP:
		kpaSendCtlChange(KPA_CC_TAP, false);
		break;
	}
}


bool keyReleaseUsedByLooper(byte inKey){

	bool retVal = true;

	if (kpaState.looperIsOn){
		switch (inKey){
		case SWTCH_LOOPER_START: kpaSendLooperCmd(KPA_LOOPER_CC_START, false);  break;
		case SWTCH_LOOPER_STOP: kpaSendLooperCmd(KPA_LOOPER_CC_STOP, false);  break;
		case SWTCH_LOOPER_TRIGGER: kpaSendLooperCmd(KPA_LOOPER_CC_TRIGGER, false);  break;
		case SWTCH_LOOPER_UNDO: kpaSendLooperCmd(KPA_LOOPER_CC_UNDOREDO, false);  break;
		case SWTCH_LOOPER_HALFTIME: kpaSendLooperCmd(KPA_LOOPER_CC_HALFTIME, false);  break;
		case SWTCH_LOOPER_REVERSE: kpaSendLooperCmd(KPA_LOOPER_CC_REVERSE, false);  break;
		default: retVal = false;
		}
	}
	else{
		retVal = false;
	}
	return retVal;

}

void kpaSendLooperCmd(byte inCmd, byte inKeyPress){
	kpaSendCtlChange(0x63, 0x7D);
	kpaSendCtlChange(0x62, inCmd);
	kpaSendCtlChange(0x06, 0x00);
	kpaSendCtlChange(0x26, inKeyPress);
}

void onFbvKeyHeld(byte inKey) {
	// looper position PRE for Browse mode ==> you can test several sounds for your loop
	// looper position POST for Performance ==> you can solo over your loop with a different sound
	
	//Serial.println("FBV KEY HELD");
    
	switch (inKey){
	case SWTCH_LOOPER:
		kpaState.looperIsOn = !kpaState.looperIsOn;
		fbv.setLedOnOff(SWTCH_LOOPER, kpaState.looperIsOn);

		break;
	case SWTCH_RESET:
		fbv.setDisplayTitle("-----RESET------");
		resetFunc(); //call reset 
		break;
	}
}

void requestLooperState(){
	sysexBuffer.fn = KPA_PARAM_TYPE_SINGLE;
	sysexBuffer.data[0] = (KPA_PARAM_LOOPER_STATE >> 7) & 0xFF;
	sysexBuffer.data[1] = KPA_PARAM_LOOPER_STATE & 0x7F;
	kpa.sendSysEx(KPA_SYSEX_HEADER_SIZE + 2, (const byte *)&sysexBuffer, 0);
}

void kpaSetLooperPrePost(uint8_t loc) {
	sysexBuffer.fn = 0x01;
	sysexBuffer.data[0] = 0x7F;
	sysexBuffer.data[1] = 0x35;
	sysexBuffer.data[2] = 0x0;
	sysexBuffer.data[3] = loc; // 0x0=pre, 0x1=post
	kpa.sendSysEx(KPA_SYSEX_HEADER_SIZE + 4, (const byte *)&sysexBuffer, 0);
}

void handleConnectionAndSomeRequests(){


	


	switch (connection.state) {
	case CNN_STATE_WAIT_SENSE:
		if (connection.senseReceived) {
			connection.state = CNN_STATE_CONNECT;
		}
	case CNN_STATE_CONNECT:
		if (millis() - kpaState.lastSent > 1000) {
			fbv.setDisplayTitle("CONNECTING");
			sendOwner();
			sendBiConn();
			
		}
		if (connection.ackReceived) {
			connection.state = CNN_STATE_WAIT_INITIAL_DATA;
		}
		break;
	case CNN_STATE_WAIT_INITIAL_DATA:
		if (millis() - kpaState.lastSent > 1000) {
			fbv.setDisplayTitle("INITIAL REQUEST");
			
			sendBiConn();
			connection.state = CNN_STATE_RUN;
		}
	case CNN_STATE_RUN:
		if (millis() - kpaState.lastSent > CONNECTION_INTERVAL) {
			sendBiConn();
			nextLooperStateRequest = millis() + LOOPER_STATE_REQUEST_INTERVAL; // suspend request
		}
		else if (millis() > nextLooperStateRequest){
				requestLooperState();
				nextLooperStateRequest = millis() + LOOPER_STATE_REQUEST_INTERVAL;
			}
		if (millis() - connection.lastAck > 5000) {
			connection.state = CNN_STATE_WAIT_INITIAL_DATA;
			connection.ackReceived = 0;
			connection.senseReceived = 0;
		}
		break;
	}


}
void sendOwner(void){

	sysexBuffer.fn = 0x03;
	sysexBuffer.data[0] = 0x7F;
	sysexBuffer.data[1] = 0x7F;
	strcpy((char *)sysexBuffer.data + 2, "WRBI@ORBI_05_01");
	kpa.sendSysEx(KPA_SYSEX_HEADER_SIZE + 2 + 15, (const byte *)&sysexBuffer, 0);

}

void switchFx(byte inKey) {
	for (int i = 0; i < FX_SLOTS; i++){
		if (fxSlots[i].fbv == inKey){
			if (fxSlots[i].isEnabled){
				fxSlots[i].isOn = !fxSlots[i].isOn;
				kpaSendCtlChange(fxSlots[i].contCtl, fxSlots[i].isOn);
			}
			i = FX_SLOTS;
		}

	}
}


// handle MIDI Control Change sent by the FBV
void onFbvCtlChange(byte inCtrl, byte inValue) {

	/*
	Serial.print("FBV CtlChanged ");
	Serial.println(inValue, HEX);
	*/

	if (inCtrl == LINE6FBV_CC_PDL1){
		fbvPdls[0].actPos = inValue;
		if (fbvPdls[0].ctlNum)
			kpaSendCtlChange(fbvPdls[0].ctlNum, fbvPdls[0].actPos);
	}
	else{
		fbvPdls[1].actPos = inValue;
		if (fbvPdls[1].ctlNum)
			kpaSendCtlChange(fbvPdls[1].ctlNum, fbvPdls[1].actPos);
	}
}

void setLooperDigit(uint16_t value){

	switch (value)
	{
	case 0:
		if (kpaState.looperIsOn)
			fbv.setDisplayDigit(3, 'L');
		else
			fbv.setDisplayDigit(3, ' ');
		break;
	case 1:
		fbv.setDisplayDigit(3, 'S');  //stop
		break;
	case 2:
		fbv.setDisplayDigit(3, 'T'); // trigger
		break;
	case 3:
		fbv.setDisplayDigit(3, 'O'); // overdub
		break;
	case 4:
		fbv.setDisplayDigit(3, 'P'); // play
		break;
	case 5:
		fbv.setDisplayDigit(3, 'R'); // record
		break;
	}

}


void setup()
{
	Serial.begin(115200);
	Serial.println("los gehts...");

	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);

	// open port for FBV 
	fbv.begin(&SERIAL_FBV);

	// set callback functions for the FBV
	fbv.setHandleKeyPressed(&onFbvKeyPressed);
	fbv.setHandleKeyReleased(&onFbvKeyReleased);
	fbv.setHandleKeyHeld(&onFbvKeyHeld);
	fbv.setHandleCtrlChanged(&onFbvCtlChange);

	// enable Hold function
	fbv.setHoldTime(SWTCH_LOOPER, HOLD_TIME_SWITCH_LOOPER);
	fbv.setHoldTime(SWTCH_RESET, HOLD_TIME_RESET);

	// turn off all LEDs
	fbv.setLedOnOff(LINE6FBV_FXLOOP, 0);
	fbv.setLedOnOff(LINE6FBV_STOMP1, 0);
	fbv.setLedOnOff(LINE6FBV_STOMP2, 0);
	fbv.setLedOnOff(LINE6FBV_STOMP3, 0);
	fbv.setLedOnOff(LINE6FBV_AMP1, 0);
	fbv.setLedOnOff(LINE6FBV_AMP2, 0);
	fbv.setLedOnOff(LINE6FBV_REVERB, 0);
	fbv.setLedOnOff(LINE6FBV_PITCH, 0);
	fbv.setLedOnOff(LINE6FBV_MOD, 0);
	fbv.setLedOnOff(LINE6FBV_DELAY, 0);
	fbv.setLedOnOff(LINE6FBV_TAP, 0);
	fbv.setLedOnOff(LINE6FBV_UP, 0);
	fbv.setLedOnOff(LINE6FBV_DOWN, 0);
	fbv.setLedOnOff(LINE6FBV_CHANNELA, 0);
	fbv.setLedOnOff(LINE6FBV_CHANNELB, 0);
	fbv.setLedOnOff(LINE6FBV_CHANNELC, 0);
	fbv.setLedOnOff(LINE6FBV_CHANNELD, 0);
	fbv.setLedOnOff(LINE6FBV_FAVORITE, 0);
	fbv.setLedOnOff(LINE6FBV_PDL1_GRN, 0);
	fbv.setLedOnOff(LINE6FBV_PDL1_RED, 0);
	fbv.setLedOnOff(LINE6FBV_PDL2_GRN, 0);
	fbv.setLedOnOff(LINE6FBV_PDL2_RED, 0);

	// display initial screen
	fbv.setLedOnOff(LINE6FBV_DISPLAY, 1);
	fbv.setDisplayTitle("WRBI(AT)ORBI");
	fbv.updateUI();

	// open port for Kemper (midi) 
	kpa.begin();
	kpa.setInputChannel(KPA_MIDI_CHANNEL);
	kpa.turnThruOff();


	// define callback functions for Kemper
	kpa.setHandleActiveSensing(onKpaSense);
	kpa.setHandleControlChange(onKpaCtlChange);
	kpa.setHandleProgramChange(onKpaPgmChange);
	kpa.setHandleSystemExclusive(onKpaSysEx);

	// initiallize arrays 
	initFxSlots();
	initFbvPdlValues();

	Serial.println("fertsch");
}



void loop()
{

	fbv.read();  // Receive Commands from FBV

	kpa.read();  // Receive Information from KPA

	handleConnectionAndSomeRequests();  // keep bidirectional connection alive

	fbv.updateUI(); // update the FBV display and LEDs
}

