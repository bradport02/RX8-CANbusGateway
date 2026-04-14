/*
 * canbus.h
 * Created on: 22 Mar 2026
 * Bradley Port
 * Royal Holloway University of London
 * EE3100
 */

#ifndef SRC_COMMUNICATION_CANBUS_H_
#define SRC_COMMUNICATION_CANBUS_H_

#include "stm32h7xx_hal.h"  // ← needed for FDCAN_HandleTypeDef

extern uint8_t canError;
extern uint8_t can_drop_counter;
extern uint8_t can_sent_flag;
extern uint8_t can_sent_counter;

void CAN_Start(FDCAN_HandleTypeDef *hfdcan_rx, FDCAN_HandleTypeDef *hfdcan_tx);
void CAN_SendAmbient(uint8_t red, uint8_t green, uint8_t blue, uint16_t std_id);
void CAN_Send(uint8_t *data, uint16_t std_id);
void CAN_CheckFlag(void);


#endif /* SRC_COMMUNICATION_CANBUS_H_ */
