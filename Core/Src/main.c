/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "button.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    IDLE,
    WORK,
    SHORT_BREAK,
    LONG_BREAK,
    PAUSED
} PomodoroState;

typedef enum {
    EVENT_START,
    EVENT_PAUSE,
    EVENT_RESUME,
    EVENT_TIMEOUT,
    EVENT_RESET,
    EVENT_SKIP
} PomodoroEvent;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

Button start_button = {GPIO_PIN_RESET, GPIO_PIN_RESET, 0};
Button reset_button = {GPIO_PIN_RESET, GPIO_PIN_RESET, 0};
Button mode_button  = {GPIO_PIN_RESET, GPIO_PIN_RESET, 0};

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
volatile uint8_t time_running = 0;
volatile uint32_t ms_counter = 0;
volatile uint32_t seconds = 0;
volatile uint32_t minutes = 0;

uint8_t idle_position = 0;
uint16_t idle_animation_counter = 0;

volatile uint32_t goal = 25 * 60;
volatile uint32_t pause_goal = 5 * 60;

volatile uint8_t current_digit = 0;
uint8_t display_buffer[4] = {0, 0, 0, 0};

const uint8_t digit_segments[10] = {
	//GFEDCBA pattern
    0x3F, // 0: ABCDEF
    0x06, // 1: BC
    0x5B, // 2: ABDEG
    0x4F, // 3: ABCDG
    0x66, // 4: BCFG
    0x6D, // 5: ACDFG
    0x7D, // 6: ACDEFG
    0x07, // 7: ABC
    0x7F, // 8: ABCDEFG
    0x6F  // 9: ABCDFG
};

PomodoroState state = WORK;

typedef struct {
    uint8_t digit;
    uint8_t segment;
} AnimationFrame;

const AnimationFrame perimeter_animation[] = {
    {0, 0},  // DIG1 A
    {1, 0},  // DIG2 A
    {2, 0},  // DIG3 A
    {3, 0},  // DIG4 A
    {3, 1},  // DIG4 B
    {3, 2},  // DIG4 C
    {3, 3},  // DIG4 D
    {2, 3},  // DIG3 D
    {1, 3},  // DIG2 D
    {0, 3},  // DIG1 D
    {0, 4},  // DIG1 E
    {0, 5}   // DIG1 F
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void Display_SetSegments(uint8_t pattern);
void reset_display_buffer();
void Display_UpdateBuffer(uint32_t seconds);
void Display_Scan(void);

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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	switch (state)
	{
		case IDLE:
			// Waiting for user to press Start
			time_running = 0;
			break;

		case WORK:
			// Run work timer
			time_running = 1;
			if (seconds >= goal) {
				seconds = 0;
				state = SHORT_BREAK;
			}

			break;

		case SHORT_BREAK:
			// Run short break timer
			time_running = 1;
			if (seconds >= pause_goal) {
				seconds = 0;
				state = WORK;
			}

			break;

		case LONG_BREAK:
			// Run long break timer
			break;

		case PAUSED:
			// Keep timer stopped
			time_running = 0;
			break;

		default:
			break;
	}

	Display_UpdateBuffer(seconds);

	if (Button_Pressed(BTN1_GPIO_Port, BTN1_Pin, &mode_button)) {
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		state = PAUSED;
	}
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
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
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, DSP_F_Pin|DSP_E_Pin|DSP_D_Pin|DSP_C_Pin
                          |DSP_B_Pin|DSP_A_Pin|DSP_G_Pin|DSP_DP_Pin
                          |DIG1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DIG2_Pin|DIG3_Pin|DIG4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN1_Pin */
  GPIO_InitStruct.Pin = BTN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(BTN1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DSP_F_Pin DSP_E_Pin DSP_D_Pin DSP_C_Pin
                           DSP_B_Pin DSP_A_Pin DSP_G_Pin DSP_DP_Pin
                           DIG1_Pin */
  GPIO_InitStruct.Pin = DSP_F_Pin|DSP_E_Pin|DSP_D_Pin|DSP_C_Pin
                          |DSP_B_Pin|DSP_A_Pin|DSP_G_Pin|DSP_DP_Pin
                          |DIG1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : DIG2_Pin DIG3_Pin DIG4_Pin */
  GPIO_InitStruct.Pin = DIG2_Pin|DIG3_Pin|DIG4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void Display_UpdateBuffer(uint32_t seconds)
{
    uint32_t min = (seconds / 60) % 100;
    uint32_t sec = seconds % 60;

    display_buffer[0] = min / 10;
    display_buffer[1] = min % 10;
    display_buffer[2] = sec / 10;
    display_buffer[3] = sec % 10;
}

void Display_Scan(void)
{
    // 2. Disable all digits
	HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DIG2_GPIO_Port, DIG2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DIG3_GPIO_Port, DIG3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DIG4_GPIO_Port, DIG4_Pin, GPIO_PIN_RESET);

	// 3. Set segments for current digit
	if (state == IDLE)
	{
	    if (current_digit == perimeter_animation[idle_position].digit)
	    {
	        Display_SetSegments(1U << perimeter_animation[idle_position].segment);
	    }
	    else
	    {
	        Display_SetSegments(0);
	    }
	}
	else
	{
	    Display_SetSegments(digit_segments[display_buffer[current_digit]]);
	}

	// 4. Enable selected digit
	switch (current_digit)
	{
		case 0:
			HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG1_Pin, GPIO_PIN_SET);
			break;

		case 1:
			HAL_GPIO_WritePin(DIG2_GPIO_Port, DIG2_Pin, GPIO_PIN_SET);
			break;

		case 2:
			HAL_GPIO_WritePin(DIG3_GPIO_Port, DIG3_Pin, GPIO_PIN_SET);
			break;

		case 3:
			HAL_GPIO_WritePin(DIG4_GPIO_Port, DIG4_Pin, GPIO_PIN_SET);
			break;
	}
	current_digit = (current_digit + 1) % 4;
}

void Display_IdleAnimationUpdate(void)
{
    idle_animation_counter++;

    if (idle_animation_counter >= 150)
    {
        idle_animation_counter = 0;

        idle_position++;

        if (idle_position >= 12)
        {
            idle_position = 0;
        }
    }
}

void reset_display_buffer() {
	for (int i = 0; i < 4; i++) {
		display_buffer[i] = 0;
	}
}
void Display_SetSegments(uint8_t pattern)
{
    HAL_GPIO_WritePin(DSP_A_GPIO_Port, DSP_A_Pin,
        (pattern & (1 << 0)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(DSP_B_GPIO_Port, DSP_B_Pin,
        (pattern & (1 << 1)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(DSP_C_GPIO_Port, DSP_C_Pin,
        (pattern & (1 << 2)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(DSP_D_GPIO_Port, DSP_D_Pin,
        (pattern & (1 << 3)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(DSP_E_GPIO_Port, DSP_E_Pin,
        (pattern & (1 << 4)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(DSP_F_GPIO_Port, DSP_F_Pin,
        (pattern & (1 << 5)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(DSP_G_GPIO_Port, DSP_G_Pin,
        (pattern & (1 << 6)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
    	if (time_running) {
			// 1. Timekeeping
			ms_counter++;

			if (ms_counter >= 1000)
			{
				ms_counter = 0;
				seconds++;
			}
		}
    	if (state == IDLE)
    	{
    	    Display_IdleAnimationUpdate();
    	}

		Display_Scan();
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
#ifdef USE_FULL_ASSERT
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
