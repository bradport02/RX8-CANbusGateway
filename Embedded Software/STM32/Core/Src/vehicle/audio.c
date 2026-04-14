/*
 * audio.c
 *
 *  Created on: 23 Mar 2026
 *      Author: bradp
 */


/*
 * Input Order:
 * 0 - PI
 * 1 - CD
 * 2 - FM
 * 3 - AUX
 */

#include "drivers/tda7719.h"
#include "main.h"
#include "audio.h"
#include "communication/lcd.h"
#include <stdio.h>
#include <string.h>

extern void Report_USB(const char* text);

int8_t previousVolume= 0;
extern uint8_t lcdReset;
extern uint32_t lcdResetTimer;
extern char lcdBuf[13];
extern char lastLCDMessage[13];

void Audio_Init(void) {
	/*
	// In your init code, before any audio setup
	HAL_GPIO_WritePin(GPIOE, AMP_MUTE_Pin, GPIO_PIN_SET);  // Unmute amp
	HAL_GPIO_WritePin(GPIOE, AMPSTATE_Pin, GPIO_PIN_RESET);
    // 1. Select input source (Pi = 0)
    TDA7719_SetInput(4, 0, 0, 0);        // input 0, single source, no boost, CFG0

    // 2. Unmute everything
    TDA7719_SetMute(1, 0, 0, 0, 0, 0, 0); // mute=0, all defaults

    // 3. Master volume — 0dB
    TDA7719_SetVolume(0, 0, 0);           // 0dB, no out boost, no soft step

    // 4. Speaker attenuators — open all four channels to 0dB
    TDA7719_SetVolLF(0, 0);
    TDA7719_SetVolRF(0, 0);
    TDA7719_SetVolLR(0, 0);
    TDA7719_SetVolRR(0, 0);
    */

	HAL_GPIO_WritePin(GPIOE, AMP_MUTE_Pin, GPIO_PIN_SET);  // Unmute amp
	HAL_GPIO_WritePin(GPIOE, AMPSTATE_Pin, GPIO_PIN_SET);
	// Use verified writes during init to confirm everything lands
	// Disable all bypass — INPUT2 register
	//TDA7719_WriteRegVerified(TDA7719_INPUT2, 0x00, "Input2 bypass OFF");
	//TDA7719_WriteRegVerified(TDA7719_INPUT, 0x01, "Input");
	//TDA7719_WriteRegVerified(TDA7719_MUTE,  0x00, "Mute OFF");  // 1 = unmuted
	TDA7719_WriteRegVerified(TDA7719_LOUD,  0x0F, "LOUD");
	//Audio_SendVolume(0);
	//TDA7719_WriteReg(TDA7719_VOL, 0); //volume to half (15 on pi)
	TDA7719_SetMute(1, 0, 2, 1, 0, 0, 1);
	TDA7719_SetMixControl(1,1,1,1,1,0,1,0);
	TDA7719_SetInput(0,0,0,0);
	TDA7719_SetInput2(0,0,0,1,1,1);


	// Subaddress 0x05 - Volume softstep ON, everything else OFF
	TDA7719_SetSoft1(
	    0,  // soft_loun  - Loudness   OFF
	    0,  // soft_vol   - Volume     ON  ← only this enabled
	    0,  // soft_treb  - Treble     OFF
	    0,  // soft_mid   - Middle     OFF
	    0,  // soft_bass  - Bass       OFF
	    0,  // soft_lf    - Speaker LF OFF
	    0,  // soft_fr    - Speaker RF OFF
	    0   // soft_lr    - Speaker LR OFF
	);

	// Subaddress 0x06 - RR/Sub off, 5ms step, default DC detector settings
	TDA7719_SetSoft2(
	    0,  // soft_rr        - Speaker RR    OFF
	    0,  // soft_sub_l     - Subwoofer L   OFF
	    0,  // soft_sub_r     - Subwoofer R   OFF
	    1,  // soft_time      - 5ms per step  (0=5ms, 1=10ms)
	    0,  // soft_zero      - ±100mV window (0=100, 1=75, 2=50)
	    0   // soft_time_cons - 44µs spike TC (default, underlined)
	);

	//TDA7719_SetLoudness(0, 0, 0, 0);
	Audio_SendVolume(15);
	//TDA7719_WriteRegVerified(TDA7719_SOFT1, 0x0A, "Soft");
	//TDA7719_WriteRegVerified(TDA7719_SOFT2, 0x0A, "Soft");
	//TDA7719_WriteRegVerified(TDA7719_VOL,   0x0F, "Volume 0dB");
	//TDA7719_SetVolume(0, 0, 0);
	TDA7719_WriteRegVerified(TDA7719_ATT_LF, 0x0F, "Att LF 0dB");
	TDA7719_WriteRegVerified(TDA7719_ATT_RF, 0x0F, "Att RF 0dB");
	TDA7719_WriteRegVerified(TDA7719_ATT_LR, 0x0F, "Att LR 0dB");
	TDA7719_WriteRegVerified(TDA7719_ATT_RR, 0x0F, "Att RR 0dB");

	// Then check status
	TDA7719_Status s = TDA7719_ReadStatus();
    char dbg[64];
	if(s.valid){
	    snprintf(dbg, sizeof(dbg), "[TDA7719] Status: muted=%d busy=%d\r\n", s.muted, s.busy);
	    Report_USB(dbg);
	} else {
		Report_USB("Read: Failed");
	    //printf("[TDA7719] Status read FAILED - I2C issue\r\n");
	}
}

