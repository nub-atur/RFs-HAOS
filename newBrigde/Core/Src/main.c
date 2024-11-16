/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include "LoRa.h"
#include <stdlib.h>
#include "cJSON.h"
#include "string.h"
#include <ctype.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart5;

/* USER CODE BEGIN PV */
LoRa myLoRa;
uint8_t read_data_lora[128];
uint8_t read_data_bluetooth[128];
int		RSSI_lora;

uint8_t waitBuf[] = "Waiting for node ..... \r\n";
uint8_t resetBuf[] = "Reset hardware now .... \r\n";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_UART5_Init(void);
static void MX_SPI2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_UART5_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  // MODULE SETTINGS ----------------------------------------------
  	myLoRa = newLoRa();

  	myLoRa.hSPIx                 = &hspi2;
  	myLoRa.CS_port               = LoRa_NSS_GPIO_Port;
  	myLoRa.CS_pin                = LoRa_NSS_Pin;
  	myLoRa.reset_port            = LoRa_RS_GPIO_Port;
  	myLoRa.reset_pin             = LoRa_RS_Pin;
  	myLoRa.DIO0_port			 = LoRa_DIO0_GPIO_Port;
  	myLoRa.DIO0_pin				 = LoRa_DIO0_Pin;

  	myLoRa.frequency             = 433;		     			// default = 433 MHz
  	myLoRa.spredingFactor        = SF_7;					// default = SF_7
  	myLoRa.bandWidth			 = BW_125KHz;			    // default = BW_125KHz
  	myLoRa.crcRate				 = CR_4_5;					// default = CR_4_5
  	myLoRa.power			     = POWER_17db;				// default = 20db
  	myLoRa.overCurrentProtection = 120; 					// default = 100 mA
  	myLoRa.preamble				 = 10;		  				// default = 8;

  	LoRa_setSyncWord(&myLoRa, 0x23);
  	LoRa_reset(&myLoRa);
  	while(LoRa_init(&myLoRa) != LORA_OK){
  		HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
  		CDC_Transmit_FS(waitBuf, sizeof(waitBuf));
  	}

  	// START CONTINUOUS RECEIVING -----------------------------------
  	LoRa_startReceiving(&myLoRa);

	//-----------------Initialize JSON------------------------------------
	cJSON_Hooks hooks = {mymalloc,free};
	cJSON_InitHooks(&hooks);
	//--------------------------------------------------------------------
	HAL_Delay(10000);

	uint32_t current = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  packet data;

	  CLEAR(read_data_bluetooth);
	  CLEAR(read_data_lora);

	  if(check == 0){
		  // LORA RECEIVING DATA - - - - - - - - - - - - - - - - - - - - - - - -
		  	  if(LoRa_receive(&myLoRa, read_data_lora, 128) > 0){
		  		  RSSI_lora = LoRa_getRSSI(&myLoRa);
		  		  parseJS((char*)read_data_lora, &data);
		  	  }

		  	  if(HAL_UART_Receive(&huart5, read_data_bluetooth, 20, 1000) == HAL_OK){
		  		  if (strlen((char*)read_data_bluetooth) > 0){
		  			  char *token = strtok((char*)read_data_bluetooth, ",");
		  			  if (token != NULL) data.nodeBluetooth.timeBLT = atoi(token);
		  			  token = strtok(NULL, ",");
		  			  if (token != NULL) data.nodeBluetooth.energyBLT = strtod(token, NULL);
		  			  token = strtok(NULL, ",");
		  			  if (token != NULL) data.nodeBluetooth.disData = atoi(token);
		  			  token = strtok(NULL, ",");
		  			  if (token != NULL) data.nodeBluetooth.RSSI_bluet = atoi(token);
		  		  }
		  		  if (!isdigit(data.nodeBluetooth.timeBLT)) CLEAR(data.nodeBluetooth);
		  	  }

		  	if (HAL_GetTick() - current >= 1000){
		  	  cJSON *root = cJSON_CreateObject(); /*create JSON string root*/
		  	  	  cJSON *rootNode = cJSON_CreateArray();
		  	  	  cJSON_AddItemToObject(root, "root", rootNode);
		  	  	  cJSON_AddItemToArray(rootNode, cJSON_CreateNumber(data.energyRoot));
		  	  	  cJSON_AddItemToArray(rootNode, cJSON_CreateNumber(data.timeRoot));
		  	  	  cJSON_AddItemToArray(rootNode, cJSON_CreateNumber(RSSI_lora));

		  	  	  cJSON *defaultNode = cJSON_CreateArray();
		  	  	  cJSON_AddItemToObject(root, "node2", defaultNode);
		  	  	  cJSON_AddItemToArray(defaultNode, cJSON_CreateNumber(data.node2.motor2));
		  	  	  cJSON_AddItemToArray(defaultNode, cJSON_CreateNumber(data.node2.obs2));
		  	  	  cJSON_AddItemToArray(defaultNode, cJSON_CreateNumber(data.node2.time2));
		  	  	  cJSON_AddItemToArray(defaultNode, cJSON_CreateNumber(data.node2.ack2));

		  	  	  cJSON *sendOnlyNode = cJSON_CreateArray();
		  	  	  cJSON_AddItemToObject(root, "node1", sendOnlyNode);
		  	  	  cJSON_AddItemToArray(sendOnlyNode, cJSON_CreateNumber(data.node1.temp1));
		  	  	  cJSON_AddItemToArray(sendOnlyNode, cJSON_CreateNumber(data.node1.hum1));
		  	  	  cJSON_AddItemToArray(sendOnlyNode, cJSON_CreateNumber(data.node1.time1));
		  	  	  cJSON_AddItemToArray(sendOnlyNode, cJSON_CreateNumber(data.node1.ack1));

		  	  	  cJSON *bluetoothNode = cJSON_CreateArray();
		  	  	  cJSON_AddItemToObject(root, "bltN", bluetoothNode);
		  	  	  cJSON_AddItemToArray(bluetoothNode,cJSON_CreateNumber(data.nodeBluetooth.energyBLT));
		  	      cJSON_AddItemToArray(bluetoothNode,cJSON_CreateNumber(data.nodeBluetooth.timeBLT));
		  	      cJSON_AddItemToArray(bluetoothNode,cJSON_CreateNumber(data.nodeBluetooth.disData));
		  	      cJSON_AddItemToArray(bluetoothNode,cJSON_CreateNumber(data.nodeBluetooth.RSSI_bluet));

			  char *ret = cJSON_PrintUnformatted(root);
			  strcat(ret, "\r\n");
			  uint8_t result = CDC_Transmit_FS((uint8_t*)ret, strlen(ret));

			  if(result == USBD_OK) HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
			  cJSON_Delete(root);
			  free(ret);

			  current = HAL_GetTick();
		  }
	  } else if (check == 1) {
		    LoRa_transmit(&myLoRa, (uint8_t*)ReceivedData, dataSize, 100);
//		  	HAL_UART_Transmit(&huart5, (uint8_t*)ReceivedData, dataSize, 100);
		  	HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);

		  	for(int i = 0; i < dataSize; i++)
			{
				ReceivedData[i] = 0;
			}
			check = 0;
	  }

	  HAL_Delay(200);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 9600;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LoRa_RS_Pin|LoRa_NSS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : KEY1_Pin KEY2_Pin KEY3_Pin */
  GPIO_InitStruct.Pin = KEY1_Pin|KEY2_Pin|KEY3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LoRa_DIO0_Pin */
  GPIO_InitStruct.Pin = LoRa_DIO0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LoRa_DIO0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RED_LED_Pin */
  GPIO_InitStruct.Pin = RED_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RED_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LoRa_RS_Pin LoRa_NSS_Pin BLUE_LED_Pin */
  GPIO_InitStruct.Pin = LoRa_RS_Pin|LoRa_NSS_Pin|BLUE_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void parseJS(char *jsonString, packet *data)
{
	cJSON *root = cJSON_Parse(jsonString);

	cJSON *name = cJSON_GetObjectItem(root,"name");

	if(!name) return;

	//get data RootNode
	cJSON *rootData = cJSON_GetObjectItem(root, "root");
	if(rootData->type == cJSON_Array){
		uint8_t i,size = cJSON_GetArraySize(rootData);
		for(i=0;i<size;i++){
			cJSON *dat = cJSON_GetArrayItem(rootData,i);
			switch(i){
			default: break;
			case 0:
				data->timeRoot = dat->valueint;
				break;
			case 1:
				data->energyRoot = dat->valuedouble;
				break;
			}
		}
	}

	//get data node1
	cJSON *node1 = cJSON_GetObjectItem(root, "node1");
	if(node1->type == cJSON_Array){
		uint8_t i,size = cJSON_GetArraySize(node1);
		for(i=0;i<size;i++){
			cJSON *dat = cJSON_GetArrayItem(node1,i);
			switch(i){
			default: break;
			case 0:
				data->node1.hum1 = dat->valuedouble;
				break;
			case 1:
				data->node1.temp1 = dat->valuedouble;
				break;
			case 2:
				data->node1.time1 = dat->valueint;
				break;
			case 3:
				data->node1.ack1 = dat->valueint;
				break;
			}
		}
	}

	//get data node2
	cJSON *node2 = cJSON_GetObjectItem(root, "node2");
	if(node2->type == cJSON_Array){
		uint8_t i,size = cJSON_GetArraySize(node2);
		for(i=0;i<size;i++){
			cJSON *dat = cJSON_GetArrayItem(node2,i);
			switch (i){
				default: break;
				case 0:
					data->node2.motor2 = dat->valueint;
					break;
				case 3:
					data->node2.ack2 = dat->valueint;
					break;
				case 2:
					data->node2.time2 = dat->valueint;
					break;
				case 1:
					data->node2.obs2 = dat->valueint;
			}
		}
	}

	if(!root) cJSON_Delete(root);
}

void *mymalloc(unsigned int size)
{
	return malloc(size);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
