/*
 * audio.h
 *
 *  Created on: 23 Mar 2026
 *      Author: bradp
 */

#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdint.h>
#include "stm32h7xx_hal.h"


//wheel buttons etc.

void Audio_Init(void);
void Audio_SendMute(uint8_t state);
void Audio_SendVolume(uint8_t volume);
void Audio_SendInput(uint8_t source);


#endif /* SRC_VEHICLE_AUDIO_H_ */
