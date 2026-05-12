#include "protocol.h"

uint8_t data[16] = {0};  // zero padded
uint8_t len = 0;

void Send_LCD(const char* text)
{
    data[16] = {0};  // zero padded
    len = strlen(text);
    if (len > 12) len = 12;  // clamp to 12 chars max
    memcpy(data, text, len);
    Protocol_Send(&Serial1, ESP, CMD_LCD, data, len);
    Protocol_WaitForACK(CMD_LCD);
}

void Send_TempUp(){
    data[16] = {0};  // zero padded
    Protocol_Send(&Serial1, ESP, CMD_TEMP_UP, data, 0);
    Protocol_WaitForACK(CMD_TEMP_UP);
}

void Send_TempDown(){
    data[16] = {0};  // zero padded
    Protocol_Send(&Serial1, ESP, CMD_TEMP_DOWN, data, 0);
    Protocol_WaitForACK(CMD_TEMP_DOWN);
}

void Send_FanUp(){
    data[16] = {0};  // zero padded
    Protocol_Send(&Serial1, ESP, CMD_FAN_UP, data, 0);
    Protocol_WaitForACK(CMD_FAN_UP);
}

void Send_FanDown(){
    data[16] = {0};  // zero padded
    Protocol_Send(&Serial1, ESP, CMD_FAN_DOWN, data, 0);
    Protocol_WaitForACK(CMD_FAN_DOWN);
}

void Send_ClimateOFF(){
    data[16] = {0};  // zero padded
    Protocol_Send(&Serial1, ESP, CMD_CLIMATE_OFF, data, 0);
    Protocol_WaitForACK(CMD_CLIMATE_OFF);
}

void Send_VentMode(){
    data[16] = {0};  // zero padded
    Protocol_Send(&Serial1, ESP, CMD_VENT_MODE, data, 0);
    Protocol_WaitForACK(CMD_VENT_MODE);
}

void Climate_SendAuto(void){
    data[16] = {0};  // zero padded
    Protocol_Send(&Serial1, ESP, CMD_AUTO_MODE, data, 0);
    Protocol_WaitForACK(CMD_AUTO_MODE);
}

void Climate_SendAC(void);
void Climate_SendFrontDemist(void);
void Climate_SendRearDemist(void);
void Climate_SendAirSource(void);
void Climate_SendAmbientToggle(){
    data[16] = {0};  // zero padded
    Protocol_Send(&Serial1, ESP, CMD_AMBIENT, data, 0);
    Protocol_WaitForACK(CMD_AMBIENT);
}