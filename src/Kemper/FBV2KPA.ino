/*!
*  @file       FBV2KPA.ino
*  Project     Control the Kemper Profiling Amplifier with a Line6 FBV Longboard
*  @brief      Control KPA with FBV
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



#include "Line6Fbv.h"
#include "KPA.h"

// had performance problems in earlier version
// maybe no longer needed
const int PAUSE_REQUESTS = 20;
const int FLASH_TIME = 1000;

// Controller Numbers for Pedals. default WAH and VOL
byte mCcPdl1 = KPA_CC_WAH;
byte mCcPdl2 = KPA_CC_VOL;

// positions in the array
const byte FX_SLOT_POS_A = 0;
const byte FX_SLOT_POS_B = 1;
const byte FX_SLOT_POS_C = 2;
const byte FX_SLOT_POS_D = 3;
const byte FX_SLOT_POS_X = 4;
const byte FX_SLOT_POS_MOD = 5;
const byte FX_SLOT_POS_DLY = 6;
const byte FX_SLOT_POS_REV = 7;
const byte CC_BANK_LSB = 0x20;

Line6Fbv mFbv = Line6Fbv();
KPA mKpa = KPA();

int mBPM = 0;

byte mMidiBank = 0;
byte mMidiPgm = 0;

bool mLooperMode = false;

bool mSendInitialRequests = false;
bool mSendNextRequest = true;
unsigned long mReqestLastSent = 0;
unsigned long mPauseRequestsUntil = 0;

byte mPerformanceSlotNames[5][16] = {0x00};  // not yet recognized


/*
	bool mAmp1On = false;
	bool mAmp2On = false;
*/

byte mActPerformance = 0;
byte mNextPerformance = 0;   // for UP/DOWN events
byte mActChannel = 0; // A B C D E=Favorite
int mActPgmNum = 99999;

bool mTunerIsOn = false;



struct ParamRequest
{
	byte requestType;
	byte addressPage;
	byte paramNumber;
	bool isFx;  // if the parameter represents an FX slot, set the corresponding LED on FBV      
	bool requested;
	bool received;
	
};



// Paramter Requests to be sent to KPA after receiving Program Change info from the KPA
const int PARAM_REQUESTS = 17;

ParamRequest FX_A_ON = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_A, KPA_PARAM_NUM_FX_ON, true };
ParamRequest FX_B_ON = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_B, KPA_PARAM_NUM_FX_ON, true };
ParamRequest FX_C_ON = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_C, KPA_PARAM_NUM_FX_ON, true };
ParamRequest FX_D_ON = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_D, KPA_PARAM_NUM_FX_ON, true };
ParamRequest FX_X_ON = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_X, KPA_PARAM_NUM_FX_ON, true };
ParamRequest FX_MOD_ON = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_MOD, KPA_PARAM_NUM_FX_ON, true };
ParamRequest FX_DLY_ON = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_DLY, KPA_PARAM_NUM_FX_ON2, true };
ParamRequest FX_REV_ON = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_REV, KPA_PARAM_NUM_FX_ON2, true };
ParamRequest FX_A_TY = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_A, KPA_PARAM_NUM_FX_TYPE, true };
ParamRequest FX_B_TY = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_B, KPA_PARAM_NUM_FX_TYPE, true };
ParamRequest FX_C_TY = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_C, KPA_PARAM_NUM_FX_TYPE, true };
ParamRequest FX_D_TY = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_D, KPA_PARAM_NUM_FX_TYPE, true };
ParamRequest FX_X_TY = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_X, KPA_PARAM_NUM_FX_TYPE, true };
ParamRequest FX_MOD_TY = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_MOD, KPA_PARAM_NUM_FX_TYPE, true };
ParamRequest FX_DLY_TY = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_DLY, KPA_PARAM_NUM_FX_TYPE, true };
ParamRequest FX_REV_TY = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_FX_REV, KPA_PARAM_NUM_FX_TYPE, true };

ParamRequest RIG_TEMPO = { KPA_PARAM_TYPE_SINGLE, KPA_ADDR_RIG, KPA_PARAM_NUM_RIG_TEMPO, false };

