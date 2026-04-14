/*
 * lm75.h
 * Created on: 23 Mar 2026
 * Author: Bradley Port
 * Royal Holloway University of London
 * EE3000
 */

#ifndef DRIVERS_LM75_H_
#define DRIVERS_LM75_H_

#include "stm32h7xx_hal.h"

//I2C Addresses - Need to scan I2C to obtain
#define LM75_ADDR1	0x90
#define LM75_ADDR2	0x90
#define LM75_ADDR3	0x90

//Register Addresses
#define LM75_TEMP	0x00
#define LM75_CONF	0x01
#define LM75_THYS	0x02
#define LM75_TOS	0x03

//Configuration register bits
#define LM75_STDN	0x01 //Shutdown
#define LM75_ISR	0x02 //Interrupt
#define LM75_OS		0x04 //Active High or Low
#define LM75_Fault1	0x00
#define LM75_Fault2	0x00
#define LM75_Fault4	0x00
#define LM75_Fault6	0x00

void    LM75_Init(I2C_HandleTypeDef *hi2c);
float   LM75_ReadTemp(void);
void    LM75_SetConfig(uint8_t config);
uint8_t LM75_GetConfig(void);
void    LM75_SetTOS(float temp);
void    LM75_SetTHYST(float temp);
float   LM75_GetTOS(void);
float   LM75_GetTHYST(void);
void    LM75_Shutdown(void);
void    LM75_Wakeup(void);

#endif /* SRC_DRIVERS_LM75_H_ */
