/**
  ******************************************************************************
  * @file    Peripheral.c
  * @author  sbKim
  * @version V1.0.0
  * @date    2015/05/21
  * @brief   Code for Peripheral.c module
  ******************************************************************************
**/

#include "Peripheral.h"

//#ifndef HPA_36C
#if ( MACHINE == HPA_130W )
  #define SENSOR_BUFFER_SIZE    10
  static unsigned char bufIndex=0;
  static unsigned short sensorBuf[SENSOR_BUFFER_SIZE]={0};
  static unsigned short avgBuf[SENSOR_BUFFER_SIZE]={0};
  extern unsigned int dOzoneSensoredValue;
#endif

#ifdef  INCLUDE_DIP_SWITCH
  extern unsigned char statusDIP1;                   // SJM 200619 default value = 1;
#endif

//#ifdef  HPA_36C
#if ( MACHINE == HPA_36C )
  extern unsigned char intCounter;
void inputPortInit()    // SJM 200818 added because of DC sensor port
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable the key Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  
  /* Configure the Input pin : DC sensor port */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
#ifdef  LPP_V1_00_BD
  /* Enable the key Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  /* Configure the Relay pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;     // SJM 201014 BAT_ID input
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  // only valid for NOPULL
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
#endif
}
#endif  // HPA_36C, ( MACHINE == HPA_36C )

#ifdef  INCLUDE_BATTERY_CHECKER
  #define VOLTAGE_BUFFER_SIZE    20
  static unsigned char voltBufIndex=0;
  static unsigned char avgBufSize=0;
  static unsigned short voltageBuf[VOLTAGE_BUFFER_SIZE]={0};
  static unsigned short voltageAvgBuf[VOLTAGE_BUFFER_SIZE]={0};
  extern unsigned int voltageValue;
  unsigned char stabilizedVolt = FALSE;

void readBatteryVoltage()
{
  unsigned int temp;
  unsigned int analogInput;
  unsigned int avgADCout;
  register int i,j;
  
  voltageBuf[voltBufIndex] = ADCTripleConvertedValue[1];
//  if (sensorBuf[bufIndex]>=4095) return;    // SJM 190812 remove sensor saturation
  if (avgBufSize<VOLTAGE_BUFFER_SIZE)  avgBufSize++;
  if (avgBufSize>=5) stabilizedVolt = TRUE;
  if (++voltBufIndex==VOLTAGE_BUFFER_SIZE) {
    voltBufIndex = 0;
  }
  if (avgBufSize>10) {
    for (i=0;i<avgBufSize;i++)
      voltageAvgBuf[i] = voltageBuf[i];
    for (i=0;i<(avgBufSize-1);i++) 
      for (j=i+1;j<avgBufSize;j++)
        if (voltageAvgBuf[i]>voltageAvgBuf[j]) {
          temp = voltageAvgBuf[i];
          voltageAvgBuf[i] = voltageAvgBuf[j];
          voltageAvgBuf[j] = temp;
        }
    for (i=1,temp=0;i<(avgBufSize-1);i++)   // remove min & Max
      temp += voltageAvgBuf[i];
    avgADCout = (unsigned int)(((float)temp/(avgBufSize-2)));
  }
  else {
    for (i=0,temp=0;i<avgBufSize;i++)
      temp += voltageBuf[i];
    avgADCout = (unsigned int)(((float)temp/(avgBufSize)));
  }
    analogInput = (unsigned int)(avgADCout*ADC_VOLTAGE);            // in mV
//    voltageValue = (unsigned int)(analogInput*COEF_SLOPE+COEF_INTERCEPT); 
    voltageValue = (unsigned int)(1.61*analogInput);
    // 0.161 = 1610K/100K/1000(mV)*10 ==> x10 200826 SJM
}
#endif  // INCLUDE_BATTERY_CHECKER

//#ifndef HPA_36C
#if ( MACHINE == HPA_130W )
  #ifdef  INCLUDE_OZONE_CONTROL
void readOzoneSensor()
{
  unsigned int temp,tempVal;
  int i,j;
  
  sensorBuf[bufIndex] = ADCTripleConvertedValue[2];
//  if (sensorBuf[bufIndex]>=4095) return;    // SJM 190812 remove sensor saturation
  if (++bufIndex==SENSOR_BUFFER_SIZE) {
    bufIndex = 0;
  }
  {
    for (i=0;i<SENSOR_BUFFER_SIZE;i++)
      avgBuf[i] = sensorBuf[i];
    for (i=0;i<(SENSOR_BUFFER_SIZE-1);i++) 
      for (j=i+1;j<SENSOR_BUFFER_SIZE;j++)
        if (avgBuf[i]>avgBuf[j]) {
          temp = avgBuf[i];
          avgBuf[i] = avgBuf[j];
          avgBuf[j] = temp;
        }
    for (i=1,temp=0;i<(SENSOR_BUFFER_SIZE-1);i++)   // remove min & Max
      temp += avgBuf[i];
    #ifdef  INCLUDE_DIP_SWITCH
    if (statusDIP1)     // SJM 200619 before main B/D ver 2.1
      // saturated at 3.0V, 153ppb = 3/5*255 by calculation.
      dOzoneSensoredValue = (unsigned int)(((float)temp/(SENSOR_BUFFER_SIZE-2))/4095*153.0);
    else {              // SJM 200619 from main B/D ver 2.1
      // SJM 200428 Vmax = 2.67V 200608 due to voltage divider  
      // saturated at 2.67V ==> 207ppb (in case of AD/16)
      // 255ppb*2.67V/2.67V = 255, 3V:4095 = 2.67V:x ==> X = 2.67/3*4095
      // in some case, saturated voltage can be dropped to 2.64 ==> 3300 was choosen.
      tempVal = (unsigned int)(((float)temp/(SENSOR_BUFFER_SIZE-2))*255.0/3300);
      if (tempVal>255)  dOzoneSensoredValue = 255;
      else              dOzoneSensoredValue = (unsigned int)tempVal;
    }
    #else
      #ifdef  USE_250PPB_SENSOR
        #ifdef  ADC_REFERENCE_3V
    // Vref = 3V  255ppb*3V/5V = 153, if Vref =3.3V 255*3.3/5 = 168.3
    dOzoneSensoredValue = (unsigned int)(((float)temp/(SENSOR_BUFFER_SIZE-2))/4095*153.0);
        #else
    dOzoneSensoredValue = (unsigned int)(((float)temp/(SENSOR_BUFFER_SIZE-2))/16);
        #endif  // ADC_REFERENCE_3V
      #else // in case of using 100 ppb sensor
    dOzoneSensoredValue = (unsigned int)(((float)temp/(SENSOR_BUFFER_SIZE-2))/40.96); 
      #endif
    #endif  // INCLUDE_DIP_SWITCH
  }
}
  #else // INCLUDE_OZONE_CONTROL
double getOzoneSensor()
{
    double dOzoneValue;
    #ifdef USE_250PPB_SENSOR     // sensor range = 0~255
      #ifdef  ADC_REFERENCE_3V
    // Vref = 3V  255ppb*3V/5V = 153, if Vref =3.3V 255*3.3/5 = 168.3
    dOzoneValue = ADCTripleConvertedValue[2] / 4095*153.0;
      #else
    dOzoneValue = ADCTripleConvertedValue[2] / 16.0;
      #endif // ADC_REFERENCE_3V
    #else                         // sensor range = 0~100
    dOzoneValue = ADCTripleConvertedValue[2] / 40.96;
    #endif
//  printf("\r\nLocal Ozone ADC Value = %d ", (ADCTripleConvertedValue[2] & 0xffff));
//  printf("\r\nLocal Ozone Sensor Value = %5.2f ppb", dOzoneValue);
  return  dOzoneValue;
}
  #endif  // INCLUDE_OZONE_CONTROL
#endif  // HPA_36C  #if ( MACHINE == HPA_130W )

void ozoneSensorInit()
{
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;

  /* Enable ADC1, DMA2 and GPIO clocks ****************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  /* DMA2 Stream0 channel2 configuration **************************************/
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC3_DR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADCTripleConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 6;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);
  DMA_Cmd(DMA2_Stream0, ENABLE);

  /* Configure ADC1 Channel 12 pin as analog input ******************************/
  #ifdef  INCLUDE_BATTERY_CHECKER
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_1;  // PC1 votage sensor
  #else
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;     // PC2 ozone sensor
  #endif
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC1 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 6;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel10,11,12,13,14,15 configuration *************************************/
  #ifdef  INCLUDE_BATTERY_CHECKER
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_3Cycles);
  #endif
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_3Cycles);

 /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
}

