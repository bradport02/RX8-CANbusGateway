/*
 * canbus.c
 * Created on: 22 Mar 2026
 * Bradley Port
 * Royal Holloway University of London
 * EE3100
 */

#include "canbus.h"
#include "main.h"

extern void Report_USB(const char* text);

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

uint8_t uart_rx_buffer[1];
uint8_t msg_buffer[100];
uint8_t canError = 0;
uint8_t can_drop_counter = 0;
uint8_t can_sent_flag = 0;
uint8_t can_sent_counter = 0;
volatile uint8_t msg_index = 0;
volatile uint8_t msg_ready = 0;
uint8_t msg[64];

void CAN_Start(FDCAN_HandleTypeDef *hfdcan_rx, FDCAN_HandleTypeDef *hfdcan_tx){
	  FDCAN_FilterTypeDef sFilterConfig_rx;

	  sFilterConfig_rx.IdType = FDCAN_STANDARD_ID;
	  sFilterConfig_rx.FilterIndex = 0;
	  sFilterConfig_rx.FilterType = FDCAN_FILTER_MASK;
	  sFilterConfig_rx.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	  sFilterConfig_rx.FilterID1 = 0x000; //Accpet all IDs
	  sFilterConfig_rx.FilterID2 = 0x000; //Accept all IDs

	  HAL_FDCAN_ConfigFilter(hfdcan_rx, &sFilterConfig_rx);
	  if (HAL_FDCAN_Start(hfdcan_rx) != HAL_OK)
	        Report_USB("FDCAN1 Start FAILED");
	  /* Start CAN */
	  if (HAL_FDCAN_ActivateNotification(hfdcan_rx, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK){
		  Report_USB("FAILED TO INIT ISR: FDCAN_FIFO_0");
		  canError++;
	  }

	  FDCAN_FilterTypeDef sFilterConfig_tx;

	  sFilterConfig_tx.IdType = FDCAN_STANDARD_ID;
	  sFilterConfig_tx.FilterIndex = 0;
	  sFilterConfig_tx.FilterType = FDCAN_FILTER_MASK;
	  sFilterConfig_tx.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	  sFilterConfig_tx.FilterID1 = 0x000; //Accpet all IDs
	  sFilterConfig_tx.FilterID2 = 0x000; //Accept all IDs

	  HAL_FDCAN_ConfigFilter(hfdcan_tx, &sFilterConfig_tx);
	  //HAL_FDCAN_Start(hfdcan_tx);

	  if (HAL_FDCAN_Start(hfdcan_tx) != HAL_OK)
	        Report_USB("FDCAN2 Start FAILED");
	  if (HAL_FDCAN_ActivateNotification(hfdcan_tx, FDCAN_IT_TX_FIFO_EMPTY, 0) != HAL_OK) {
	      Report_USB("FAILED TO INIT TX NOTIFICATION: FDCAN2");
	      canError++;
	  }
}

void CAN_Send(uint8_t *data, uint16_t std_id){
	/* - Function to send basic can data - A TEST */
	//uint8_t data[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
	//uint16_t std_id = 0x123;
	//uint8_t len = 8;
    FDCAN_TxHeaderTypeDef txHeader = {0};
    txHeader.Identifier = std_id;
    txHeader.IdType = FDCAN_STANDARD_ID;
    txHeader.TxFrameType = FDCAN_DATA_FRAME;
    txHeader.DataLength = FDCAN_DLC_BYTES_8;  // 8-byte CAN
    txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txHeader.BitRateSwitch = FDCAN_BRS_OFF;
    txHeader.FDFormat = FDCAN_CLASSIC_CAN;
    txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;

    if(HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2) > 0)
    {
        HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &txHeader, data);
    }
}

void CAN_CheckFlag(){
	if (can_sent_flag){
		can_sent_flag = 0;
		Report_USB("CAN Frame Sent");
		HAL_Delay(100);
	}
	if (canError > 0){
		canError = 0;
		Report_USB("CAN FRAME DROPPED");
		HAL_Delay(100);
	}
}
