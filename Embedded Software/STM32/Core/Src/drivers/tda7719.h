/*
 * tda7199.h
 * Created on: 23 Mar 2026
 * Author: Bradley Port
 * Royal Holloway University of London
 * EE3000
 */

#ifndef DRIVERS_TDA7719_H_
#define DRIVERS_TDA7719_H_

#include "stm32h7xx_hal.h"

//I2C Addess
#define TDA7719_ADDR	0x88

//Register Subaddresses
#define TDA7719_INPUT	0x00
#define TDA7719_INPUT2	0x01
#define TDA7719_SOURCE	0x02
#define TDA7719_CONTROL	0x03
#define TDA7719_MUTE	0x04
#define TDA7719_SOFT1	0x05
#define TDA7719_SOFT2	0x06
#define TDA7719_LOUD	0x07
#define TDA7719_VOL		0x08
#define TDA7719_TREBLE	0x09
#define TDA7719_MIDDLE	0x0A
#define TDA7719_BASS	0x0B
#define TDA7719_SUB_M_B	0x0C
#define TDA7719_ATT_LF	0x0D
#define TDA7719_ATT_RF	0x0E
#define TDA7719_ATT_LR	0x0F
#define TDA7719_ATT_RR	0x10
#define TDA7719_SUBL	0x11
#define TDA7719_SUBR	0x12
#define TDA7719_TEST1	0x13
#define TDA7719_TEST2	0x14

// Status flags returned by TDA7719_ReadStatus
typedef struct {
    uint8_t muted;    // 1 = soft mute active
    uint8_t busy;     // 1 = softstep in progress
    uint8_t valid;    // 1 = I2C read succeeded
} TDA7719_Status;

TDA7719_Status TDA7719_ReadStatus(void);
void TDA7719_WriteRegVerified(uint8_t reg, uint8_t data, const char *name);
void TDA7719_Init(I2C_HandleTypeDef *h12c);
void TDA7719_WriteReg(uint8_t reg, uint8_t data);
void TDA7719_SetInput(int input, int md, int input_gain, int input_conf);
void TDA7719_SetInput2(int sell, int md_2, int input_gain_2, int bypass_front, int bypass_rear, int bypass_sub);
void TDA7719_SetMixSource(int mix_sell, int mix_att);
void TDA7719_SetMixControl(int mix_fl, int mix_fr, int mix_rl, int mix_rr, int rear_speak, int ref_out_sell, int level_metr, int dc);
void TDA7719_SetMute(int mute, int pin_mute, int time_mute, int sub_in_conf, int sub_enable, int fast, int filter);
void TDA7719_SetSoft1(int soft_loun, int soft_vol, int soft_treb, int soft_mid, int soft_bass, int soft_lf, int soft_fr, int soft_lr);
void TDA7719_SetSoft2(int soft_rr, int soft_sub_l, int soft_sub_r, int soft_time, int soft_zero, int soft_time_cons);
void TDA7719_SetLoudness(int loud_att, int loud_f, int loud_b, int loud_s);
void TDA7719_SetVolume(int gain, int out_gain, int soft_step);
void TDA7719_SetTreble(int gain_treb, int treb_f, int soft_treb);
void TDA7719_SetMiddle(int gain_mid, int mid_q, int soft_mid);
void TDA7719_SetBass(int gain_bass, int bass_q, int soft_bass);
void TDA7719_SetSMB(int sub_f, int sub_out, int mid_f, int bass_f, int bass_dc);
void TDA7719_SetVolLF(int lf, int soft_lf);
void TDA7719_SetVolRF(int rf, int soft_rf);
void TDA7719_SetVolLR(int lr, int soft_lr);
void TDA7719_SetVolRR(int rr, int soft_rr);
void TDA7719_SetVolSubL(int sl, int soft_sl);
void TDA7719_SetVolSubR(int sr, int soft_sr);
void TDA7719_SetTest1(int x0, int x1, int x2, int x3);
void TDA7719_SetTest2(int y0, int y1, int y2, int y3);
#endif /* SRC_DRIVERS_TDA7719_H_ */
