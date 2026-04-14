/*
 * tda7719.c
 * Created on: 23 Mar 2026
 * Author: Bradley Port
 * Royal Holloway University of London
 * EE3000
 */

#include "tda7719.h"

static I2C_HandleTypeDef *_hi2c;

#include <stdio.h>  // for printf

TDA7719_Status TDA7719_ReadStatus(void){
    TDA7719_Status s = {0};
    uint8_t raw = 0;

    HAL_StatusTypeDef result = HAL_I2C_Master_Receive(
        _hi2c,
        TDA7719_ADDR,  // read address
        &raw,
        1,
        HAL_MAX_DELAY
    );

    if(result == HAL_OK){
        s.valid = 1;
        s.muted = (raw >> 0) & 0x01;  // SM bit
        s.busy  = !((raw >> 1) & 0x01); // BZ: 0=busy, invert for clarity
    }
    return s;
}

void TDA7719_WriteRegVerified(uint8_t reg, uint8_t data, const char *name){
    char dbg[64];
    HAL_StatusTypeDef result = HAL_I2C_Mem_Write(
        _hi2c, TDA7719_ADDR, reg,
        I2C_MEMADD_SIZE_8BIT, &data, 1, 50
    );

    snprintf(dbg, sizeof(dbg), "[TDA7719] %s reg=0x%02X data=0x%02X %s\r\n",
        name, reg, data,
        (result == HAL_OK) ? "OK" : "FAIL"
    );
    Report_USB(dbg);
}

//Init I2C
void TDA7719_Init(I2C_HandleTypeDef *hi2c){
	_hi2c = hi2c;
}

void TDA7719_WriteReg(uint8_t reg, uint8_t data){
	//Write to the TDA7719
	HAL_I2C_Mem_Write(
			_hi2c,
			TDA7719_ADDR,
			reg,
			I2C_MEMADD_SIZE_8BIT,
			&data,
			1,
			30
			);
}

void TDA7719_SetInput(int input, int md, int input_gain, int input_conf){
	uint8_t val = 0;
	// |= - or assignment to build a single byte from multiple values
	val |= (input & 0x07);
	val |= (md & 0x01) << 3;
	val |= (input_gain & 0x01) << 4;
	val |= (input_conf & 0x07) << 5;
	TDA7719_WriteReg(TDA7719_INPUT, val);
}

void TDA7719_SetInput2(int sell, int md_2, int input_gain_2, int bypass_front, int bypass_rear, int bypass_sub){
    uint8_t val = 0;
    val |= (sell & 0x07);
    val |= (md_2 & 0x01) << 3;
    val |= (input_gain_2 & 0x01) << 4;
    val |= (bypass_front & 0x01) << 5;
    val |= (bypass_rear & 0x01) << 6;
    val |= (bypass_sub & 0x01) << 7;
    TDA7719_WriteReg(TDA7719_INPUT2, val);
}

void TDA7719_SetMixSource(int mix_sell, int mix_att){
    uint8_t val = 0;
    val |= (mix_sell & 0x07);
    //mix_att is negative — take abs and shift to bits [7:3]
    uint8_t att = (uint8_t)((-mix_att) & 0x1F);
    val |= (att << 3);
    TDA7719_WriteReg(TDA7719_SOURCE, val);
}

void TDA7719_SetMixControl(int mix_fl, int mix_fr, int mix_rl, int mix_rr, int rear_speak, int ref_out_sell, int level_metr, int dc){
    //Mix control / level meter / DC offset
	uint8_t val = 0;
    val |= (mix_fl & 0x01);
    val |= (mix_fr & 0x01) << 1;
    val |= (mix_rl & 0x01) << 2;
    val |= (mix_rr & 0x01) << 3;
    val |= (rear_speak & 0x01) << 4;
    val |= (ref_out_sell & 0x01) << 5;
    val |= (level_metr & 0x01) << 6;
    val |= (dc & 0x01) << 7;
    TDA7719_WriteReg(TDA7719_CONTROL, val);
}

void TDA7719_SetMute(int mute, int pin_mute, int time_mute, int sub_in_conf, int sub_enable, int fast, int filter){
    uint8_t val = 0;
    val |= (mute & 0x01);
    val |= (pin_mute & 0x01) << 1;
    val |= (time_mute & 0x03) << 2;
    val |= (sub_in_conf & 0x01) << 4;
    val |= (sub_enable & 0x01) << 5;
    val |= (fast & 0x01) << 6;
    val |= (filter & 0x01) << 7;
    TDA7719_WriteReg(TDA7719_MUTE, val);
}

void TDA7719_SetSoft1(int soft_loun, int soft_vol, int soft_treb, int soft_mid, int soft_bass, int soft_lf,int soft_fr, int soft_lr){
    uint8_t val = 0;
    val |= (soft_loun & 0x01);
    val |= (soft_vol  & 0x01) << 1;
    val |= (soft_treb & 0x01) << 2;
    val |= (soft_mid  & 0x01) << 3;
    val |= (soft_bass & 0x01) << 4;
    val |= (soft_lf   & 0x01) << 5;
    val |= (soft_fr   & 0x01) << 6;
    val |= (soft_lr   & 0x01) << 7;  // Note: matches original library bit position
    TDA7719_WriteReg(TDA7719_SOFT1, val);
}

