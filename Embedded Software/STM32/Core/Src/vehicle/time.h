/*
 * time.h
 *
 *  Created on: 3 Apr 2026
 *      Author: bradp
 */

#ifndef VEHICLE_TIME_H_
#define VEHICLE_TIME_H_

#include "stm32h7xx_hal.h"
#include <string.h>
#include <stdio.h>

void Time_SendTime(UART_HandleTypeDef *huart);
void Time_GetTime(void);

typedef struct {
	uint8_t	hours;
	uint8_t minutes;
	uint8_t seconds;
	uint8_t day;
	uint8_t month;
	uint16_t year;
	uint8_t format; // 24 = 0, 12 = 1
} time_t;


#endif /* SRC_VEHICLE_TIME_H_ */
