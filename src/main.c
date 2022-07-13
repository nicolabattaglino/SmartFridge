
#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"
#include "SPI_temperature.h"
#include "SPI_VOC.h"
#include "string.h"

#define DIM 13
#define DEBUG_ADDRESS   0x80000010;
static void MX_TIM7_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI3_Init(void);
void SystemClock_Config(void);
void Print_DEBUG(char* message);
void Error_Handler(void);
void VOCReceive(SPI_HandleTypeDef hspi3,uint8_t* vocReceivedSPI3);
void temperatureReceive(SPI_HandleTypeDef hspi1,uint8_t* tempReceivedSPI1);


int aperto;
int VOCflag;
int Tempflag;
uint8_t tempReceivedSPI1[DIM];
uint8_t vocReceivedSPI3[DIM];

TIM_HandleTypeDef htim3; 
TIM_HandleTypeDef htim2; 
TIM_HandleTypeDef htim7; 

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi3;

int main(void)
{

  HAL_Init();
  SystemClock_Config();

  MX_TIM7_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_SPI1_Init();
  MX_SPI3_Init();


  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();


  GPIO_InitTypeDef GPIO_InitStruct;

/////////////////////////////////////////////////////////
// led initialization : GPIOA PIN 5

  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // led off
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);

/////////////////////////////////////////////////////////
// button initialization : GPIOA PIN 3
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);

  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;

  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/////////////////////////////////////////////////////////
// Buzzer initialization : GPIOA PIN 12
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  // frigo inizialmente chiuso
  aperto = 0;
  // init flag voc 
  VOCflag = 0;

  // init temperature flag
  Tempflag = 0;

/////////////////////////////////////////////////////////
// start timer for periodically /////////////////////////
// sensors reading (Temperature and VOC) ////////////////
/////////////////////////////////////////////////////////


if( HAL_TIM_Base_Start_IT(&htim7) != HAL_OK) {
              Print_DEBUG("HAL_TIM_Base_Start_IT7");
              Error_Handler();
      }

Print_DEBUG("--- Start main loop --- ");
  while (1)
  {
    

  }
}

////////////////////////////////////////////////
////// call back gpio interrupt ////////////////
////////////////////////////////////////////////

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_3){

    if (aperto == 0){
      aperto = 1;
      Print_DEBUG("-----------------------------");
      Print_DEBUG("Fridge Door Open, timer started");
      Print_DEBUG("-----------------------------");
      if( HAL_TIM_Base_Start_IT(&htim3) != HAL_OK) {
        Print_DEBUG("HAL_TIM_Base_Start_IT1");
        Error_Handler();
   }
    } else if (aperto == 1){
      aperto = 0;
      Print_DEBUG("-----------------------------");
      Print_DEBUG("Fridge Door closed, timer stopped");
      Print_DEBUG("-----------------------------");
      if( HAL_TIM_Base_Stop_IT(&htim2) != HAL_OK) {
             Print_DEBUG("HAL_TIM_Base_Start_IT2");
             Error_Handler();
        }
       if( HAL_TIM_Base_Stop_IT(&htim3) != HAL_OK) {
        Print_DEBUG("HAL_TIM_Base_Start_IT3");
        Error_Handler(); 
        }
        
    }
  }
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);
  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_GPIO_EXTI_Callback could be implemented in the user file
   */
}


////////////////////////////////////////////////
////// call elapsed timers /////////////////////
////////////////////////////////////////////////

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