void TDA7719_SetSoft2(int soft_rr, int soft_sub_l, int soft_sub_r, int soft_time, int soft_zero, int soft_time_cons){
    uint8_t val = 0;
    val |= (soft_rr & 0x01);
    val |= (soft_sub_l & 0x01) << 1;
    val |= (soft_sub_r & 0x01) << 2;
    val |= (soft_time & 0x01) << 3;
    val |= (soft_zero & 0x03) << 4;
    val |= (soft_time_cons & 0x03) << 6;
    TDA7719_WriteReg(TDA7719_SOFT2, val);
}

void TDA7719_SetLoudness(int loud_att, int loud_f, int loud_b, int loud_s){
    uint8_t val = 0;
    val |= ((uint8_t)(-loud_att) & 0x0F);
    val |= (loud_f & 0x03) << 4;
    val |= (loud_b & 0x01) << 6;
    val |= (loud_s & 0x01) << 7;
    TDA7719_WriteReg(TDA7719_LOUD, val);
}

static uint8_t encodeGain(int gain){
    if(gain < 0){
        return (uint8_t)(gain + 15);    // -15→0x00, -1→0x0E, 0→0x0F
    }
    else if(gain == 0){
        return 0x0F;                    // 0dB attenuation side
    }
    else{
        return (uint8_t)(0x3F - gain);  // +1→0x3E, +10→0x35, +15→0x30
    }
}

void TDA7719_SetVolume(int gain, int out_gain, int soft_step){
    uint8_t val = encodeGain(gain);
    val |= (out_gain & 0x01) << 6;
    val |= (soft_step & 0x01) << 7;
    // Debug — use verified write so we can see what's being sent
    TDA7719_WriteRegVerified(TDA7719_VOL, val, "SetVolume");
}

void TDA7719_SetTreble(int gain_treb, int treb_f, int soft_treb){
    uint8_t val = encodeGain(gain_treb);
    val |= (treb_f & 0x03) << 5;
    val |= (soft_treb & 0x01) << 7;
    TDA7719_WriteReg(TDA7719_TREBLE, val);
}

void TDA7719_SetMiddle(int gain_mid, int mid_q, int soft_mid){
    uint8_t val = encodeGain(gain_mid);
    val |= (mid_q & 0x03) << 5;
    val |= (soft_mid & 0x01) << 7;
    TDA7719_WriteReg(TDA7719_MIDDLE, val);
}

void TDA7719_SetBass(int gain_bass, int bass_q, int soft_bass){
    uint8_t val = encodeGain(gain_bass);
    val |= (bass_q & 0x03) << 5;
    val |= (soft_bass & 0x01) << 7;
    TDA7719_WriteReg(TDA7719_BASS, val);
}

void TDA7719_SetSMB(int sub_f, int sub_out, int mid_f, int bass_f, int bass_dc){
	//set sub / mid / bass
    uint8_t val = 0;
    val |= (sub_f & 0x03);
    val |= (sub_out & 0x01) << 2;
    val |= (mid_f & 0x03) << 3;
    val |= (bass_f & 0x03) << 5;
    val |= (bass_dc & 0x01) << 7;
    TDA7719_WriteReg(TDA7719_SUB_M_B, val);
}

static uint8_t encodeAtt(int att){
	//helper function for speaker attenuation
    return (uint8_t)((-att) + 0x10);
}

// Registers 13-16 — Speaker attenuation
void TDA7719_SetVolLF(int lf, int soft_lf){
    uint8_t val = encodeAtt(lf) | ((soft_lf & 0x01) << 7);
    TDA7719_WriteReg(TDA7719_ATT_LF, val);
}

void TDA7719_SetVolRF(int rf, int soft_rf){
    uint8_t val = encodeAtt(rf) | ((soft_rf & 0x01) << 7);
    TDA7719_WriteReg(TDA7719_ATT_RF, val);
}

void TDA7719_SetVolLR(int lr, int soft_lr){
    uint8_t val = encodeAtt(lr) | ((soft_lr & 0x01) << 7);
    TDA7719_WriteReg(TDA7719_ATT_LR, val);
}

void TDA7719_SetVolRR(int rr, int soft_rr){
    uint8_t val = encodeAtt(rr) | ((soft_rr & 0x01) << 7);
    TDA7719_WriteReg(TDA7719_ATT_RR, val);
}

void TDA7719_SetVolSubL(int sl, int soft_sl){
    uint8_t val = encodeAtt(sl) | ((soft_sl & 0x01) << 7);
    TDA7719_WriteReg(TDA7719_SUBL, val);
}

void TDA7719_SetVolSubR(int sr, int soft_sr){
    uint8_t val = encodeAtt(sr) | ((soft_sr & 0x01) << 7);
    TDA7719_WriteReg(TDA7719_SUBR, val);
}

void TDA7719_SetTest1(int x0, int x1, int x2, int x3){
    uint8_t val = 0;
    val |= (x0 & 0x01);
    val |= (x1 & 0x0F) << 1;
    val |= (x2 & 0x01) << 5;
    val |= (x3 & 0x01) << 6;
    TDA7719_WriteReg(TDA7719_TEST1, val);
}

void TDA7719_SetTest2(int y0, int y1, int y2, int y3){
    uint8_t val = 0;
    val |= (y0 & 0x01);
    val |= (y1 & 0x01) << 1;
    val |= (y2 & 0x01) << 2;
    val |= (y3 & 0x03) << 3;
    TDA7719_WriteReg(TDA7719_TEST2, val);
}




