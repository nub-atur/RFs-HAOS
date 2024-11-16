/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct packet {
    double energyRoot;
	int timeRoot;

    struct node1 {
		double temp1, hum1;
        uint8_t ack1;
        int time1;
    } node1;

    struct node2 {
        uint8_t motor2;
        uint8_t obs2;
        uint8_t ack2;
        int time2;
    } node2;

    struct nodeBluetooth{
    	int timeBLT;
    	double energyBLT;
    	uint16_t disData;
    	int RSSI_bluet;
    } nodeBluetooth;
} packet;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
void *mymalloc(unsigned int size);
void convertAndPrintHighestUnit(uint16_t totalSeconds, char* str);
void parseJS(char *jsonString, packet *data);
uint8_t miniErrCheck(uint8_t *p, uint8_t i);
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KEY1_Pin GPIO_PIN_0
#define KEY1_GPIO_Port GPIOA
#define KEY2_Pin GPIO_PIN_1
#define KEY2_GPIO_Port GPIOA
#define KEY3_Pin GPIO_PIN_2
#define KEY3_GPIO_Port GPIOA
#define LoRa_DIO0_Pin GPIO_PIN_4
#define LoRa_DIO0_GPIO_Port GPIOC
#define RED_LED_Pin GPIO_PIN_5
#define RED_LED_GPIO_Port GPIOC
#define LoRa_RS_Pin GPIO_PIN_0
#define LoRa_RS_GPIO_Port GPIOB
#define LoRa_NSS_Pin GPIO_PIN_1
#define LoRa_NSS_GPIO_Port GPIOB
#define BLUE_LED_Pin GPIO_PIN_2
#define BLUE_LED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
//#define iDEBUG
#define CLEAR(s) memset(&(s), '\0', sizeof(s))
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
