/*
 * protocol.h
 * Created on: 22 Mar 2026
 * Bradley Port
 * Royal Holloway University of London
 * EE3100
 */

#include "protocol.h"
#include "main.h"
#include "usbd_cdc_if.h"
#include <string.h>
#include "lcd.h"
#include "climate.h"
#include "vehicle/audio.h"
#include "canbus.h"
#include "vehicle/time.h"

extern void Report_USB(const char* text);

volatile uint8_t PACKET_READY4 = 0;
uint8_t RxBuf4[RX_PACKET_LEN];

uint8_t HandshakePI = 0;
uint8_t HandshakeESP = 0;

char lcdBuf[13] = {0};
extern char lastLCDMessage[13];

volatile uint8_t PACKET_READY5 = 0;
uint8_t RxBuf5[RX_PACKET_LEN];

uint8_t ambientData[5] = {0};

uint8_t Protocol_CRC8(uint8_t *data, uint8_t len) {
	uint8_t crc = 0x00;
	for (uint8_t i = 0; i < len; i++) {
		crc ^= data[i];
		for (uint8_t j = 0; j < 8; j++) {
			if (crc& 0x80) {
				crc = (crc << 1) ^ 0x07;
			}
			else{
				crc <<= 1;
			}
		}
	}
	return crc;
}

void Protocol_BuildTx(TxPacket_t *pkt, uint8_t start, uint8_t cmd, uint8_t *data, uint8_t len){
	//Fills the whole packet with padding
	memset(pkt, PADDING, sizeof(TxPacket_t));
	pkt->start = start;
	pkt->cmd = cmd;
	pkt->len = len;

	//Chceck packet exists
	if (data != NULL && len > 0){
		//copies whenever is smaller between TX and len
		memcpy(pkt->data, data, len > TX_DATA_LEN ? TX_DATA_LEN : len);
	}

	uint8_t crc_buf[TX_DATA_LEN + 2];
	crc_buf[0] = pkt->cmd;
	crc_buf[1] = pkt->len;
	memcpy(&crc_buf[2], pkt->data, TX_DATA_LEN);
	pkt->crc = Protocol_CRC8(crc_buf, TX_DATA_LEN + 2);

}

void Protocol_Send(UART_HandleTypeDef *huart, uint8_t start, uint8_t cmd, uint8_t *data, uint8_t len){
	TxPacket_t pkt;
	//& = memory address
	Protocol_BuildTx(&pkt, start, cmd, data, len);
	HAL_UART_Transmit(huart, (uint8_t*)&pkt, TX_PACKET_LEN, 100);
}

void Protocol_SendACK(UART_HandleTypeDef *huart, uint8_t start, uint8_t cmd){
	uint8_t ack = ACK;
	Protocol_Send(huart, start, cmd, &ack, 1);
}

void Protocol_SendNACK(UART_HandleTypeDef *huart, uint8_t start, uint8_t cmd){
	uint8_t nack = NACK;
	Protocol_Send(huart, start, cmd, &nack, 1);
}

void Protocol_ProcessPacket(UART_HandleTypeDef *huart, uint8_t *buf){
    if (huart->Instance == UART4){
    	HAL_UARTEx_ReceiveToIdle_DMA(huart, RxBuf4, RX_PACKET_LEN);
    }
    else if (huart->Instance == UART5){
    	HAL_UARTEx_ReceiveToIdle_DMA(huart, RxBuf5, RX_PACKET_LEN);
    }

	RxPacket_t *pkt = (RxPacket_t*)buf;
	RxPacket_t incomingPkt = *pkt;

	if (incomingPkt.start != ESP && incomingPkt.start != PI){
		Report_USB("SYNC LOST — bad start byte, scanning...");
		Protocol_SendNACK(huart, STM, 0x00);
		return;
	}

	if (incomingPkt.cmd == 0x0F && volumeChange){
		return;
	}

	uint8_t crc_buf[RX_DATA_LEN + 2];
	crc_buf[0] = incomingPkt.cmd;
	crc_buf[1] = incomingPkt.len;
	memcpy(&crc_buf[2], incomingPkt.data, RX_DATA_LEN);
	uint8_t expected = Protocol_CRC8(crc_buf, RX_DATA_LEN + 2);

	if (expected != incomingPkt.crc){
		Protocol_SendNACK(huart, STM, incomingPkt.cmd);
		return;
	}

	Protocol_HandlePacket(huart, incomingPkt);

}

