#include "pir.h"

void pirInit()
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable the key Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  /* Configure the Relay pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

unsigned char pirPort1, pirPort2;

unsigned int getPirPort1()
{

    pirPort1 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4);
    if(pirPort1 == SET) return TRUE;
    else  return FALSE;

}

unsigned int getPirPort2()
{
    pirPort2 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3);
    if(pirPort2 == SET) return TRUE;
    else  return FALSE;  
}

