/*
 * protocol.h
 * Created on: 22 Mar 2026
 * Bradley Port
 * Royal Holloway University of London
 * EE3100
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include "stm32h7xx_hal.h"

/*
 * Packet Data
 */

// Device bytes
#define	PI		0xAA
#define STM		0xBB
#define ESP		0xCC

// Response bytes
#define ACK		0x06
#define NACK	0x15
#define PADDING	0x00

// Packet Size
#define TX_DATA_LEN		16
#define RX_DATA_LEN		16
#define TX_PACKET_LEN	20
#define RX_PACKET_LEN	20

/*
 * Command Definitions
 */

// SYSTEM
#define	CMD_HANDSHAKE		0x01
#define CMD_LOG				0x02
#define CMD_LCD				0x03

// CLIMATE
#define CMD_TEMP_UP			0x04
#define CMD_TEMP_DOWN		0x05
#define CMD_FAN_UP			0x06
#define CMD_FAN_DOWN		0x07
#define CMD_REAR_DEMIST		0x08
#define CMD_AC_MODE			0x09
#define CMD_VENT_MODE		0x0A
#define CMD_CIRC_MODE		0x0B
#define CMD_AUTO_MODE		0x0C
#define CMD_CLIMATE_OFF		0x0D
#define CMD_CLIMATE_STATUS	0x0E

// AUDIO
#define CMD_VOLUME			0x0F
#define CMD_BEEP_STATE		0x10
#define CMD_RADIO			0x11
#define CMD_CD_DATA			0x12
#define CMD_CD_STATE		0x13
#define CMD_ACTIVE_CALL		0x14
#define CMD_AUDIO_SETTINGS	0x15
#define CMD_AUDIO_SRC		0x16

// LIGHTING
#define CMD_AMBIENT			0x18
#define CMD_LCD_BRIGHTNESS	0x19
//1A Resereved
//1B Reserved

// SYSTEM
#define CMD_SENDTIME		0x1C
#define CMD_REQUESTTIME		0x1D

// TX Packet structure
typedef struct{
	uint8_t	start;
	uint8_t cmd;
	uint8_t len;
	uint8_t data[TX_DATA_LEN];
	uint8_t crc;
} TxPacket_t;

// TX Packet structure
typedef struct{
	uint8_t	start;
	uint8_t cmd;
	uint8_t len;
	uint8_t data[RX_DATA_LEN];
	uint8_t crc;
} RxPacket_t;


extern volatile uint8_t PACKET_READY4;
extern uint8_t	RxBuf4[RX_PACKET_LEN];
extern volatile uint8_t PACKET_READY5;
extern uint8_t	RxBuf5[RX_PACKET_LEN];
extern uint8_t volumePending;
extern uint8_t volumeChange;
extern uint8_t HandshakePI;
extern uint8_t HandshakeESP;

uint8_t	Protocol_CRC8(uint8_t *data, uint8_t len);
void 	Protocol_BuildTx(TxPacket_t *pkt, uint8_t start, uint8_t cmd, uint8_t *data, uint8_t len);
void	Protocol_Send(UART_HandleTypeDef *huart, uint8_t start, uint8_t cmd, uint8_t *data, uint8_t len);
void 	Protocol_ProcessPacket(UART_HandleTypeDef *huart, uint8_t *buf);
void 	Protocol_HandlePacket(UART_HandleTypeDef *huart, RxPacket_t  pkt);
void	Protocol_SendACK(UART_HandleTypeDef *huart, uint8_t start, uint8_t cmd);
void 	Protocol_SendNACK(UART_HandleTypeDef *huart, uint8_t start, uint8_t cmd);
void 	Protocol_CheckFlag(UART_HandleTypeDef *huart4, UART_HandleTypeDef *huart5);

#endif
