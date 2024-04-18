#include "relay.h"

unsigned char ozoneLampOnFlag = FALSE;
unsigned char uvLampOnFlag = FALSE;
unsigned char rciOnFlag = FALSE;
unsigned char fan2OnFlag = FALSE;

#ifdef  INCLUDE_POWER_CONTROL
void initPowerControl()  // only for test in Shell....
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable the GPIO_LED Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
  /* Configure the Relay pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | 
    GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_13 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_WriteBit(GPIOA,GPIO_Pin_15,Bit_SET);
}
#endif
void relayControlInit()
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable the GPIO_LED Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  /* Configure the Relay pin */
#ifdef INCLUDE_POWER_CONTROL
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | 
    GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_13 | GPIO_Pin_15;
#else
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | 
    GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_13;
#endif
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
#ifdef INCLUDE_POWER_CONTROL
  GPIO_WriteBit(GPIOA,GPIO_Pin_15,Bit_SET);
#endif  
    /* Configure the Relay pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}
#if 0       // SJM 190715 never used!!!
void relayTest()
{
  /* Set PG6 and PG8 */
    GPIOA->BSRRL = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_13;
    GPIOB->BSRRL = GPIO_Pin_0 | GPIO_Pin_1;
    Delay(1000);
    /* Reset PG6 and PG8 */
    GPIOA->BSRRH = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_13;
    GPIOB->BSRRH = GPIO_Pin_0 | GPIO_Pin_1;
}
#endif
void relayControl(unsigned char relay, unsigned char onoff)
{
  switch(relay) {
  case RELAY_AC_FAN1:
    if(onoff == RELAY_ON) GPIOA->BSRRL = GPIO_Pin_3;
    else                  GPIOA->BSRRH = GPIO_Pin_3;
    break;
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  case RELAY_OZONE_LAMP:
    ozoneLampOnFlag = onoff;
    if(onoff == RELAY_ON) GPIOA->BSRRL = GPIO_Pin_2;
    else                  GPIOA->BSRRH = GPIO_Pin_2;

    printf("\r\n realy.c : [OzoneLamp=%d]",onoff);

    break;
  case RELAY_AC_UV:
    uvLampOnFlag = onoff;
    if(onoff == RELAY_ON) GPIOB->BSRRL = GPIO_Pin_0;
    else                  GPIOB->BSRRH = GPIO_Pin_0;
    break;
  case RELAY_AC_FAN2:
    fan2OnFlag = onoff;
    if(onoff == RELAY_ON) GPIOA->BSRRL = GPIO_Pin_13;
    else                  GPIOA->BSRRH = GPIO_Pin_13;
    break;
#endif
  case RELAY_PLASMA:
    if(onoff == RELAY_ON) GPIOB->BSRRL = GPIO_Pin_1;
    else                  GPIOB->BSRRH = GPIO_Pin_1;
#ifdef  HUNGARIAN_ISSUE
    printf(" relay.c : [Plate1=%d]",onoff);
#endif
    break;    
  case RELAY_PLASMA2:
    if(onoff == RELAY_ON) GPIOA->BSRRL = GPIO_Pin_0;
    else                  GPIOA->BSRRH = GPIO_Pin_0;
#ifdef  HUNGARIAN_ISSUE
    printf(" realy.c [Plate2=%d]",onoff);
#endif
    break;
  case RELAY_SPI:
    if(onoff == RELAY_ON) GPIOA->BSRRL = GPIO_Pin_4;
    else                  GPIOA->BSRRH = GPIO_Pin_4;
    break;
  case RELAY_RCI:
    rciOnFlag = onoff;
    if(onoff == RELAY_ON) GPIOA->BSRRL = GPIO_Pin_1;
    else                  GPIOA->BSRRH = GPIO_Pin_1;
    break;
  }
}