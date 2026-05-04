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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"  // extra libraries
#include "stdlib.h"
#include "string.h"
#include "math.h"
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
SPI_HandleTypeDef hspi4;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

/* USER CODE BEGIN PV */

uint8_t machine_start = 0;  // variable declaration
uint8_t machine_stop = 0;

uint8_t data_array [3];
uint8_t data_array2 [3];

int rising_edge_count = 0;

float Duty_Cycle;
float first_ccr_count = 0;
float second_ccr_count = 0;
float period = 0.0;
float frequency_hertz;
float frequency_kilohertz;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI4_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM5_Init(void);
/* USER CODE BEGIN PFP */
void stop_buzzer();
void start_buzzer(uint32_t frequency);
void emergency_stop(void); // Emergency stop declaration
void machine_reset(void);  // rest delcaration
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
  MX_SPI4_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);  // timer inintalization
  HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_2);
  HAL_SPI_Init(&hspi4);


  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_0, GPIO_PIN_RESET); // intializes start and stop
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_1, GPIO_PIN_RESET);

  // SETTING INITIAL CONDITIONS OF START AND STOP LEDS
  HAL_GPIO_WritePin(GPIOF, Start_LED_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOF, Stop_LED_Pin, GPIO_PIN_RESET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    machine_stop = 0;  // resets stop fucntion

  // Start -  buzzer beeps at  5.0 KHz, GREEN LED blinks 6 times

    while (machine_start == 1){  // starts the conveyer bely
    start_buzzer(5500);

      for (int i = 0; i < 6; i++)  // for loop controlls green LED and the buzzer
      {
        while (machine_stop == 1)  // checks for emergency stop signal to go high
        {
          if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1) == GPIO_PIN_RESET)
          {
            break;
          }

          HAL_GPIO_WritePin(GPIOF, Start_LED_Pin, GPIO_PIN_RESET); // green LED to low

          stop_buzzer();

          emergency_stop();  // emergency stop call


          i = i;  // stops at current state and saves it
        }


        TIM2->PSC = 0;                   // Presclaer
        TIM2->ARR = 12002;                 // (60000000 / (5000 - 1) = 12002.4
        TIM2->CCR1 = 6001;                // 12002 * 0.5 = 6001 = 50% DUTY CYCLE

        HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); // buzzer enable
        HAL_GPIO_WritePin(GPIOF, Start_LED_Pin, GPIO_PIN_SET); // geen LED enable

        HAL_Delay(500);                // 1 sec delay

        HAL_GPIO_WritePin(GPIOF, Start_LED_Pin, GPIO_PIN_RESET);  // Green led low

        HAL_Delay(500);                // 1 sec delay
      }


      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1); // stops buzzer


      // Start -  Reads input freq. and outputs duty cycle
      // INITIAL 30KHz START SIGNAL
      TIM4->PSC = 0;                    // PRESCALER
      TIM4->ARR = 2000;                 // (60000000 / (30000 - 1) = 2000.0
      TIM4->CCR3 = 1000;                // 2000 * 0.5 = 1000 = 50% DUTY CYCLE

      // STARTING PWM FOR BELT
      HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
      HAL_GPIO_WritePin(GPIOD, Sensor1_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOD, Sensor2_Pin, GPIO_PIN_SET);

      while (HAL_GPIO_ReadPin(GPIOD, Sensor1_Pin) == GPIO_PIN_SET)
      {

        while (machine_stop == 1) // checks for emergency stop
        {
          if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1) == GPIO_PIN_RESET)
          {
            break;
          }


          emergency_stop(); // emergency stop call


          HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3); // stops PWM
        }

        // STARTING PWM FOR BELT
        HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);


        // OUTPUTTING FREQUENCY TO TERMINAL I/O in HERTZ
        printf("FREQUENCY = %.1f Hz \n", frequency_hertz);


        HAL_Delay(5000);

        if (frequency_hertz >= 4700 && frequency_hertz <= 4890)
        {
          // CHANGING DUTY CYCLE
          TIM4->CCR3 = 1800;                // 2000 * 0.9 = 1800 = 90% DUTY CYCLE
          Duty_Cycle = 1800;
          printf("Duty Cycle = %.1f Hz \n", Duty_Cycle);
          HAL_Delay(5000);
        }
        else if(frequency_hertz >= 4900 && frequency_hertz <= 5090)
        {
          // CHANGING DUTY CYCLE
          TIM4->CCR3 = 1600;                // 2000 * 0.8 = 1600 = 80% DUTY CYCLE
          Duty_Cycle = 1600;
          printf("Duty Cycle = %.1f Hz \n", Duty_Cycle);
          HAL_Delay(5000);
        }
        else if(frequency_hertz >= 5100 && frequency_hertz <= 5290)
        {
          // CHANGING DUTY CYCLE
          TIM4->CCR3 = 1400;                // 2000 * 0.7 = 1400 = 70% DUTY CYCLE
          Duty_Cycle = 1400;
          printf("Duty Cycle = %.1f Hz \n", Duty_Cycle);
          HAL_Delay(5000);
        }
        else if(frequency_hertz >= 5300 && frequency_hertz <= 5490)
        {
          // CHANGING DUTY CYCLE
          TIM4->CCR3 = 1200;                // 2000 * 0.6 = 1200 = 60% DUTY CYCLE
          Duty_Cycle = 1200;
          printf("Duty Cycle = %.1f Hz \n", Duty_Cycle);
          HAL_Delay(5000);
        }
        else if(frequency_hertz >= 5500 && frequency_hertz <= 5690)
        {
          // CHANGING DUTY CYCLE
          TIM4->CCR3 = 1000;                // 2000 * 0.5 = 1800 = 50% DUTY CYCLE
          Duty_Cycle = 1000;
          printf("Duty Cycle = %.1f Hz \n", Duty_Cycle);
          HAL_Delay(5000);
        }
        else if(frequency_hertz >= 5700 && frequency_hertz <= 5890)
        {
          // CHANGING DUTY CYCLE
          TIM4->CCR3 = 800;                // 2000 * 0.4 = 800 = 40% DUTY CYCLE
          Duty_Cycle = 800;
          printf("Duty Cycle = %.1f Hz \n", Duty_Cycle);
          HAL_Delay(5000);
        }
        else if(frequency_hertz >= 5900 && frequency_hertz < 6090)
        {
          // CHANGING DUTY CYCLE
          TIM4->CCR3 = 600;                // 2000 * 0.3 = 600 = 30% DUTY CYCLE
          Duty_Cycle = 600;
          printf("Duty Cycle = %.1f Hz \n", Duty_Cycle);
          HAL_Delay(5000);
        }
        else if(frequency_hertz >= 6100 && frequency_hertz <= 6290)
        {
          // CHANGING DUTY CYCLE
          TIM4->CCR3 = 400;                // 2000 * 0.2 = 400 = 20% DUTY CYCLE
          Duty_Cycle = 400;
          printf("Duty Cycle = %.1f Hz \n", Duty_Cycle);
          HAL_Delay(5000);
        }
          else if(frequency_hertz >= 6300 && frequency_hertz <= 6490)
        {
          // CHANGING DUTY CYCLE
          TIM4->CCR3 = 192;                // 2000 * 0.1 = 200 = 10% DUTY CYCLE
          Duty_Cycle = 192;
          printf("Duty Cycle = %.1f Hz \n", Duty_Cycle);
          HAL_Delay(5000);
        }
        else
        {
          TIM4->CCR3 = TIM4->CCR3;
        }
      }


      // checks IR sensors for location of object
