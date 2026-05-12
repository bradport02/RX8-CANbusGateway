/*
protocol.cpp
23/03/2026
Bradley Port
Royal Holloway University of London
EE3000

File to interperate the protocol defined. 
*/

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>

// Device bytes
#define PI      0xAA
#define STM     0xBB
#define ESP     0xCC

// Response bytes
#define ACK     0x06
#define NACK    0x15
#define PADDING 0x00

// Packet Size
#define TX_DATA_LEN     16 // STM32 payload size
#define RX_DATA_LEN     16  // ESP32 payload size
#define STM_PACKET_LEN  20 // What the STM32 sends
#define ESP_PACKET_LEN  20 // What the ESP32 sends

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

// AUDIO
#define CMD_AUDIO_SRC		0x0E
#define CMD_VOLUME			0x0F
#define CMD_BEEP_STATE		0x10
#define CMD_RADIO			0x11
#define CMD_CD_DATA			0x12
#define CMD_CD_STATE		0x13
#define CMD_ACTIVE_CALL		0x14
#define CMD_AUDIO_SETTINGS	0x15

// LIGHTING
#define CMD_AMBIENT			0x18
#define CMD_LCD_BRIGHTNESS	0x19
// ... (Add the rest of your CMD defines here) ...

/* * PACKING IS CRITICAL ON ESP32 
 * This stops the compiler from adding hidden bytes to your struct
 */
#pragma pack(push, 1)

// 20-Byte Packet (Received from STM32)
typedef struct {
    uint8_t start;
    uint8_t cmd;
    uint8_t len;
    uint8_t data[TX_DATA_LEN];
    uint8_t crc;
} StmPacket_t;

// 12-Byte Packet (Transmitted by ESP32)
typedef struct {
    uint8_t start;
    uint8_t cmd;
    uint8_t len;
    uint8_t data[RX_DATA_LEN];
    uint8_t crc;
} EspPacket_t;

#pragma pack(pop)

// Function Prototypes (using HardwareSerial instead of UART_HandleTypeDef)
uint8_t Protocol_CRC8(uint8_t *data, uint8_t len);
void Protocol_BuildTx(EspPacket_t *pkt, uint8_t start, uint8_t cmd, uint8_t *data, uint8_t len);
void Protocol_Send(HardwareSerial *serial, uint8_t start, uint8_t cmd, uint8_t *data, uint8_t len);
void Protocol_SendACK(HardwareSerial *serial, uint8_t start, uint8_t cmd);
void Protocol_SendNACK(HardwareSerial *serial, uint8_t start, uint8_t cmd);
void Protocol_ProcessPacket(HardwareSerial *serial, uint8_t *buf);
void Protocol_HandlePacket(HardwareSerial *serial, StmPacket_t *pkt);
void Protocol_PollSerial(HardwareSerial *serial); // Replaces CheckFlag
void Protocol_WaitForACK(uint8_t cmd);

#endif