ParamRequest mParamRequests[] = {
	FX_A_TY,
	FX_B_TY,
	FX_C_TY,
	FX_D_TY,
	FX_X_TY,
	FX_MOD_TY,
	FX_DLY_TY,
	FX_REV_TY,
	FX_A_ON,
	FX_B_ON,
	FX_C_ON,
	FX_D_ON,
	FX_X_ON,
	FX_MOD_ON,
	FX_DLY_ON,
	FX_REV_ON,
	RIG_TEMPO,
};


// 
const int FX_SLOTS = 8;

struct FxSlot{
	byte fbv;             // corresponding Switch on FBV
	byte addrPage;        // SysEx Adress Page 
	byte contCtl;         // Midi CC Number to send
	bool isEnabled;       // Slot is not empty
	bool isInitialOn;     // On at program change
	bool isOn;            // actual status 
	bool isWah;           // Type for Pedal assignment  
	bool isPitch;         // Type for Pedal assignment
	
};


FxSlot mFxSlots[FX_SLOTS];


// set Addresspage, CC number, corresponding FBV Switch for each FX slot
void fInitFxSlots(){
	mFxSlots[FX_SLOT_POS_A].fbv = LINE6FBV_FXLOOP;
	mFxSlots[FX_SLOT_POS_A].addrPage = KPA_ADDR_FX_A;  // 50
	mFxSlots[FX_SLOT_POS_A].contCtl = KPA_CC_FX_A;   // 17
	
	mFxSlots[FX_SLOT_POS_B].fbv = LINE6FBV_STOMP1;
	mFxSlots[FX_SLOT_POS_B].addrPage = KPA_ADDR_FX_B; // 51
	mFxSlots[FX_SLOT_POS_B].contCtl = KPA_CC_FX_B;  // 18
	
	mFxSlots[FX_SLOT_POS_C].fbv = LINE6FBV_STOMP2;
	mFxSlots[FX_SLOT_POS_C].addrPage = KPA_ADDR_FX_C; // 52
	mFxSlots[FX_SLOT_POS_C].contCtl = KPA_CC_FX_C;  // 19
	
	mFxSlots[FX_SLOT_POS_D].fbv = LINE6FBV_STOMP3;
	mFxSlots[FX_SLOT_POS_D].addrPage = KPA_ADDR_FX_D; // 53
	mFxSlots[FX_SLOT_POS_D].contCtl = KPA_CC_FX_D;  // 20
	
	mFxSlots[FX_SLOT_POS_X].fbv = LINE6FBV_PITCH;
	mFxSlots[FX_SLOT_POS_X].addrPage = KPA_ADDR_FX_X; // 56
	mFxSlots[FX_SLOT_POS_X].contCtl = KPA_CC_FX_X;  // 22
	
	mFxSlots[FX_SLOT_POS_MOD].fbv = LINE6FBV_MOD;
	mFxSlots[FX_SLOT_POS_MOD].addrPage = KPA_ADDR_FX_MOD; // 58
	mFxSlots[FX_SLOT_POS_MOD].contCtl = KPA_CC_FX_MOD;  // 24
	
	mFxSlots[FX_SLOT_POS_DLY].fbv = LINE6FBV_DELAY;
	mFxSlots[FX_SLOT_POS_DLY].addrPage = KPA_ADDR_FX_DLY; // 74
	mFxSlots[FX_SLOT_POS_DLY].contCtl = KPA_CC_FX_DLY;  // 27 keep tail (26 cut tail)
	
	mFxSlots[FX_SLOT_POS_REV].fbv = LINE6FBV_REVERB;
	mFxSlots[FX_SLOT_POS_REV].addrPage = KPA_ADDR_FX_REV; // 75
	mFxSlots[FX_SLOT_POS_REV].contCtl = KPA_CC_FX_REV;  // 29 keep tail (28 cut tail)
	
	fResetFxSlotValues();
	
}

// reset all FX Slot Parameters before requesting data from the KPA
void fResetFxSlotValues(){
	for (int i = 0; i < FX_SLOTS; i++){
		mFxSlots[i].isEnabled = 0;
		mFxSlots[i].isInitialOn = 0;
		mFxSlots[i].isOn = 0;
		mFxSlots[i].isWah = 0;
		mFxSlots[i].isPitch = 0;
	}
	mFxSlots[FX_SLOT_POS_DLY].isEnabled = true;
	mFxSlots[FX_SLOT_POS_REV].isEnabled = true;

}

