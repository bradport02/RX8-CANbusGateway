/*
 * lcd.h
 * Created on: 23 Mar 2026
 * Author: Bradley Port
 * Royal Holloway University of London
 * EE3000
 *
 * Driver for OEM RX-8 LCD display
 */

#ifndef COMMUNICATION_LCD_H_
#define COMMUNICATION_LCD_H_

#include "stm32h7xx_hal.h"
#include "main.h"
#include <string.h>

// -------------------------------------------------------
// Pin mappings — match existing GPIO defines in main.h
// LCD_ACMODE_Pin = MODE0 (AC)
// LCD_RSMODE_Pin = MODE1 (RS)
// LCD_EN_Pin     = CS/SS
// All on GPIOD
// -------------------------------------------------------
#define LCD_AC_PORT     GPIOD
#define LCD_AC_PIN      LCD_ACMODE_Pin

#define LCD_RS_PORT     GPIOD
#define LCD_RS_PIN      LCD_RSMODE_Pin

#define LCD_CS_PORT     GPIOC
#define LCD_CS_PIN      U1LCD_CS_Pin

// -------------------------------------------------------
// PWM Backlight — TIM2 CH2 on PB3
// MCP1402T steps up to 8V
// -------------------------------------------------------
#define LCD_PWM_TIMER       htim2
#define LCD_PWM_CHANNEL     TIM_CHANNEL_2
#define LCD_PWM_DUTY_75     ((uint32_t)(4294967295UL * 0.75f))

// -------------------------------------------------------
// Icon register addresses (0x90–0x9D)
// -------------------------------------------------------
#define LCD_ICON_BASE   0x90
#define LCD_ICON_COUNT  14

// -------------------------------------------------------
// Function prototypes
// -------------------------------------------------------
void LCD_Init(SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *htim);

static inline void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline void DWT_Delay_us(uint32_t us)
{
    uint32_t start  = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < cycles);
}

// Core SPI
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_SetICON(uint8_t address, uint8_t value);

// Display control
void LCD_Startup(void);
void LCD_Clear(void);
void LCD_Home(void);
void LCD_Reset(void);
void LCD_ResetIcons(void);
void LCD_Print(const char *text);

// Backlight
void LCD_SetBacklight(uint8_t percent);

// Content
void LCD_SetFanSpeed(uint8_t speed);
void LCD_SetVentMode(uint8_t mode);
void LCD_SetDivider();
void LCD_SetMinDigit(uint8_t digit, uint8_t value);
void LCD_SetSevenSegment(uint8_t col, uint8_t value);
void LCD_SetCircMode(uint8_t circ);
void LCD_SetAuto(uint8_t Auto);
void LCD_SetAC(uint8_t AC);
void LCD_SetECO(uint8_t eco);
void LCD_SetDemistFront(uint8_t demist);

void LCD_FlushIcons(void);
void LCD_FlushSevenSegment(void);
void LCD_SetMinute(uint8_t col, uint8_t value);
void LCD_SetHour(uint8_t col, uint8_t value);
void LCD_SetTempDigit(uint8_t col, uint8_t value);
void LCD_SetTime();
void LCD_SetTemperature(uint8_t tens, uint8_t units, uint8_t decimal);
void LCD_Test(void);

#endif /* COMMUNICATION_LCD_H_ */
