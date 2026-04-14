/*
 * climate.c
 * Created on: 24 Mar 2026
 * Author: Bradley Port
 * Royal Holloway University of London
 * EE3100
 *
 * STM32 HAL port of RX-8 AC serial protocol
 * Original reverse engineering: Arduino ESP32 code
 */

#include "climate.h"
#include "lcd.h"
#include "protocol.h"
#include "main.h"

extern void Report_USB(const char* text);

static UART_HandleTypeDef *_huart;
                  // single byte DMA target
uint8_t  ClimatePacketReady = 0;
ACState_t acState        = {0};
static ACState_t s		        = {0};
uint8_t ClimateBuf[AC_PACKET_LEN];
uint8_t lastPKT[AC_PACKET_LEN] = {0};
uint8_t lastSend = 0;
uint8_t climateReceived = 0;


//Climate Send Data
static const uint8_t AC_OFF[]         		= {0x04, 0x81, 0x80, 0x80, 0xFB};
static const uint8_t AC_DEFAULT[]      		= {0x04, 0x80, 0x80, 0x80, 0xFC};
static const uint8_t AC_FAN_INCREASE[] 		= {0x04, 0x80, 0x80, 0x90, 0xEC};
static const uint8_t AC_FAN_DECREASE[] 		= {0x04, 0x80, 0x80, 0xF0, 0x8C};
static const uint8_t AC_TEMP_INCREASE[]    	= {0x04, 0x80, 0x80, 0x81, 0xFB};
static const uint8_t AC_TEMP_DECREASE[]    	= {0x04, 0x80, 0x80, 0x87, 0xF5};
static const uint8_t AC_AUTO[]         		= {0x04, 0x82, 0x80, 0x80, 0xFA};
static const uint8_t AC_MODE[]         		= {0x04, 0x90, 0x80, 0x80, 0xEC};
static const uint8_t AC_ACSTATE[]           = {0x04, 0x84, 0x80, 0x80, 0xF8};
static const uint8_t AC_FRONT_DEMIST[] 		= {0x04, 0xA0, 0x80, 0x80, 0xDC};
static const uint8_t AC_REAR_DEMIST[]  		= {0x04, 0xC0, 0x80, 0x80, 0xBC};
static const uint8_t AC_AIR_SOURCE[]  		= {0x04, 0x88, 0x80, 0x80, 0xF4};
static const uint8_t AC_AMBIENT_TOGGLE[]   	= {0x04, 0x80, 0xA0, 0x80, 0xDC};

void Climate_SendOFF(void)     			{ Climate_Send((uint8_t*)AC_OFF); }
void Climate_SendDefault(void)     		{ Climate_Send((uint8_t*)AC_DEFAULT); }
void Climate_SendFanIncrease(void) 		{ Climate_Send((uint8_t*)AC_FAN_INCREASE); }
void Climate_SendFanDecrease(void) 		{ Climate_Send((uint8_t*)AC_FAN_DECREASE); }

void Climate_SendTempIncrease(void) {
	if (s.tempTens == 3 && s.tempUnits == 2){
		return;
	}
	Climate_Send((uint8_t*)AC_TEMP_INCREASE);
}
void Climate_SendTempDecrease(void) 	{ Climate_Send((uint8_t*)AC_TEMP_DECREASE); }
void Climate_SendAuto(void)     		{ Climate_Send((uint8_t*)AC_AUTO); }
void Climate_SendMode(void) 			{ Climate_Send((uint8_t*)AC_MODE); }
void Climate_SendAC(void) 				{ Climate_Send((uint8_t*)AC_ACSTATE); }
void Climate_SendFrontDemist(void) 		{ Climate_Send((uint8_t*)AC_FRONT_DEMIST); }
void Climate_SendRearDemist(void) 		{ Climate_Send((uint8_t*)AC_REAR_DEMIST); }
void Climate_SendAirSource(void) 		{ Climate_Send((uint8_t*)AC_AIR_SOURCE); }
void Climate_SendAmbientToggle(void) 	{ Climate_Send((uint8_t*)AC_AMBIENT_TOGGLE); }

