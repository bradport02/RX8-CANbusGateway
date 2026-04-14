/*
 * climate.h
 * Created on: 24 Mar 2026
 * Author: Bradley Port
 * Royal Holloway University of London
 * EE3100
 */

#ifndef COMMUNICATION_CLIMATE_H_
#define COMMUNICATION_CLIMATE_H_

#include "stm32h7xx_hal.h"
#include <string.h>
#include <stdio.h>

// -------------------------------------------------------
// Packet lengths
// -------------------------------------------------------
#define AC_PACKET_LEN       6
#define AC_TX_PACKET_LEN    5

// -------------------------------------------------------
// Enums
// -------------------------------------------------------
typedef enum {
    VENT_FACE           = 0,
    VENT_FEET_DEMIST    = 1,
    VENT_FEET_FACE      = 2,
    VENT_FEET           = 3
} VentMode_t;

typedef enum {
    CIRC_FRESH_AIR      = 0,
    CIRC_RECIRCULATE    = 1
} CircMode_t;

typedef enum {
    SYS_AMBIENT         = 0,
    SYS_TEMPERATURE     = 1,
    SYS_OFF             = 2
} SystemMode_t;

typedef enum {
    AC_CMD_DEFAULT      = 0,
    AC_CMD_FAN_INCREASE = 1,
    AC_CMD_FAN_DECREASE = 2
} ACCommand_t;

// -------------------------------------------------------
// Parsed AC state
// -------------------------------------------------------
typedef struct {
    uint8_t      tempTens;
    uint8_t      tempUnits;
    uint8_t      tempDecimal;
    uint8_t      fanSpeed;
    SystemMode_t sysMode;
    VentMode_t   ventMode;
    CircMode_t   circMode;
    uint8_t 	 demistDependency;
    uint8_t      autoEnabled;
    uint8_t      acEnabled;
    uint8_t      ecoEnabled;
    uint8_t      demistFront;
    uint8_t      demistRear;
    uint8_t      valid;
} ACState_t;

// -------------------------------------------------------
// Extern — needed by main.c callback
// -------------------------------------------------------
extern uint8_t  ClimatePacketReady;
extern uint8_t  ClimateBuf[AC_PACKET_LEN];
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;

// -------------------------------------------------------
// Function prototypes
// -------------------------------------------------------

void Climate_SendOFF(void);
void Climate_SendDefault(void);
void Climate_SendFanIncrease(void);
void Climate_SendFanDecrease(void);
void Climate_SendTempIncrease(void);
void Climate_SendTempDecrease(void);
void Climate_SendAuto(void);
void Climate_SendMode(void);
void Climate_SendAC(void);
void Climate_SendFrontDemist(void);
void Climate_SendRearDemist(void);
void Climate_SendAirSource(void);
void Climate_SendAmbientToggle(void);
void Climate_SetLCD(ACState_t *s);

void        Climate_Init(UART_HandleTypeDef *huart);
void        Climate_CheckFlag(void);
uint8_t     Climate_PacketReady(void);
ACState_t   Climate_GetState(void);
void        Climate_ClearReady(void);
void        Climate_PrintState(ACState_t *state);

// TX
void        Climate_Send(uint8_t *packet);
void        Climate_SendCommand(ACCommand_t cmd);
void 		Climate_PrintStateSingle();

void Climate_SendStatus(UART_HandleTypeDef *huart);

#endif /* COMMUNICATION_CLIMATE_H_ */