// respond to pressed keys on the FBV
void onFbvKeyPressed(byte inKey) {
	
	bool switchBank = 0;
	bool switchChannel = 0;
	byte key;
	
	key = inKey;
	
		// link pedal switches to their corresponding slot switches
		if (key == LINE6FBV_PDL1_SW){
			// key = actual wah switch
		}
		
		if (key == LINE6FBV_PDL2_SW){
			// key = actual pitch switch
		}
		
		switch (key){
			case LINE6FBV_UP:
			mFbv.setLedOnOff(LINE6FBV_UP, true);
			switchBank = true;
			break;
			case LINE6FBV_DOWN:
			mFbv.setLedOnOff(LINE6FBV_DOWN, true);
			switchBank = true;
			break;
			case LINE6FBV_CHANNELA:
			case LINE6FBV_CHANNELB:
			case LINE6FBV_CHANNELC:
			case LINE6FBV_CHANNELD:
			case LINE6FBV_FAVORITE:
			switchChannel = true;
			break;
			case LINE6FBV_TAP:
			mKpa.sendCtlChange(KPA_CC_TAP, true);
			break;
			case LINE6FBV_FXLOOP:
			fSwitchFxSlot(FX_SLOT_POS_A);
			break;
			case LINE6FBV_STOMP1:
			fSwitchFxSlot(FX_SLOT_POS_B);
			break;
			case LINE6FBV_STOMP2:
			fSwitchFxSlot(FX_SLOT_POS_C);
			break;
			case LINE6FBV_STOMP3:
			fSwitchFxSlot(FX_SLOT_POS_D);
			break;
		}
	
		if (!mLooperMode){
			switch (key){
			case LINE6FBV_MOD:
				fSwitchFxSlot(FX_SLOT_POS_MOD);
				break;
			case LINE6FBV_DELAY:
				fSwitchFxSlot(FX_SLOT_POS_DLY);
				break;
			case LINE6FBV_REVERB:
				fSwitchFxSlot(FX_SLOT_POS_REV);
				break;
			case LINE6FBV_PITCH:
				fSwitchFxSlot(FX_SLOT_POS_X);
				break;
			}
		}
		else // looperMode
		{
			switch (key)
			{
			case LINE6FBV_AMP1: mKpa.sendLooperCmd(KPA_LOOP_REC_PLAY, true); break;
			case LINE6FBV_AMP2: mKpa.sendLooperCmd(KPA_LOOP_STOP_ERASE, true); break;
			case LINE6FBV_REVERB: mKpa.sendLooperCmd(KPA_LOOP_TRIGGER, true); break;
			case LINE6FBV_PITCH: mKpa.sendLooperCmd(KPA_LOOP_REVERSE, true); break;
			case LINE6FBV_MOD: mKpa.sendLooperCmd(KPA_LOOP_HALFTIME, true); break;
			case LINE6FBV_DELAY: mKpa.sendLooperCmd(KPA_LOOP_UNDO, true); break;
			}

		}
		
		
		
		if (switchBank){
			fSetNextPerformanceValue(key);
		}
		if (switchChannel){
			fChangeProgram(key);
		}
	
}

void fResetParamRequests(){
	for (size_t i = 0; i < PARAM_REQUESTS; i++)
	{
		mParamRequests[i].received = false;
		mParamRequests[i].requested = false;
	}
	
}

void fSwitchFxSlot(byte slotNum){
	if (mFxSlots[slotNum].isEnabled == true){
		mFxSlots[slotNum].isOn = !mFxSlots[slotNum].isOn;
		mKpa.sendCtlChange(mFxSlots[slotNum].contCtl, mFxSlots[slotNum].isOn);
		fSetLedForFxSlot(slotNum);
	}
	mFbv.syncLedFlash();
}

