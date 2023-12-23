/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stepgen.h"
#include "liquidcrystal_i2c.h"
#include "string.h"
#include "utils.h"
#include "process.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
uint32_t debug_arr[32];
uint16_t adc_value;
uint16_t test_val;


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
#define in_button_Pin GPIO_PIN_15
#define in_button_GPIO_Port GPIOB
#define in_buttonA10_Pin GPIO_PIN_10
#define in_buttonA10_GPIO_Port GPIOA
#define in_buttonA11_Pin GPIO_PIN_11
#define in_buttonA11_GPIO_Port GPIOA
#define in_buttonA12_Pin GPIO_PIN_12
#define in_buttonA12_GPIO_Port GPIOA
#define in_enc_B_Pin GPIO_PIN_3
#define in_enc_B_GPIO_Port GPIOB
#define in_enc_A_Pin GPIO_PIN_4
#define in_enc_A_GPIO_Port GPIOB
#define in_enc_bt_Pin GPIO_PIN_5
#define in_enc_bt_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
