#include "RTC.h"

RTC_TimeTypeDef RTC_TimeStructure;
RTC_InitTypeDef RTC_InitStructure;
RTC_DateTypeDef RTC_DateStructure;
RTC_TimeTypeDef  RTC_TimeStampStructure;
RTC_DateTypeDef  RTC_TimeStampDateStructure;

unsigned int AsynchPrediv;
unsigned int SynchPrediv;

extern unsigned int setRTCFlag;

void rtc_check()
{
  if (RTC_ReadBackupRegister(RTC_BKP_DR0) == 0x32F2)
  {
        /* Check if the Power On Reset flag is set */
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
    {
      printf("\r\n Power On Reset occurred....\n\r");
    }
    /* Check if the Pin Reset flag is set */
    else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
    {
      printf("\r\n External Reset occurred....\n\r");
    }

    printf("\r\n No need to configure RTC....\n\r");
    
    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Allow access to RTC */
    PWR_BackupAccessCmd(ENABLE);

    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();

    /* Clear the RTC Alarm Flag */
    RTC_ClearFlag(RTC_FLAG_ALRAF);

    /* Clear the EXTI Line 17 Pending bit (Connected internally to RTC Alarm) */
    EXTI_ClearITPendingBit(EXTI_Line17);

    /* Get Current Date*/
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
  }
}

