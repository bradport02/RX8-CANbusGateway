/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define QSPI_D2_Pin GPIO_PIN_2
#define QSPI_D2_GPIO_Port GPIOE
#define LED_1Hz_Pin GPIO_PIN_3
#define LED_1Hz_GPIO_Port GPIOE
#define PWM_OUT_Pin GPIO_PIN_5
#define PWM_OUT_GPIO_Port GPIOE
#define OUT1Hz_Pin GPIO_PIN_13
#define OUT1Hz_GPIO_Port GPIOC
#define U1LCD_CS_Pin GPIO_PIN_0
#define U1LCD_CS_GPIO_Port GPIOC
#define U1LCD_MOSI_Pin GPIO_PIN_1
#define U1LCD_MOSI_GPIO_Port GPIOC
#define U1LCD_MISO_Pin GPIO_PIN_2
#define U1LCD_MISO_GPIO_Port GPIOC
#define WheelButtonIN_Pin GPIO_PIN_3
#define WheelButtonIN_GPIO_Port GPIOC
#define ESP32_RX_Pin GPIO_PIN_0
#define ESP32_RX_GPIO_Port GPIOA
#define ESP32_TX_Pin GPIO_PIN_1
#define ESP32_TX_GPIO_Port GPIOA
#define Beep_Out_Pin GPIO_PIN_4
#define Beep_Out_GPIO_Port GPIOA
#define LMOUT_Pin GPIO_PIN_6
#define LMOUT_GPIO_Port GPIOA
#define illumination_in_Pin GPIO_PIN_7
#define illumination_in_GPIO_Port GPIOA
#define CD_MUTE_Pin GPIO_PIN_4
#define CD_MUTE_GPIO_Port GPIOC
#define CDENABLE_Pin GPIO_PIN_5
#define CDENABLE_GPIO_Port GPIOC
#define CDEJECT_Pin GPIO_PIN_0
#define CDEJECT_GPIO_Port GPIOB
#define QSPI_CLK_Pin GPIO_PIN_2
#define QSPI_CLK_GPIO_Port GPIOB
#define APSTATE_Pin GPIO_PIN_10
#define APSTATE_GPIO_Port GPIOE
#define AMP_MUTE_Pin GPIO_PIN_11
#define AMP_MUTE_GPIO_Port GPIOE
#define AMPSTATE_Pin GPIO_PIN_12
#define AMPSTATE_GPIO_Port GPIOE
#define Tuner_RST_Pin GPIO_PIN_13
#define Tuner_RST_GPIO_Port GPIOE
#define U1LCD_SCK_Pin GPIO_PIN_10
#define U1LCD_SCK_GPIO_Port GPIOB
#define CAN2_RXD_Pin GPIO_PIN_12
#define CAN2_RXD_GPIO_Port GPIOB
#define CAN2_TXD_Pin GPIO_PIN_13
#define CAN2_TXD_GPIO_Port GPIOB
#define FAN_OUT_Pin GPIO_PIN_14
#define FAN_OUT_GPIO_Port GPIOB
#define Fan_DOWN_Pin GPIO_PIN_9
#define Fan_DOWN_GPIO_Port GPIOD
#define Temp_UP_Pin GPIO_PIN_10
#define Temp_UP_GPIO_Port GPIOD
#define Temp_DOWN_Pin GPIO_PIN_11
#define Temp_DOWN_GPIO_Port GPIOD
#define FAN_TACHO_Pin GPIO_PIN_12
#define FAN_TACHO_GPIO_Port GPIOD
#define QSPI_D3_Pin GPIO_PIN_13
#define QSPI_D3_GPIO_Port GPIOD
#define CDBUS__Pin GPIO_PIN_6
#define CDBUS__GPIO_Port GPIOC
#define QSPI_D0_Pin GPIO_PIN_9
#define QSPI_D0_GPIO_Port GPIOC
#define OUT25MHz_Pin GPIO_PIN_8
#define OUT25MHz_GPIO_Port GPIOA
#define QSPI_D1_Pin GPIO_PIN_10
#define QSPI_D1_GPIO_Port GPIOC
#define STM32_TX_Pin GPIO_PIN_12
#define STM32_TX_GPIO_Port GPIOC
#define CAN1_RXD_Pin GPIO_PIN_0
#define CAN1_RXD_GPIO_Port GPIOD
#define CAN1_TXD_Pin GPIO_PIN_1
#define CAN1_TXD_GPIO_Port GPIOD
#define STM32_RX_Pin GPIO_PIN_2
#define STM32_RX_GPIO_Port GPIOD
#define LCD_EN_Pin GPIO_PIN_4
#define LCD_EN_GPIO_Port GPIOD
#define LCD_ACMODE_Pin GPIO_PIN_5
#define LCD_ACMODE_GPIO_Port GPIOD
#define LCD_RSMODE_Pin GPIO_PIN_6
#define LCD_RSMODE_GPIO_Port GPIOD
#define Backlight_PWM_Pin GPIO_PIN_3
#define Backlight_PWM_GPIO_Port GPIOB
#define QSPI_CS_Pin GPIO_PIN_6
#define QSPI_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