// -------------------------------------------------------
// BCD helpers — match Arduino bcdtoint / bcdtodec
// -------------------------------------------------------
static uint8_t bcdToInt(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0)
{
    return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

static uint8_t bcdToDecimal(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0)
{
    // Only 0101 = 5, everything else = 0
    if (!b3 && b2 && !b1 && b0)
        return 5;
    return 0;
}

// -------------------------------------------------------
// Parse a complete 6-byte packet into ACState_t
// -------------------------------------------------------
static void parsePacket(uint8_t *pkt)
{
	//LCD_ResetIcons();

    switch (pkt[0])
    {
        case 0x0E: s.sysMode = SYS_AMBIENT;     break;
        case 0x0D: s.sysMode = SYS_TEMPERATURE; break;
        default:   s.sysMode = SYS_OFF;         break;
    }

    // Temperature BCD digits

    if (s.sysMode == 2){
    	s.tempTens = 1;
    	s.tempUnits = 8;
    	s.tempDecimal = 0;
    	s.fanSpeed = 0;
    }
    else{
		s.tempTens    = bcdToInt((pkt[1] >> 3) & 1, (pkt[1] >> 2) & 1,
								 (pkt[1] >> 1) & 1, (pkt[1] >> 0) & 1);
		s.tempUnits   = bcdToInt((pkt[2] >> 3) & 1, (pkt[2] >> 2) & 1,
								 (pkt[2] >> 1) & 1, (pkt[2] >> 0) & 1);
		s.tempDecimal = bcdToDecimal((pkt[3] >> 3) & 1, (pkt[3] >> 2) & 1,
									 (pkt[3] >> 1) & 1, (pkt[3] >> 0) & 1);

		// Fan speed — bits 6:4 of byte 2, direct value per your table
		s.fanSpeed = (pkt[2] >> 4) & 0x07;
    }
    //LCD_SetFanSpeed(s.fanSpeed);

    // System mode — byte 0
    //LCD_SetMode(s.sysMode);

    // Vent direction — bits 1:0 of byte 4
    uint8_t ventDir = pkt[4] & 0x03;
    switch (ventDir)
    {
        case 0:  s.ventMode = VENT_FACE;        break;  // bit1=0, bit0=0
        case 1:  s.ventMode = VENT_FEET_DEMIST; break;  // bit1=0, bit0=1
        case 2:  s.ventMode = VENT_FEET_FACE;   break;  // bit1=1, bit0=0
        case 3:  s.ventMode = VENT_FEET;        break;  // bit1=1, bit0=1
    }
    //LCD_SetMode(s.ventMode);


    // Circulation — bit 3 of byte 4
    s.circMode = ((pkt[4] >> 3) & 1) ? CIRC_RECIRCULATE : CIRC_FRESH_AIR;

    // Demist dependency — byte 4 bit 2
    s.demistDependency = (pkt[4] >> 2) & 1;

    // Status flags — active HIGH per your reverse engineering
    s.autoEnabled = (pkt[3] >> 6) & 1;
    s.acEnabled   = (pkt[3] >> 5) & 1;
    s.ecoEnabled  = (pkt[3] >> 4) & 1;
    s.demistFront = (pkt[4] >> 6) & 1;
    s.demistRear  = (pkt[4] >> 5) & 1;

    s.valid = 1;
    acState = s;
    //LCD_FlushIcons();
    Climate_SetLCD(&s);
    Climate_SendStatus(&huart5);
    Climate_SendStatus(&huart4);
}

// -------------------------------------------------------
// Init — configure UART7 for single byte receive
// UART7 must be 4800 baud, 8E1, inverted RX in CubeMX
// -------------------------------------------------------
void Climate_Init(UART_HandleTypeDef *huart)
{
    _huart = huart;
    memset(ClimateBuf, 0, sizeof(ClimateBuf));
    ClimatePacketReady = 0;
    HAL_StatusTypeDef result = HAL_UARTEx_ReceiveToIdle_DMA(_huart, ClimateBuf, AC_PACKET_LEN);
    if (result != HAL_OK)
        Report_USB("Climate DMA arm FAILED");
    else
        Report_USB("Climate DMA armed OK");
}

uint8_t Climate_PacketReady(void)   { return ClimatePacketReady; }
ACState_t Climate_GetState(void)    { return acState; }
void Climate_ClearReady(void)       { ClimatePacketReady = 0; }

// -------------------------------------------------------
// Debug — print parsed state to USB CDC
// -------------------------------------------------------
void Climate_PrintState(ACState_t *s)
{
    char buf[64];

    snprintf(buf, sizeof(buf), "Temp: %d%d.%d",
             s->tempTens, s->tempUnits, s->tempDecimal);
    Report_USB(buf);

    snprintf(buf, sizeof(buf), "Fan: %d", s->fanSpeed);
    Report_USB(buf);

    switch (s->sysMode)
    {
        case SYS_AMBIENT:     Report_USB("System: Ambient");     break;
        case SYS_TEMPERATURE: Report_USB("System: Temperature"); break;
        default:              Report_USB("System: OFF");         break;
    }

    switch (s->ventMode)
    {
        case VENT_FACE:        Report_USB("Vent: Face");         break;
        case VENT_FEET_DEMIST: Report_USB("Vent: Feet+Demist");  break;
        case VENT_FEET_FACE:   Report_USB("Vent: Feet+Face");    break;
        case VENT_FEET:        Report_USB("Vent: Feet");         break;
    }

    Report_USB(s->circMode == CIRC_RECIRCULATE ?
               "Circ: Recirculate" : "Circ: Fresh Air");

    if (s->autoEnabled)  Report_USB("AUTO: ON");
    if (s->acEnabled)    Report_USB("AC: ON");
    if (s->ecoEnabled)   Report_USB("ECO: ON");
    if (s->demistFront)  Report_USB("Front Demist: ON");
    if (s->demistRear)   Report_USB("Rear Demist: ON");
}


void Climate_CheckFlag(void){
		//LCD_SetTime();
	    if (ClimatePacketReady)
	    {
	        ClimatePacketReady = 0;
	        climateReceived = 1;

	        //char dbg[48];
	        //snprintf(dbg, sizeof(dbg), "PKT: %02X %02X %02X %02X %02X %02X",
	        //         ClimateBuf[0], ClimateBuf[1], ClimateBuf[2],
	        //         ClimateBuf[3], ClimateBuf[4], ClimateBuf[5]);
	        //Report_USB(dbg);

	        if (memcmp(ClimateBuf, lastPKT, AC_PACKET_LEN) != 0) {
	            memcpy(lastPKT, ClimateBuf, AC_PACKET_LEN);
	            parsePacket(ClimateBuf);
	            Climate_PrintStateSingle();
	            //Only parse the packet if the incoming packet is different to the last
	        }
	        //parsePacket(ClimateBuf);
	        HAL_UARTEx_ReceiveToIdle_DMA(_huart, ClimateBuf, AC_PACKET_LEN);
	   }
}

void Climate_Send(uint8_t *packet)
{
    HAL_UART_Transmit(_huart, packet, AC_TX_PACKET_LEN, 100);
}


void Climate_PrintStateSingle()
{
    char buf[128];
    snprintf(buf, sizeof(buf),
        "Sys:%dT:%d%d.%d F:%d M:%d V:%d C:%d A:%d AC:%d E:%d DF:%d DR:%d",
		s.sysMode,
        s.tempTens, s.tempUnits, s.tempDecimal,
        s.fanSpeed,
        s.sysMode,
        s.ventMode,
        s.circMode,
        s.autoEnabled,
        s.acEnabled,
        s.ecoEnabled,
        s.demistFront,
        s.demistRear
    );
    Report_USB(buf);
}

void Climate_SetLCD(ACState_t *s){
	LCD_ResetIcons();
    LCD_SetTime();
    LCD_SetDivider();
	if (s->sysMode == 2){
		//LCD_SetTemperature(10, 10, 10);
		LCD_FlushIcons();
		return;
	}
	LCD_SetTemperature(s->tempTens, s->tempUnits, s->tempDecimal);
	LCD_SetFanSpeed(s->fanSpeed);
	LCD_SetVentMode(s->ventMode);
	LCD_SetCircMode(s->circMode);
	LCD_SetAuto(s->autoEnabled);
	LCD_SetAC(s->acEnabled);
	LCD_SetECO(s->ecoEnabled);
	LCD_SetDemistFront(s->demistFront);
	LCD_FlushIcons();
}


void Climate_SendStatus(UART_HandleTypeDef *huart)
{
	if (!climateReceived){
		return;
	}

    uint8_t data[12];
    data[0]  = s.tempTens;
    data[1]  = s.tempUnits;
    data[2]  = s.tempDecimal;
    data[3]  = s.fanSpeed;
    data[4]  = s.sysMode;
    data[5]  = s.ventMode;
    data[6]  = s.circMode;
    data[7]  = s.autoEnabled;
    data[8]  = s.acEnabled;
    data[9]  = s.ecoEnabled;
    data[10] = s.demistFront;
    data[11] = s.demistRear;

    Protocol_Send(huart, STM, CMD_CLIMATE_STATUS, data, sizeof(data));
}