void Protocol_HandlePacket(UART_HandleTypeDef *huart, RxPacket_t pkt){
	//Copy of pkt so DMA can be re armed


    //char dbg[32];
    //snprintf(dbg, sizeof(dbg), "CMD: 0x%02X from: 0x%02X", pkt.cmd, pkt.start);
    //Report_USB(dbg);


	switch (pkt.cmd){

		case CMD_HANDSHAKE:
			if(pkt.data[0] == ACK){
				if (pkt.start == PI){
					Report_USB("Handshake Received: Pi");
					HandshakePI = 1;
				}

				if (pkt.start == ESP){
					Report_USB("Handshake Received: ESP");
					HandshakeESP = 1;
				}
			}
			else{
				//Only send an ack if the reply wasnt an ack
				Protocol_SendACK(huart, STM, CMD_HANDSHAKE);
			}
			break;

		case CMD_LOG:
			break;

		case CMD_LCD:
			memset(lcdBuf, 0, sizeof(lcdBuf)); //set buffer to zero
			memcpy(lcdBuf, pkt.data, 12);
			lcdBuf[12] = '\0';
			memcpy(lastLCDMessage, lcdBuf, 12);
			LCD_Print(lcdBuf);
			Protocol_SendACK(huart, STM, CMD_LCD);
			break;

		//Climate Data
		case CMD_TEMP_UP:
			Climate_SendTempIncrease();
			//Climate_SendStatus(huart);
			Protocol_SendACK(huart, STM, CMD_TEMP_UP);
			//Report_USB("Temp INC");
			break;

		case CMD_TEMP_DOWN:
			Climate_SendTempDecrease();
			//Climate_SendStatus(huart);
			Protocol_SendACK(huart, STM, CMD_TEMP_DOWN);
			//Report_USB("Temp DEC");
			break;

		case CMD_FAN_UP:
			Climate_SendFanIncrease();
			Climate_SendStatus(huart);
			//Protocol_SendACK(huart, STM, CMD_FAN_UP);
			//Report_USB("FAN INC");
			break;

		case CMD_FAN_DOWN:
			Climate_SendFanDecrease();
			Climate_SendStatus(huart);
			//Protocol_SendACK(huart, STM, CMD_FAN_DOWN);
			//Report_USB("FAN DEC");
			break;

		case CMD_REAR_DEMIST:
			Climate_SendRearDemist();
			Climate_SendStatus(huart);
			//Protocol_SendACK(huart, STM, pkt.cmd);
			break;

		case CMD_AC_MODE:
			Climate_SendAC();
			Climate_SendStatus(huart);
			//Protocol_SendACK(huart, STM, pkt.cmd);
			break;

		case CMD_VENT_MODE:
			Climate_SendMode();
			Climate_SendStatus(huart);
			//Protocol_SendACK(huart, STM, pkt.cmd);
			//Report_USB("VENT MODE");
			break;

		case CMD_CIRC_MODE:
			Climate_SendAirSource();
			Climate_SendStatus(huart);
			//Protocol_SendACK(huart, STM, pkt.cmd);
			break;

		case CMD_AUTO_MODE:
			Climate_SendAuto();
			Climate_SendStatus(huart);
			//Protocol_SendACK(huart, STM, pkt.cmd);
			break;

		case CMD_CLIMATE_OFF:
			Climate_SendOFF();
			Climate_SendStatus(huart);
			//Protocol_SendACK(huart, STM, pkt.cmd);
			break;

		case CMD_CLIMATE_STATUS:
			Climate_SendStatus(huart);
			//Protocol_SendACK(huart, STM, pkt.cmd);
			break;


		//Audio Data
		case CMD_AUDIO_SRC:
			Audio_SendInput(pkt.data[0]);
			Protocol_SendACK(huart, STM, CMD_AUDIO_SRC);
			break;

		case CMD_VOLUME:
			Audio_SendVolume(pkt.data[0]);
			//Audio_SendVolReply
			Protocol_SendACK(huart, STM, CMD_VOLUME);
						 //volumeChange = 1;           // set flag, don't call I2C here
			//Protocol_SendACK(huart, STM, pkt.cmd);
			//Report_USB("Volume Recieved");
			break;

		case CMD_BEEP_STATE:
			//Potentially not needed
			break;

		case CMD_RADIO:
			break;

		case CMD_CD_DATA:
			break;

		case CMD_CD_STATE:
			break;

		case CMD_ACTIVE_CALL:
			break;

		case CMD_AUDIO_SETTINGS:
			break;

		//Lighting
		case CMD_AMBIENT:
			ambientData[0] = pkt.data[0]; //red
			ambientData[1] = pkt.data[1]; //green
			ambientData[2] = pkt.data[2]; //blue
			ambientData[3] = pkt.data[3]; //brightness
			ambientData[4] = pkt.data[4]; //On Off
			CAN_Send(ambientData,0x420);
			Protocol_SendACK(huart, STM, CMD_AMBIENT);
			char dbg[32];
			snprintf(dbg, sizeof(dbg), "Data: R=0x%02X G=0x%02X B=0x%02X",
			    pkt.data[0],
			    pkt.data[1],
			    pkt.data[2]);
			Report_USB(dbg);
			break;

		case CMD_LCD_BRIGHTNESS:
			break;

		case CMD_REQUESTTIME:
			Time_SendTime(huart);
			break;

		default:
		    Protocol_SendNACK(huart, STM, pkt.cmd);
		    break;
	}
}

void Protocol_CheckFlag(UART_HandleTypeDef *huart4, UART_HandleTypeDef *huart5){
	  if (PACKET_READY4){
		  PACKET_READY4 = 0;
		  //Report_USB("Packet Received: UART4");
		  Protocol_ProcessPacket(huart4, RxBuf4);
		  //HAL_UART_Receive_DMA(huart4, RxBuf4, RX_PACKET_LEN);
	  }

	  if (PACKET_READY5){
		  //Report_USB("Packet");
		  PACKET_READY5 = 0;
		  Protocol_ProcessPacket(huart5, RxBuf5);
		  //HAL_UART_Receive_DMA(huart5, RxBuf5, RX_PACKET_LEN);
	  }
}