/**
  * 1ms���� �߻��Ǵ� Timer ���ͷ�Ʈ�� ���� �Լ�.
  */
void TIM_Config(void)
{ 
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  NVIC_InitTypeDef  NVIC_InitStructure;

  /* Enable TIM2, TIM3 and TIM4 clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  // (84000000Hz / 1680) / 50 = 1000Hz = 1ms  
  /* TIM2 configuration */
  TIM_TimeBaseStructure.TIM_Period = 49;
  TIM_TimeBaseStructure.TIM_Prescaler = 1679;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;    
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  TIM_OCStructInit(&TIM_OCInitStructure);

  /* Immediate load of TIM2 Precaler values */
  TIM_PrescalerConfig(TIM2, 1679, TIM_PSCReloadMode_Immediate);

  /* Clear TIM2 update pending flags */
  TIM_ClearFlag(TIM2, TIM_FLAG_Update);

  //printf("go to jump!!!!!\r\n");
  
  //for usb firmware upgrade
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0xC000);
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* Enable the TIM2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable TIM2, TIM3 and TIM4 Update interrupts */
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

  /* TIM2, TIM3 and TIM4 enable counters */
  TIM_Cmd(TIM2, ENABLE);
}

/**
  * Debugging(COM3) ��Ʈ�� ���� �ʱ� �Լ�.
  */