void fSetLedForFxSlot(byte slotNum){
	
	/*
		if ((mFxSlots[slotNum].isEnabled) && (mFxSlots[slotNum].isOn))
		mFbv.setLedOnOff(mFxSlots[slotNum].fbv, true);
		else
		mFbv.setLedOnOff(mFxSlots[slotNum].fbv, false);
	*/
	
	if (mFxSlots[slotNum].isEnabled){
		if (mFxSlots[slotNum].isOn){
			mFbv.setLedOnOff(mFxSlots[slotNum].fbv, true);
		}
		else{
			mFbv.setLedFlash(mFxSlots[slotNum].fbv, FLASH_TIME);
		}
	}
	else{
		mFbv.setLedOnOff(mFxSlots[slotNum].fbv, false);
	}
	
	
	
}


void fChangeProgram(byte inKey){
	byte pgmNum;
	pgmNum = mNextPerformance * 5;
	
	switch (inKey){
		case  LINE6FBV_CHANNELA:
		break; // nothhing to add
		case  LINE6FBV_CHANNELB:
		pgmNum += 1;
		break;
		case  LINE6FBV_CHANNELC:
		pgmNum += 2;
		break;
		case  LINE6FBV_CHANNELD:
		pgmNum += 3;
		break;
		case  LINE6FBV_FAVORITE:
		pgmNum += 4;
		break;
		
	}
	
	/*
		Serial.print("APP pgmChange ");
		Serial.println(pgmNum);
	*/
	
	if (mActPerformance != mNextPerformance){
		fInitPerformanceSlotNames();
	}
	
	if (mActPgmNum != pgmNum){
		
		mActPgmNum = pgmNum;    // remember PgmNum
		mActPerformance = mNextPerformance;
		mActChannel = mActPgmNum % 5;
		
		mKpa.sendPgmChange(pgmNum);  // Pgm Change
		
		fDisplayPgmInfo();      // update display
		
	}
}

void fInitPerformanceSlotNames(){
	for (size_t i = 0; i < 5; i++){
    	for (size_t j = 0; j < 16; j++){
			mPerformanceSlotNames[i][j] = 0x00;
		}
	}
}


void fSetNextPerformanceValue(byte inKey){
	
	
	
	switch (inKey){
	case LINE6FBV_UP:
	if (mNextPerformance < 124)
	mNextPerformance++;
	break;
	case LINE6FBV_DOWN:
	if (mNextPerformance > 0)
	mNextPerformance--;
	break;
}

Serial.print("APP mNextPerformance ");
Serial.println(mNextPerformance);

// Display new bank in the title field
mFbv.setDisplayNumber(mNextPerformance + 1);

if (mNextPerformance != mActPerformance){
	mFbv.setDisplayFlash(FLASH_TIME, (FLASH_TIME / 5 ));
	
}
else{
	mFbv.setLedOnOff(LINE6FBV_DISPLAY, true);
}

}

void onFbvKeyReleased(byte inKey, byte inKeyHeld) {
	
	/*
		Serial.print("FBV KyeReleased ");
		Serial.println(inKey, HEX);
	*/
	switch (inKey){
		case LINE6FBV_UP:
		mFbv.setLedOnOff(LINE6FBV_UP, false);
		break;
		case LINE6FBV_DOWN:
		mFbv.setLedOnOff(LINE6FBV_DOWN, false);
		break;
		case LINE6FBV_TAP:
		mKpa.sendCtlChange(KPA_CC_TAP, false);
		mKpa.sendParamRequest(KPA_PARAM_TYPE_SINGLE, KPA_ADDR_RIG, KPA_PARAM_NUM_RIG_TEMPO);
		break;
	}
	if (mLooperMode){
		switch (inKey)
		{
			case LINE6FBV_AMP1:   mKpa.sendLooperCmd(KPA_LOOP_REC_PLAY, false); break;
			case LINE6FBV_AMP2:   mKpa.sendLooperCmd(KPA_LOOP_STOP_ERASE, false); break;
			case LINE6FBV_REVERB: mKpa.sendLooperCmd(KPA_LOOP_TRIGGER, false); break;
			case LINE6FBV_PITCH:  mKpa.sendLooperCmd(KPA_LOOP_REVERSE, false); break;
			case LINE6FBV_MOD:    mKpa.sendLooperCmd(KPA_LOOP_HALFTIME, false); break;
			case LINE6FBV_DELAY:  mKpa.sendLooperCmd(KPA_LOOP_UNDO, false); break;
		}
	}
}