unsigned char week[7][4]={"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};

void RTC_TimeShow()
{
    /* Get the current Time and Date */
  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
//  printf("\n\r============== Current Time Display ============================\n\r");
  printf("\r\n  The current time (Hour-Minute-Second) is :  %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
  /* Unfreeze the RTC DR Register */
  (void)RTC->DR;
  
}

void RTC_DateShow()
{
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
    printf("\n\r  The current date is :  20%0.2d-%0.2d-%0.2d-%s \n\r", RTC_DateStructure.RTC_Year, RTC_DateStructure.RTC_Month, RTC_DateStructure.RTC_Date, week[RTC_DateStructure.RTC_WeekDay]);
}

void RTC_TimeStampShow(void)
{
  /* Get the current TimeStamp */
  RTC_GetTimeStamp(RTC_Format_BIN, &RTC_TimeStampStructure, &RTC_TimeStampDateStructure);
  printf("\n\r==============TimeStamp Display (Time and Date)=================\n\r");
  printf("\n\r  The current timestamp time (Hour-Minute-Second) is :  %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStampStructure.RTC_Hours, RTC_TimeStampStructure.RTC_Minutes, RTC_TimeStampStructure.RTC_Seconds);
  printf("\n\r  The current timestamp date (WeekDay-Date-Month) is :  %0.2d-%0.2d-%s \n\r", RTC_TimeStampDateStructure.RTC_WeekDay, RTC_TimeStampDateStructure.RTC_Date, week[RTC_TimeStampDateStructure.RTC_Month]);
}

unsigned int rtc_init()
{
  /* RTC configuration  */
  RTC_Config();
  
  /* Configure the RTC data register and RTC prescaler */
  RTC_InitStructure.RTC_AsynchPrediv = AsynchPrediv;
  RTC_InitStructure.RTC_SynchPrediv = SynchPrediv;
  RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
  
  /* Check on RTC init */
  if (RTC_Init(&RTC_InitStructure) == ERROR)
  {
    printf("\n\r        /!\\***** RTC Prescaler Config failed ********/!\\ \n\r");
    return RTC_FAIL;
  }
  else
  {
    printf("RTC Success!!\r\n");
    return RTC_SUCCESS;
  }
}


#define RTC_CLOCK_SOURCE_LSE   /* LSE used as RTC source clock */
void RTC_Config(void)
{
  /* Enable the PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Allow access to RTC */
  PWR_BackupAccessCmd(ENABLE);

#if defined (RTC_CLOCK_SOURCE_LSI)  /* LSI used as RTC source clock*/
/* The RTC Clock may varies due to LSI frequency dispersion. */
  /* Enable the LSI OSC */ 
  RCC_LSICmd(ENABLE);

  /* Wait till LSI is ready */  
  while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

  SynchPrediv = 0xFF;
  AsynchPrediv = 0x7F;

#elif defined (RTC_CLOCK_SOURCE_LSE) /* LSE used as RTC source clock */
  /* Enable the LSE OSC */
  RCC_LSEConfig(RCC_LSE_ON);

  /* Wait till LSE is ready */  
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  SynchPrediv = 0xFF;
  AsynchPrediv = 0x7F;
    
#else
#endif /* RTC_CLOCK_SOURCE_LSI */

  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();

  /* Enable The TimeStamp */
  RTC_TimeStampCmd(RTC_TimeStampEdge_Falling, ENABLE);    
}

/**
  * @brief  Display the current date on the Hyperterminal.
  * @param  None
  * @retval None
  */

/*void RTC_DateShow(void)
{
  // Get the current Date 
  RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
  printf("\n\r============== Current Date Display ============================\n\r");
  printf("\n\r  The current date (WeekDay-Date-Month-Year) is :  %0.2d-%0.2d-%0.2d-%0.2d \n\r", RTC_DateStructure.RTC_WeekDay, RTC_DateStructure.RTC_Date, RTC_DateStructure.RTC_Month, RTC_DateStructure.RTC_Year);
}*/

/**
  * @brief  Display the current time on the Hyperterminal.
  * @param  None
  * @retval None
  */

/*void RTC_TimeShow(void)
{
  // Get the current Time 
  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
  printf("\n\r  The current time is :  %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
}*/

/**
  * @brief  Gets numeric values from the hyperterminal.
  * @param  MinValue: minimum value to be used.
  * @param  MaxValue: maximum value to be used.
  * @retval None
  */
uint8_t USART_Scanf(uint32_t MinValue, uint32_t MaxValue)
{
  uint32_t index = 0;
  uint32_t tmp[2] = {0, 0};

  while (index < 2)
  {
    /* Loop until RXNE = 1 */
    while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET)
    {}
    tmp[index++] = (USART_ReceiveData(USART3));
    if ((tmp[index - 1] < 0x30) || (tmp[index - 1] > 0x39))
    {
      printf("\n\r Please enter valid number between 0 and 9\n\r");
      index--;
    }
  }
  /* Calculate the Corresponding value */
  index = (tmp[1] - 0x30) + ((tmp[0] - 0x30) * 10);
  /* Checks the value */
  if ((index < MinValue) || (index > MaxValue))
  {
    printf("\n\r Please enter valid number between %d and %d\n\r", MinValue, MaxValue);
    return 0xFF;
  }
  return index;
}

/**
  * @brief  Returns the time entered by user, using Hyperterminal.
  * @param  None
  * @retval None
  */
void RTC_TimeRegulate(void)
{
  uint32_t tmp_hh = 0xFF, tmp_mm = 0xFF, tmp_ss = 0xFF;

  printf("\n\r==============Time Settings=====================================\n\r");
  RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
  printf("  Please Set Hours\n\r");
  
  setRTCFlag = TRUE;
  while (tmp_hh == 0xFF)
  {
    tmp_hh = USART_Scanf(0, 23);
    RTC_TimeStructure.RTC_Hours = tmp_hh;
  }
  printf(":  %0.2d\n\r", tmp_hh);
  
  printf("  Please Set Minutes\n\r");
  while (tmp_mm == 0xFF)
  {
    tmp_mm = USART_Scanf(0, 59);
    RTC_TimeStructure.RTC_Minutes = tmp_mm;
  }
  printf(":  %0.2d\n\r", tmp_mm);
  
  printf("  Please Set Seconds\n\r");
  while (tmp_ss == 0xFF)
  {
    tmp_ss = USART_Scanf(0, 59);
    RTC_TimeStructure.RTC_Seconds = tmp_ss;
  }
  printf(":  %0.2d\n\r", tmp_ss);

  /* Configure the RTC time register */
  if(RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure) == ERROR)
  {
    printf("\n\r>> !! RTC Set Time failed. !! <<\n\r");
  } 
  else
  {
    printf("\n\r>> !! RTC Set Time success. !! <<\n\r");
    RTC_TimeShow();
    /* Indicator for the RTC configuration */
    RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
  }
  
  tmp_hh = 0xFF;
  tmp_mm = 0xFF;
  tmp_ss = 0xFF;

  printf("\n\r==============Date Settings=====================================\n\r");

  printf("  Please Set WeekDay (01-07)\n\r");
  while (tmp_hh == 0xFF)
  {
    tmp_hh = USART_Scanf(1, 7);
    RTC_DateStructure.RTC_WeekDay = tmp_hh;
  }
  printf(":  %0.2d\n\r", tmp_hh);
  tmp_hh = 0xFF;
  printf("  Please Set Date (01-31)\n\r");
  while (tmp_hh == 0xFF)
  {
    tmp_hh = USART_Scanf(1, 31);
    RTC_DateStructure.RTC_Date = tmp_hh;
  }
  printf(":  %0.2d\n\r", tmp_hh);
  
  printf("  Please Set Month (01-12)\n\r");
  while (tmp_mm == 0xFF)
  {
    tmp_mm = USART_Scanf(1, 12);
    RTC_DateStructure.RTC_Month = tmp_mm;
  }
  printf(":  %0.2d\n\r", tmp_mm);
  
  printf("  Please Set Year (00-99)\n\r");
  while (tmp_ss == 0xFF)
  {
    tmp_ss = USART_Scanf(0, 99);
    RTC_DateStructure.RTC_Year = tmp_ss;
  }
  printf(":  %0.2d\n\r", tmp_ss);

  /* Configure the RTC date register */
  if(RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure) == ERROR)
  {
    printf("\n\r>> !! RTC Set Date failed. !! <<\n\r");
  } 
  else
  {
    printf("\n\r>> !! RTC Set Date success. !! <<\n\r");
    RTC_DateShow();
    /* Indicator for the RTC configuration */
    RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
  }
  
  setRTCFlag = FALSE;

}
