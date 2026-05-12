/*
commands.cpp
23/03/2026
Bradley Port
Royal Holloway University of London
EE3000

commands the esp can call to send data to stm or pi
*/

#ifndef COMMANDS_H
#define COMMANDS_H

#include <Arduino.h>

// Function Prototypes (using HardwareSerial instead of UART_HandleTypeDef)
void Send_LCD(const char* text);
void Send_ClimateOFF(void);
void Send_TempUp(void);
void Send_TempDown(void);
void Send_FanUp(void);
void Send_FanDown(void);
void Send_VentMode(void);
void Climate_SendAuto(void);
void Climate_SendAC(void);
void Climate_SendFrontDemist(void);
void Climate_SendRearDemist(void);
void Climate_SendAirSource(void);
void Climate_SendAmbientToggle(void);
#endif