void onFbvKeyHeld(byte inKey) {
	/*Serial.print("onFbvKeyHeld ");
	Serial.print(inKey, HEX);
	Serial.print(" act Channel ");
	Serial.println(mActChannel, HEX);*/
	switch (inKey){
		case LINE6FBV_CHANNELA:
		if (mActChannel == 0)
		mLooperMode = !mLooperMode;
		break;
		case LINE6FBV_CHANNELB:
		if (mActChannel == 1)
		mLooperMode = !mLooperMode;
		break;
		case LINE6FBV_CHANNELC:
		if (mActChannel == 2)
		mLooperMode = !mLooperMode;
		break;
		case LINE6FBV_CHANNELD:
		if (mActChannel == 3)
		mLooperMode = !mLooperMode;
		break;
		case LINE6FBV_FAVORITE:
		if (mActChannel == 4)
		mLooperMode = !mLooperMode;
		break;
		
	}
	
	if (mLooperMode)
	mFbv.setDisplayDigit(3, 'L');
	else
	mFbv.setDisplayDigit(3, ' ');
}




// handle MIDI Control Change sent by the FBV
void onFbvCtlChanged(byte inCtrl, byte inValue) {
	
	/*
		Serial.print("FBV CtlChanged ");
		Serial.println(inValue, HEX);
	*/
	if (inCtrl == LINE6FBV_PDL1){
		mKpa.sendCtlChange(mCcPdl1, inValue);
	}
	else{
		mKpa.sendCtlChange(mCcPdl2, inValue);
	}
}

// in Heartbeat of FBV send Heartbeat to KPA
void onFbvHeartbeat() {
	
	Serial.println("FBV: Heartbeat");
	mKpa.sendHeartbeat(); // tell KPA that midi controller is connected. needed for bidirectional communication
}


void fDisplayPgmInfo(){
	
	byte channels[5] = { 0, 0, 0, 0, 0 };
	
	mFbv.setDisplayNumber(mActPerformance + 1);
	
	channels[mActChannel] = 1;
	
	mFbv.setLedOnOff(LINE6FBV_CHANNELA, channels[0]);
	mFbv.setLedOnOff(LINE6FBV_CHANNELB, channels[1]);
	mFbv.setLedOnOff(LINE6FBV_CHANNELC, channels[2]);
	mFbv.setLedOnOff(LINE6FBV_CHANNELD, channels[3]);
	mFbv.setLedOnOff(LINE6FBV_FAVORITE, channels[4]);
	
	mFbv.setLedOnOff(LINE6FBV_DISPLAY, true);
	
	/*
		Serial.print("fDisplayPgmInfo ");
		Serial.print(channels[0]);
		Serial.print(channels[1]);
		Serial.print(channels[2]);
		Serial.print(channels[3]);
		Serial.println(channels[4]);
	*/
}


void fSendRequestsToKpa(){
	
	bool allReceived = true;
	bool resend = false;
	
	if (!mSendInitialRequests)
	return;
	
	if (millis() < mPauseRequestsUntil)
	return;
	
	// check if all requests are already received
	for (size_t i = 0; i < PARAM_REQUESTS; i++)
	{
		if (!mParamRequests[i].received){
			allReceived = false;
			i = PARAM_REQUESTS;
		}
	}
	
	
	// if all requests are answered, initial requests are no longer sent
	if (allReceived){
		mSendInitialRequests = false;
		mFbv.syncLedFlash();
		//mFbv.setDisplayTitle(mPerformanceSlotNames[mActChannel]);
		return;
	}
	
	
	if (!mSendNextRequest){
		if (millis() - mReqestLastSent > 100){
			mReqestLastSent = millis();
			mSendNextRequest = true;
			resend = true;
		}
		else{
			return;
		}
	}
	
	
	for (size_t i = 0; i < PARAM_REQUESTS; i++)
	{
		if (!mParamRequests[i].received){
			if ((!mParamRequests[i].requested) || (resend)){
				mKpa.sendParamRequest(mParamRequests[i].requestType,
				mParamRequests[i].addressPage,
				mParamRequests[i].paramNumber);
				mSendNextRequest = false;
				mParamRequests[i].requested = true;
				/*
				Serial.print("fSendRequestsToKpa ");
					Serial.print(mParamRequests[i].requestType, HEX);
					Serial.print(" - ");
					Serial.print(mParamRequests[i].addressPage, HEX);
					Serial.print(" - ");
					Serial.println(mParamRequests[i].paramNumber, HEX);
				*/
			}
			i = PARAM_REQUESTS;
		}
	}
	
	
}




