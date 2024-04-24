/**
  ******************************************************************************
  * @file    Peripheral.h 
  * @author  sbKim
  * @version V1.0.0
  * @date    2015/05/21
  * @brief   Header for Peripheral.h module
  ******************************************************************************
**/
#ifndef __PERIPHERAL_H
#define __PERIPHERAL_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#define ADC3_DR_ADDRESS    ((uint32_t)0x4001204C)
static __IO uint16_t ADCTripleConvertedValue[1];

void TIM_Config(void);
void Com_init(USART_InitTypeDef* USART_InitStruct);
void Uart_init();
void Uart6_init();
void ozoneSensorInit();
//double getOzoneSensor();

// kwon: 2024-4-25, uart6 rx int for esp8226
#define MAX_ESP_RX_BUFFER      1024
#define MAX_ESP_COMMAND_LEN    64
#define MAX_ESP_CLIENT_NUM     10

typedef struct _cb_data_t
{
    uint8_t buf[MAX_ESP_RX_BUFFER];
    uint16_t length;
}cb_data_t;

#endif /* __PERIPHERAL_H */