//
//
//      HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3); // stops the PWM
//
//      if (HAL_GPIO_ReadPin(GPIOD, Sensor1_Pin) == GPIO_PIN_RESET || HAL_GPIO_ReadPin(GPIOD, Sensor2_Pin) == GPIO_PIN_RESET)
//      {
//
//        for (int i = 0; i < 10; i++)  // controls stopping buzzer
//        {
//          while (machine_stop == 1) // checks for emergency stop
//          {
//            if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1) == GPIO_PIN_RESET)
//            {
//              break;
//            }
//
//            emergency_stop();  // emergency stop
//
//            i = i; // stops at current state and saves it
//
//            HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1); // stops buzzer
//          }
//
//          // SETTING UP PWM FOR BUZZER
//          TIM2->PSC = 0;                    // PRESCALER
//          TIM2->ARR = 17148;                // (60000000 / (3500 - 1) = 17147.7
//          TIM2->CCR1 = 8574;                // 17148 * 0.5 = 8574 = 50% DUTY CYCLE
//
//          // STARTING BUZZER
//          HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
//
//          // TURN RED LED ON
//          HAL_GPIO_WritePin(GPIOF, Stop_LED_Pin, GPIO_PIN_SET);
//
//          // DELAY OF 500 MILLISECONDS
//          HAL_Delay(500);
//
//          // TURN RED LED OFF
//          HAL_GPIO_WritePin(GPIOF, Stop_LED_Pin, GPIO_PIN_RESET);
//
//          // DELAY OF 500 MILLISECONDS
//          HAL_Delay(500);
//        }
//      }

      // STOPPING BUZZER
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);

      // RESETTING MACHINE BY CALLING FUNCTION
      machine_reset();
    }

    // CHECK IF MACHINE E-STOP BUTTON WAS PRESSED
    while (machine_stop == 1)
    {
      // BREAKING LOOP CONDITION
      if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1) == GPIO_PIN_RESET)
      {
        break;
      }
    }

    // DELAY OF 500 MILLISECONDS
    HAL_Delay(500);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 120;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI4_Init(void)
{

  /* USER CODE BEGIN SPI4_Init 0 */

  /* USER CODE END SPI4_Init 0 */

  /* USER CODE BEGIN SPI4_Init 1 */

  /* USER CODE END SPI4_Init 1 */
  /* SPI4 parameter configuration*/
  hspi4.Instance = SPI4;
  hspi4.Init.Mode = SPI_MODE_MASTER;
  hspi4.Init.Direction = SPI_DIRECTION_2LINES;
  hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi4.Init.NSS = SPI_NSS_SOFT;
  hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi4.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI4_Init 2 */

  /* USER CODE END SPI4_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 0;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 65535;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, Start_LED_Pin|Stop_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, Reset_Pin|CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PF0 PF1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : Start_LED_Pin Stop_LED_Pin */
  GPIO_InitStruct.Pin = Start_LED_Pin|Stop_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : Reset_Pin CS_Pin */
  GPIO_InitStruct.Pin = Reset_Pin|CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : Sensor1_Pin Sensor2_Pin */
  GPIO_InitStruct.Pin = Sensor1_Pin|Sensor2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

}

/* USER CODE BEGIN 4 */
// ************* FUNCTIONS THAT WERE DECLARED AT TOP OF PROGRAM ****************
void start_buzzer (uint32_t frequency){
  uint32_t buzz_period = 60000000 / frequency;
  TIM2->ARR = buzz_period;
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  TIM2->CCR4 = buzz_period / 2;
  }

void stop_buzzer () {
  HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
}


/**
    @brief THIS FUNCTION IS USED TO RESET MACHINE TO IDLE STATE
    @retval NONE
*/
void machine_reset(void)
{
  machine_start = 0;
  machine_stop = 0;


}

void emergency_stop(void)
{

  //DELAY FOR 5 MILLISECONDs
  HAL_Delay(5);

  //DELAY FOR 5 MILLISECONDs
  HAL_Delay(5);


  //DELAY FOR 5 MILLISECONDs
  HAL_Delay(5);

  //DELAY FOR 5 MILLISECONDs
  HAL_Delay(5);
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