void onKpaCtlChanged(byte inCtlNum, byte inCtlVal)
{
	
	Serial.print("onKpaCtlChanged ");
	Serial.print(inCtlNum, HEX);
	Serial.print(" - ");
	Serial.print(inCtlVal, HEX);
	Serial.println(" ");
	
	// only Bank Vhange LSB is relevant 
	switch (inCtlNum)
	{
		case CC_BANK_LSB:
		mMidiBank = inCtlVal;
		break;
		default:
		break;
	}
}

void onKpaPgmChanged(byte inMidiPgmNum)
{

	Serial.print("onKpaPgmChanged ");
	Serial.print(inMidiPgmNum, HEX);
	Serial.println(" ");


	mMidiPgm = inMidiPgmNum;
	
	mActPgmNum = (mMidiBank * 128) + mMidiPgm;
	
	mActChannel = mActPgmNum % 5;
    mNextPerformance = mActPgmNum / 5;
	mActPerformance = mNextPerformance;
	
	fDisplayPgmInfo();
	
	// as the KPA sends some parameter infos after Pgm Change
	// wait a bit before starting requests
	mPauseRequestsUntil = millis() + PAUSE_REQUESTS;
	mSendInitialRequests = true;
	fResetFxSlotValues();
	fResetParamRequests();
	fSendSlotNameRequest();
}


void fSendSlotNameRequest(){
	byte request[] = { 0xF0, 0x00, 0x20, 0x33, 0x00, 0x00, 0x47, 0x00, 0x0, 0x00, 0x01, 0x00, 0 /*slot nUmber*/, 0xF7 };

	request[12] = mActChannel + 1;

	mKpa.sendSysEx(request, 14);

}

void onKpaStringX(byte inXsb1, byte inXsb2, byte inXsb3, byte inXsb4, byte inXsb5, byte * inSysEx, unsigned int inByteNum){
	Serial.println("StringX: ");
	Serial.print(inXsb1, HEX);
	Serial.print("-");
	Serial.print(inXsb2, HEX);
	Serial.print("-");
	Serial.print(inXsb3, HEX);
	Serial.print("-");
	Serial.print(inXsb4, HEX);
	Serial.print("-");
	Serial.print(inXsb5, HEX);
	Serial.print("->");

	for (size_t i = 0; i < inByteNum; i++)
	{
		//Serial.print(inSysEx[i], HEX);
		Serial.write(inSysEx[i]);
		//Serial.print("-");
	}
	Serial.println();
	
	if (inXsb5 == (mActChannel + 1)){
		
		mFbv.setDisplayTitle(inSysEx);
		mFbv.setLedOnOff(LINE6FBV_DISPLAY, 1);
	}

}



void onKpaSysEx(byte* inSysExData, unsigned inSysExSize)
{
	/*
	if (inSysExData[8] == 0x7C)
		return;
	*/
	Serial.print("onKpaSysEx ");
	for (size_t i = 0; i < inSysExSize; i++){
		Serial.print(inSysExData[i], HEX);
		Serial.print("-");
	}
	
	Serial.println(" ");
}

