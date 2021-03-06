/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define EEPROM_ADDR 0b10100000
#define IOEXPD_ADDR 0b01000000 // 0100 000 + 0 7bits(+1bit)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t eepromExample_Write_Flag = 0;
uint8_t eepromExample_Read_Flag = 0;
uint8_t IOExpdrExample_Write_Flag = 0;
uint8_t IOExpdrExample_Read_Flag = 0;
uint8_t eepromDataRead_Back[4];
uint8_t IOExpdrDataRead_Back;
uint8_t IOExpdrData_Write = 0b00000000;

uint8_t BlueButtonArray[2] = {1,1};//[0,1] == {Now,Past}

typedef enum
{
	detect_Button,
	Read_IOExpdr,
	Write_eeprom,
	Read_eeprom,
	Write_IOExpdr,
}State_Machine;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void EEPROMWriteExample(uint8_t SW_data);
void EEPROMReadExample(uint8_t *Rdata, uint16_t len);

void IOExpenderInit();
void IOExpenderReadPinA(uint8_t *Rdata);
void IOExpenderWritePinB(uint8_t Wdata);
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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(100);
  IOExpenderInit();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

//	detect_Button,
//	Read_IOExpdr,
//	Write_eeprom,
//	Read_eeprom,
//	Write_IOExpdr,

	static State_Machine State = Read_eeprom;
	switch(State)
	{
		case detect_Button :
			BlueButtonArray[1] = BlueButtonArray[0];//Past
			BlueButtonArray[0] = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);//Now
			if(BlueButtonArray[1] == 0 && BlueButtonArray[0] == 1)
			{
				State = Read_IOExpdr;
			}
			break;
		case Read_IOExpdr :
			IOExpdrExample_Read_Flag = 1;
			IOExpenderReadPinA(&IOExpdrDataRead_Back);
			State = Write_eeprom;
			HAL_Delay(50);  // HAVE TO !!!!!
			break;
		case Write_eeprom :
			eepromExample_Write_Flag = 1;
			EEPROMWriteExample(IOExpdrDataRead_Back);
			State = Read_eeprom;
			HAL_Delay(50);
			break;
		case Read_eeprom :
			eepromExample_Read_Flag = 1;
			EEPROMReadExample(eepromDataRead_Back, 4);
			State = Write_IOExpdr;
			HAL_Delay(50);
			break;
		case Write_IOExpdr :
			IOExpdrExample_Write_Flag = 1;
			IOExpdrData_Write = ((eepromDataRead_Back[0]) | (eepromDataRead_Back[1]<<1) |
					 (eepromDataRead_Back[2]<<2) | (eepromDataRead_Back[3]<<3)) | 0b11110000;
			HAL_Delay(50);
			IOExpenderWritePinB(IOExpdrData_Write);
			State = detect_Button;
			HAL_Delay(50);
			break;
	}
//	EEPROMWriteExample();
//	EEPROMReadExample(eepromDataRead_Back, 4);
//
//	IOExpenderReadPinA(&IOExpdrDataRead_Back);
//	IOExpenderWritePinB(IOExpdrData_Write);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void EEPROMWriteExample(uint8_t SW_data) {
	if (eepromExample_Write_Flag && hi2c1.State == HAL_I2C_STATE_READY) {

		static uint8_t data[4] = {0};
		data[0] =(SW_data>>3) & 0b00000001;
		data[1] =(SW_data>>2) & 0b00000001;
		data[2] =(SW_data>>1) & 0b00000001;
		data[3] = SW_data & 0b00000001;

		HAL_I2C_Mem_Write_IT(&hi2c1, EEPROM_ADDR, 0x34, I2C_MEMADD_SIZE_16BIT,
				data, 4);



		eepromExample_Write_Flag = 0;
	}
}
void EEPROMReadExample(uint8_t *Rdata, uint16_t len) {
	if (eepromExample_Read_Flag && hi2c1.State == HAL_I2C_STATE_READY) {

		HAL_I2C_Mem_Read_IT(&hi2c1, EEPROM_ADDR, 0x34, I2C_MEMADD_SIZE_16BIT,
				Rdata, len);
		eepromExample_Read_Flag = 0;
	}
}
void IOExpenderInit() {
	//Init All
	static uint8_t Setting[0x16] = { 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //0x00 = set input , 0x01 = set output
			0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,    //0x0C = set pull-up
			0x00, 0x00, 0x00, 0x00 };
	HAL_I2C_Mem_Write(&hi2c1, IOEXPD_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, Setting,
			0x16, 100);
}
void IOExpenderReadPinA(uint8_t *Rdata) {
	if (IOExpdrExample_Read_Flag && hi2c1.State == HAL_I2C_STATE_READY) {
		HAL_I2C_Mem_Read_IT(&hi2c1, IOEXPD_ADDR, 0x12, I2C_MEMADD_SIZE_8BIT,
				Rdata, 1);
		IOExpdrExample_Read_Flag =0;
	}
}
void IOExpenderWritePinB(uint8_t Wdata) {
	if (IOExpdrExample_Write_Flag && hi2c1.State == HAL_I2C_STATE_READY) {
		static uint8_t data;
		data = Wdata;
		HAL_I2C_Mem_Write_IT(&hi2c1, IOEXPD_ADDR, 0x15, I2C_MEMADD_SIZE_8BIT,
				&data, 1);
		IOExpdrExample_Write_Flag=0;
	}
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