if(htim == &htim7) {

    // temperature reading
    hspi1.Instance->DR = 0; // output something on MOSI while reading MISO

 while ((hspi1.Instance->SR & SPI_FLAG_RXNE) == 0){} 
    temperatureReceive(hspi1,tempReceivedSPI1);
    float value_temp = ((150.0/255.0)*tempReceivedSPI1[0]) - 40;  // (precision * value between 0-255) - 40 [temperature range -40 to 110 C]
    char s[4];
    gcvt(value_temp,3,s);
    Print_DEBUG("-----------------------------");
    Print_DEBUG("Receive Temperature data: ");
    Print_DEBUG(s);
    Print_DEBUG("-----------------------------");

    if (Tempflag == 0 && (value_temp > 10 || value_temp < -8 )){
          Print_DEBUG("-----------------------------");
          Print_DEBUG("Critic Temperature");
          Print_DEBUG("-----------------------------");

          if( HAL_TIM_Base_Stop_IT(&htim2) != HAL_OK) {
            Print_DEBUG("HAL_TIM_Base_Stop_IT2");
            Error_Handler();
          }
          Tempflag = 1;
          if( HAL_TIM_Base_Start_IT(&htim2) != HAL_OK) {
            Print_DEBUG("HAL_TIM_Base_Start_IT2");
            Error_Handler();
          }
    } else if (Tempflag == 1 && (value_temp <= 10 && value_temp >= -8 )){
          Print_DEBUG("-----------------------------");
          Print_DEBUG("Fridge returned to Normal Temperature");
          Print_DEBUG("-----------------------------");

          Tempflag = 0;
          if( HAL_TIM_Base_Stop_IT(&htim2) != HAL_OK) {
            Print_DEBUG("HAL_TIM_Base_Stop_IT2");
            Error_Handler();
          }
    }

    // VOC reading

hspi3.Instance->DR = 0; // output something on MOSI while reading MISO

 while ((hspi3.Instance->SR & SPI_FLAG_RXNE) == 0){} 
    VOCReceive(hspi3,vocReceivedSPI3);
    int value_voc = vocReceivedSPI3[0];  //percentage between 0-100
    char s_voc[4];
    itoa(value_voc,s_voc,10);
    Print_DEBUG("-----------------------------");
    Print_DEBUG("Receive VOC data: ");
    Print_DEBUG(s_voc);
    Print_DEBUG("-----------------------------");


    if (VOCflag == 0 && value_voc > 50){
      VOCflag = 1;
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      Print_DEBUG("---------------------------------------");
      Print_DEBUG("LED ON, volatile compounds detected");
      Print_DEBUG("---------------------------------------");
    } else if (VOCflag == 1 && value_voc <= 50){
      VOCflag = 0;
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      Print_DEBUG("----------------------------------------------------");
      Print_DEBUG("LED OFF,volatile compounds level returned to normal");
      Print_DEBUG("----------------------------------------------------");

    }

  }

 ///////////////////////////////////////////
 ///// timer 2 periodical buzzer sound /////
 ///////////////////////////////////////////

  if(htim == &htim2) {
    // pin buzzer toggle
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_12);
    Print_DEBUG("Buzzer alarm playing");
  }

 ///////////////////////////////////////////
 ///// timer 3: when elapsed start /////////
 ///// the timer for buzzer sound //////////
 ///////////////////////////////////////////

  if(htim == &htim3) {
    if( HAL_TIM_Base_Stop_IT(&htim3) != HAL_OK) {
    Print_DEBUG("HAL_TIM_Base_Start_IT1");
      Error_Handler();
   }
   Print_DEBUG("---------------------------------------");
    Print_DEBUG("Door timer elapsed, play buzzer sound");

    // start timer 2 for toggle of buzzer pin
    if( HAL_TIM_Base_Stop_IT(&htim2) != HAL_OK) {
            Print_DEBUG("HAL_TIM_Base_Stop_IT2");
            Error_Handler();
          }
    if( HAL_TIM_Base_Start_IT(&htim2) != HAL_OK) {
              Print_DEBUG("HAL_TIM_Base_Start_IT2");
              Error_Handler();
      }
    }      

}

void Print_DEBUG(char* message){
  char *p = (char*)DEBUG_ADDRESS;
  char start[9] = "[NOTIFY] ";
  for (int g = 0; g < 8; g++){
    *p = start[g];
  }
  for (int g = 0; message[g] != '\0'; g++){
    *p = message[g];
  }
  *p = '\n';
}

// timer 1 clock config set to 25MHz

void SystemClock_Config()
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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}


///////////////////////////////////
///// SPI1 Temperature sensor /////
///////////////////////////////////

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

///////////////////////////////////
///// SPI3 VOC sensor /////////////
///////////////////////////////////

static void MX_SPI3_Init(void)
{

  /* SPI1 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }

}

 // 8 seconds ((25MHz/100000) * 2000)

////////////////////////////////////////////////////
///// Timer 7 read Temperature and VOC sensors /////
////////////////////////////////////////////////////

static void MX_TIM7_Init(void)
{


  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 100000;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 1000; 
  htim7.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim7, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }


}

///////////////////////////////////
///// Timer 2 for buzzer on/off ///
///////////////////////////////////

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{


  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};


  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 100000;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 100;
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
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

////////////////////////////////////////////
///// Timer 3, door opened, start buzzer /////
////////////////////////////////////////////

static void MX_TIM3_Init(void)
{



  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};


  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 100000;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 2000;
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
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
 

}


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