void onKpaParamSingle(byte inAddrPage, byte inParamNum, byte inMsb, byte inLsb)
{
	
	bool found = false;
	
	if (inAddrPage == 0x7C)
	return;
	
	Serial.print("onKpaParamSingle ");
	Serial.print(inAddrPage, HEX);
	Serial.print(" - ");
	Serial.print(inParamNum, HEX);
	Serial.print(" - ");
	Serial.print(inMsb, HEX);
	Serial.print(" - ");
	Serial.print(inLsb, HEX);
	Serial.println(" ");
	
	for (size_t i = 0; i < PARAM_REQUESTS; i++)
	{
		if (mParamRequests[i].addressPage == inAddrPage){
			if (mParamRequests[i].paramNumber == inParamNum){
				if (mParamRequests[i].isFx){
					for (size_t j = 0; j < FX_SLOTS; j++)
					{
						if (mFxSlots[j].addrPage == mParamRequests[i].addressPage){
							//Serial.println("addr page found");
							switch (mParamRequests[i].paramNumber)
							{
								case KPA_PARAM_NUM_FX_TYPE:
								mFxSlots[j].isEnabled = (inMsb || inLsb);
								fSetLedForFxSlot(j);
								break;
								case KPA_PARAM_NUM_FX_ON:
								case KPA_PARAM_NUM_FX_ON2:
								mFxSlots[j].isOn = inLsb;
								if (mFxSlots[j].isOn){
									if (!mParamRequests[i].received){
										mFxSlots[j].isInitialOn = true;
									}
								}
								fSetLedForFxSlot(j);
								mFbv.syncLedFlash();
								break;
								default:
								break;
							}
						}
					}
				}
				else  // if (mParamRequests[i].isFx)
				{
					switch (inAddrPage)
					{
						case KPA_ADDR_RIG:
						if (inParamNum == KPA_PARAM_NUM_RIG_TEMPO){
							mBPM = (inMsb * 2) + (inLsb / 64);
							fSetTapLed();
						}
						break;
						case 0x7F:  // Tuner on/off
							if (inParamNum == 0x7E){
								mTunerIsOn = (inLsb % 2); // 1 =ON, 2 = OFF

						}
							break;

						case 0x7D:
							break;
						default:
						break;
					}
				}
				mParamRequests[i].received = true;
				found = true;
				i = PARAM_REQUESTS;
			}
		}
	}
	
	if (found){
		mSendNextRequest = true;
		return;
	}
	
	
}


void onKpaParamString(byte inAddrPage, byte inParamNum, byte* inStr, unsigned inStrSize)
{
	/*
	Serial.print("onKpaParamString ");
	Serial.print(inAddrPage, HEX);
	Serial.print(" - ");
	Serial.print(inParamNum, HEX);
	Serial.print(" - :");
	
	for (size_t i = 0; i < inStrSize; i++){
		Serial.write(inStr[i]);
	}
	Serial.println(" ");
	*/
	
	switch (inAddrPage)
	{
	case 0x00:
		switch (inParamNum)
		{
			case KPA_PARAM_NUM_RIG_NAME:
			mFbv.setDisplayTitle(inStr);
			mFbv.setLedOnOff(LINE6FBV_DISPLAY, 1);
		}
	}
}




void fSetTapLed(){
	
	//Serial.print("fSetTapLed "); Serial.println(mBPM);
	int ms;
	
	if (mBPM == 0){
		mFbv.setLedOnOff(LINE6FBV_TAP, false);
	}
	else{
		ms = 60000 / mBPM;
		mFbv.setLedFlash(LINE6FBV_TAP, ms);
	}
	
	
}

void setup() {
	Serial.begin(115200);
	Serial.println("los gehts...");
	
	mFbv.begin(&Serial1); // open port 
	
	// set callback functions for the FBV
	mFbv.setHandleKeyPressed(&onFbvKeyPressed);
	mFbv.setHandleKeyReleased(&onFbvKeyReleased);
	mFbv.setHandleKeyHeld(&onFbvKeyHeld);
	mFbv.setHandleHeartbeat(&onFbvHeartbeat);   
	mFbv.setHandleCtrlChanged(&onFbvCtlChanged);
	
	mFbv.setLedOnOff(LINE6FBV_DISPLAY, 1);
	mFbv.setDisplayTitle("WRBI(AT)ORBI");
	
	mKpa.begin(&Serial3); // open port 
	
	mKpa.setHandleCtlChanged(&onKpaCtlChanged);
	mKpa.setHandlePgmChanged(&onKpaPgmChanged);
	mKpa.setHandleSysEx(&onKpaSysEx);
	mKpa.setHandleParamSingle(&onKpaParamSingle); 
	mKpa.setHandleParamString(&onKpaParamString);
	mKpa.setHandleParamStringX(onKpaStringX);
	
	fInitFxSlots();
	Serial.println("ready");
	
}

void loop() {
	
	fSendRequestsToKpa();
	
	mFbv.read();  // Receive Commands from FBV
	
	mKpa.read();  // Receive Information from KPA
	
	mFbv.updateUI();  // update Display and LEDs on the FBV to the values set in the setDisplay..... routines
}