void Audio_SendVolume(uint8_t volume){
	if (volume < 0) volume = 0;
    if (volume >  30) volume =  30;

    if (previousVolume == 0 && volume == 0){
    	return;
    	//save i2c writes
    }

    if (volume == 0){
    	//Audio_SendMute(1);
    	TDA7719_SetMute(0,0,2,1,0,0,1);
    	//Report_USB("MUTED");
    }

    if (previousVolume == 0 && volume > 0){
    	TDA7719_SetMute(1,0,2,1,0,0,1);
    }

    /*
    if (previousVolume <= 15 && volume > 15){
        TDA7719_WriteReg(TDA7719_VOL, 0x0F);  // slider 15 = 0x0F
        HAL_Delay(80);  // wait for softstep to reach 0dB (15 steps x 5ms)
    }
    else if (previousVolume > 15 && volume <= 15){
        TDA7719_WriteReg(TDA7719_VOL, 0x3F);  // slider 15 = 0x0F
        HAL_Delay(80);  // wait for softstep to reach 0dB (15 steps x 5ms)
    }
    */

    /*
    if (previousVolume < volume){
		for (int currentVolume = (previousVolume + 1); currentVolume <= volume; currentVolume++){
			//currentVolume++; //too choppy
			if (currentVolume > 15){
				TDA7719_WriteReg(TDA7719_VOL, 63 - (currentVolume - 15));
			}
			else{
				TDA7719_WriteReg(TDA7719_VOL, currentVolume);
			}
			HAL_Delay(50);
		}
    }
    else{
		for (int currentVolume = (previousVolume - 1); currentVolume >= volume; currentVolume--){
			//currentVolume--; //too choppy
			if (currentVolume > 15){
				TDA7719_WriteReg(TDA7719_VOL, 63 - (currentVolume - 15));
			}
			else{
				TDA7719_WriteReg(TDA7719_VOL, currentVolume);
			}
			HAL_Delay(50);
		}
    }
    */

    previousVolume = volume;

    //Handle LCD Volume Expression
	lcdReset = 1;
	lcdResetTimer = HAL_GetTick();
	snprintf(lcdBuf, sizeof(lcdBuf), "Volume: %d",volume);
	LCD_Print(lcdBuf);

    if (volume > 15){
    	volume = 63 - (volume - 15);
    }
    //TDA7719_SetVolume(volume, 0, 0);
    TDA7719_WriteReg(TDA7719_VOL, volume);
}

void Audio_SendInput(uint8_t source){
    /*
     * 0 = Pi  (DAC  — QD1)
     * 1 = CD        — QD2
     * 2 = Radio(FM) — QD3
     * 3 = AUX       — QD4
     */
	TDA7719_SetInput(source,0,0,0);
	TDA7719_SetInput2(source, 0, 0, 1, 1, 1);
	Report_USB("Setting source");
}
