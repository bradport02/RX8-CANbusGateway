/*
 * lm75.c
 * Created on: 23 Mar 2026
 * Author: Bradley Port
 * Royal Holloway University of London
 * EE3000
 *
 * Driver for the LM75 Temperature Sensors
 */

#include "lm75.h"

static I2C_HandleTypeDef *_hi2c;

void LM75_Init(I2C_HandleTypeDef *hi2c){
	_hi2c = hi2c;
}

static int16_t LM75_ReadReg16(uint8_t reg){
	uint8_t buf[2] = {0};
	HAL_I2C_Mem_Read(
			_hi2c,
			LM75_ADDR1,
			reg,
			I2C_MEMADD_SIZE_8BIT,
			buf,
			2,
			HAL_MAX_DELAY
			);
	//MSB = buf[0]
	return (int16_t)((buf[0] << 8) | buf[1]);
}

static void LM75_Write_Reg16(uint8_t reg, int16_t value){
	uint8_t buf[2];
	buf[0] = (value >> 8) & 0xFF;
	buf[1] = value & 0xFF;
	HAL_I2C_Mem_Write(
			_hi2c,
			LM75_ADDR1,
			reg,
			I2C_MEMADD_SIZE_8BIT,
			buf,
			2,
			HAL_MAX_DELAY
			);

}


float LM75_ReadTemp(void){
    int16_t raw = LM75_ReadReg16(LM75_TEMP);

    //Lower 7 bits are don't care — shift right to align
    //D15-D7 are the 9 valid bits
    raw >>= 7;

    //multiply by 0.5 to convert LSB to degrees
    return raw * 0.5f;
}

void LM75_SetConfig(uint8_t config){
    //Datasheet: D7, D6, D5 must be written as 0
    config &= 0x1F;
    HAL_I2C_Mem_Write(
        _hi2c,
        LM75_ADDR1,
        LM75_CONF,
        I2C_MEMADD_SIZE_8BIT,
        &config,
        1,
        HAL_MAX_DELAY
    );
}

uint8_t LM75_GetConfig(void){
	//read configuration values
    uint8_t config = 0;
    HAL_I2C_Mem_Read(
        _hi2c,
        LM75_ADDR1,
        LM75_CONF,
        I2C_MEMADD_SIZE_8BIT,
        &config,
        1,
        HAL_MAX_DELAY
    );
    return config;
}

static int16_t LM75_TempToReg(float temp){
    //Multiply by 2 to work in 0.5°C steps, then shift left 7
    int16_t val = (int16_t)(temp * 2.0f);
    return (val << 7);
}
void LM75_SetTOS(float temp){
	//Set overtemperature limit
	//default = +80deg
    LM75_WriteReg16(LM75_TOS, LM75_TempToReg(temp));
}

void LM75_SetTHYST(float temp){
	//Set hystersis threshold
	//default +75deg
    LM75_WriteReg16(LM75_THYS, LM75_TempToReg(temp));
}

float LM75_GetTOS(void){
    int16_t raw = LM75_ReadReg16(LM75_TOS);
    raw >>= 7;
    return raw * 0.5f;
}

float LM75_GetTHYST(void){
    int16_t raw = LM75_ReadReg16(LM75_THYS);
    raw >>= 7;
    return raw * 0.5f;
}

void LM75_Shutdown(void){
	//Shutdown can be used to reduce current useage
    uint8_t config = LM75_GetConfig();
    config |= LM75_STDN;
    LM75_SetConfig(config);
}

void LM75_Wakeup(void){
	//wakeup from shutdown
    uint8_t config = LM75_GetConfig();
    config &= ~LM75_STDN;
    LM75_SetConfig(config);
}