void Com_init(USART_InitTypeDef* USART_InitStruct)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);    
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  
    /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);

  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

  /* Configure USART Tx as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Configure USART Rx as alternate function  */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* USART configuration */
  USART_Init(USART3, USART_InitStruct);
    
  /* Enable USART */
  USART_Cmd(USART3, ENABLE);
}

/**
  * UART3�� ���ͷ�Ʈ �� ��Ʈ ������ ���� �Լ�.
  */
void Uart_init()
{
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
  Com_init(&USART_InitStructure);
  
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
}


void Com6_init(USART_InitTypeDef* USART_InitStruct)
{
  
  printf("\n Com6_init .....");
  
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_DeInit(GPIOC);

//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);    
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
  
    /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);

  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

  /* Configure USART Tx as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure USART Rx as alternate function  */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* USART configuration */
  USART_Init(USART6, USART_InitStruct);
//  USART_ITConfig(USART6, USART_IT_TXE, ENABLE);
//  USART_ITConfig(USART6, USART_IT_TC, ENABLE);
  
    
  /* Enable USART */
  USART_Cmd(USART6, ENABLE);
}

/**
  * UART6�� ���ͷ�Ʈ �� ��Ʈ ������ ���� �Լ�.
  */
void Uart6_init()
{
  
  printf("\nUart6_init .....");
  
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  // kwon: rasperry는 115200 속도
  USART_InitStructure.USART_BaudRate = 115200;

  // kwon: esp8266은 96000 
  // USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
  Com6_init(&USART_InitStructure);
  
  printf("\nUart6 ITConfig .....");

// kwon: 2024-4-21 임시 테스트 인터럽터를 막았다.
   USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);
}


#ifdef  INCLUDE_STOP_MODE
#if 0
void Uart5_init_org()      // remote port
{
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStructure.USART_Mode = USART_Mode_Rx;
  
  Com_init(&USART_InitStructure);
  
  //USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
  
  USART_ITConfig(USART5, USART_IT_RXNE, ENABLE);

}

void Uart5_SJM_init()
{
  EXTI_InitTypeDef EXTI_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_DeInit(GPIOD);       // SJM 200819 for sure
  
  // configure EXTI Line 15
  EXTI_ClearITPendingBit(EXTI_Line15);
  EXTI_InitStructure.EXTI_Line = EXTI_Line15;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx;
  
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);    
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);    
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
  
  /* Connect PXx to USARTx_Tx*/
//  GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);   SJM 200819
  
  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_UART5);
  
  /* Configure USART Tx as alternate function  */
/*
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
*/  
  /* Configure USART Rx as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  /* USART configuration */
  USART_Init(UART5, &USART_InitStructure);
  
  /* Enable USART */
  USART_Cmd(UART5, ENABLE);
  
  //USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
  USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
}
#endif // 0
void prepareRemoteSensor()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  intCounter = 0;
  GPIO_DeInit(GPIOD);       // SJM 200819 for sure
  
  /* Enable the BUTTON Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);    
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /* Configure Button pin as input */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_Init(GPIOD, &GPIO_InitStructure);


    /* Connect Button EXTI Line to Button GPIO Pin */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource2);

    /* Configure Button EXTI line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

      EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set Button EXTI Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; // 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure); 
}

#endif