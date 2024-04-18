/***********************************************************************************************************************
 * Project      : õ���� ���� û����
 * Description  : �ö�� �� UV ������ �̿��� ���� �ҵ��⸦ �����ϴ� ���α׷�
 * Author       : ����
 * Date         : 2015-05-21
 * Version      : 0.01
 * History      :
  2015-05-21 - stm32f4�� systick ���α׷����� ���� ���� �ۼ�.
************************************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "main.h"
#include "Peripheral.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  TimingDelay_Decrement();
}

extern unsigned char continueOperation;    // SJM 190724 unify continue operation
extern unsigned int testLedBlink;
unsigned int testLedTimer = 0;
unsigned int ledTimer = 0;
unsigned int g_60hz;
//extern unsigned int G1_SF;
//extern unsigned int G2_SF;
//extern unsigned int G3_SF;
//extern unsigned int G4_SF;
extern unsigned int currentState;
extern unsigned int segBlinkOnTime;
extern unsigned char ozoneLampOnFlag;
extern unsigned char uvLampOnFlag;
extern unsigned char rciOnFlag;
extern unsigned char fan2OnFlag;
extern PLASMAINFO plasmaInfo;
extern DISINFO disInfo;
extern IONINFO ionInfo;
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  extern void readOzoneSensor();
#endif
#ifdef  INCLUDE_STOP_MODE
  unsigned char stopFlag = 0;
  unsigned char intCounter = 0;
  short curSec, prevSec;
  unsigned char resetFlag;
  unsigned char countIdleTime = 0;  // flag to count Idle Time
  unsigned short idleTimeSec = 0;
  unsigned short idleTimer = 0;
  unsigned char enterStopMode = 0;
#endif
#define OZONE_SENSOR_CHECK_TIME     1000     // SJM 190812 check ozone sensor every 500mSec.
                                              // SJM 190827 500 -> 1000
//PIR
//#define PID_CHECK_TIME 5000
#define PID_CHECK_TIME 1
#define PID_CHECK_INTERVAL          100     // SJM 190820 check PID sensor every 100mSec.
#define PID_CHECK_COUNT             3       // SJM 190820 check PID sensor at least 3 times
unsigned int pidDetectedStart;
unsigned int pidOnTime;
unsigned int pidCheckTimer = 0;             // SJM 190820 add to fix PID check routine
unsigned int pidCheckCount = 0;             // SJM 190820 add to fix PID check routine

//unsigned int pidLEDOnFlag = FALSE;      SJM 190711 always assigned to 'FALSE' & never used

//#define LED_ON_TIME 5000          SJM 190809 neverUsed
//unsigned int ledOnTime;           SJM 190809 neverUsed

//�Ҹ�ǰ ī���� 
unsigned int plusCounterMin;

unsigned int oneSecTimer;
//unsigned int fiveSecTimer;        SJM 190809 currently no Use.(used in commented code)

extern unsigned char pidDetect;

//Timer
int prepareTimer;
int destructionTimer;

//plasma blink
unsigned char plasmaBlinkOn = TRUE;
unsigned int plasmaBlinkOnTimer = 0;
unsigned int plasmaBlinkOnFlag = FALSE;
unsigned int plasmaBlinkOffTimer = 0;
unsigned int plasmaBlinkOffFlag;

//unsigned int PlasmaOffTime = 0;   SJM 200907 NoUse
#ifdef  HUNGARIAN_ISSUE
unsigned int plasmaOnTime = PLASMA_BLINK_ON_TIME;
unsigned int plasmaOffTime = PLASMA_BLINK_OFF_TIME;
#endif

//#ifndef  HPA_36C
#if ( MACHINE == HPA_130W)
  unsigned int dOzoneSensoredValue;
#endif
extern unsigned int g_extOzoneSenseFlag;
extern unsigned int g_remoteFlag;

unsigned long  sysTickCounter = 0;
unsigned char  halfSecFlag = FALSE;
unsigned short sensorCheck =0;

//#ifdef  HPA_36C
#if ( MACHINE == HPA_36C)
  unsigned short  quarterSecCounter = 0;
  unsigned char  quarterSecFlag = FALSE;
  unsigned char  batteryCheckFlag = FALSE;
  unsigned short batteryCheck =0;
  unsigned char   blinkLED = FALSE;
  unsigned short  blinkLEDcounter=0;

  void readBatteryVoltage();
//#endif
//#ifdef  MONEY_STERILIZER
#elif ( MACHINE == MONEY_STERILIZER )
  int downTimer;
  unsigned char controllerStarted;
#endif

void TIM2_IRQHandler(void)
{
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

//  sysTickCounter++;     // SJM 190703 add general system counter.
  if (halfSecFlag==FALSE) {
    sysTickCounter++;
    if (sysTickCounter>=500) {
      halfSecFlag = TRUE;
      sysTickCounter =0;
    }
  }
//#ifdef  HPA_36C
  //key Check
  timer_key_check();
  //Led On control
  if(++ledTimer > 2) {
    ledTimer = 0;
    if(++g_60hz > 4) {
      g_60hz = 0;
    }
    timerLed();

    if( (currentState == STATE_STER)||(currentState == STATE_READY_STER)||
       (currentState == STATE_PREPARE)||(currentState == STATE_STER_STOP)) {

        PIRLedTimer();
    }
    else if( !(currentState == STATE_POWER_OFF)) {
      WhiteLed();
    }
  }
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  // ozone sensor check 
  if (++sensorCheck >= OZONE_SENSOR_CHECK_TIME) {
    sensorCheck = 0;
    readOzoneSensor();
  }
#endif

  //test timer and onesec Flag
  if(++testLedTimer >= ONESEC) {
      testLedTimer = 0;
      testLedBlink = TRUE;
      oneSecFlag = TRUE;
//      dOzoneSensoredValue = getOzoneSensor();
  }
  if (++pidCheckTimer >= PID_CHECK_INTERVAL) {
    pidCheckTimer = 0;
    if( (getPirPort1() == TRUE) || (getPirPort2() == TRUE) ) {
      if (++pidCheckCount>PID_CHECK_COUNT) {
        pidCheckCount = PID_CHECK_COUNT;
        pidDetect = TRUE;
        if(pidDetectedStart == FALSE) {
          pidDetectedStart = TRUE;
          pidOnTime++;
        }
        else {
          if(++pidOnTime > PID_CHECK_TIME) {
            pidDetectedStart = FALSE;
            pidOnTime = 0;
          }
        }
      }
    }
//    else if( (getPirPort1() == FALSE) && (getPirPort2() == FALSE) ) {
    else {
//      if (--pidCheckCount<=0) {
        pidCheckCount = 0;
        pidDetect = FALSE;
        if(pidDetectedStart == TRUE) {
          if(++pidOnTime > PID_CHECK_TIME) {
            pidDetectedStart = FALSE;
            pidOnTime = 0;
          } 
        }
//      }
    }
  }

  if(currentState == STATE_CONSUMABLE_WARNING) {
    if(segBlinkOnTime == TRUE) {
      if(++(sysConfig.blinkOnTimer) >= 750) {
        sysConfig.blinkOnTimer = 0;
        sysConfig.blinkOnFlag = TRUE;
        segBlinkOnTime = FALSE;
      }
    }
    else {
      if(++(sysConfig.blinkOffTimer) >= 250) {
        sysConfig.blinkOffTimer = 0;
        sysConfig.blinkOffFlag = TRUE;
        segBlinkOnTime = TRUE;
      }
    }
  } // end of (currentState == STATE_CONSUMABLE_WARNING)
  
  if (currentState == STATE_DESTRUCTION) {
    if(++oneSecTimer >= ONESEC) {
      oneSecTimer = 0;
      if(destructionTimer >= 0) {
        destructionTimer--;
      }
    }
    if(segBlinkOnTime == TRUE) {
      if(++(sysConfig.blinkOnTimer) >= 750) {
        sysConfig.blinkOnTimer = 0;
        sysConfig.blinkOnFlag = TRUE;
        segBlinkOnTime = FALSE;
      }
    }
    else {
      if(++(sysConfig.blinkOffTimer) >= 250) {
        sysConfig.blinkOffTimer = 0;
        sysConfig.blinkOffFlag = TRUE;
        segBlinkOnTime = TRUE;
      }
    }
  }     // end of (currentState == STATE_DESTRUCTION)
  
  if (currentState == STATE_PREPARE) {
    if(++oneSecTimer >= ONESEC) {
      oneSecTimer = 0;
      if(prepareTimer >= 0) {
        prepareTimer--;
      }  
    }    
    if(segBlinkOnTime == TRUE) {
      if(++(sysConfig.blinkOnTimer) >= 750) {
        sysConfig.blinkOnTimer = 0;
        sysConfig.blinkOnFlag = TRUE;
        segBlinkOnTime = FALSE;
      }
    }
    else
    {
      if(++(sysConfig.blinkOffTimer) >= 250)
      {
        sysConfig.blinkOffTimer = 0;
        sysConfig.blinkOffFlag = TRUE;
        segBlinkOnTime = TRUE;
      }
    }
  }       // end of (currentState == STATE_PREPARE)

  //�� ���� �ð� ī��Ʈ �� ����, ����, ī����
//  if(currentState == STATE_STER || currentState == STATE_DIS ||  currentState == STATE_ION) {
    if(++plusCounterMin >= ONESEC * 60) { //minite
      plusCounterMin = 0;
      if (fan2OnFlag)       sysConfig.filterCountMin++;     // SJM 190723 Fan1&2 run at the same time.
      if (rciOnFlag)        sysConfig.rciOperatingMin++;
//#ifndef HPA_36C  // SJM 200413 No UV-Lamp and Ozpne-Lamp in HPA_36C
#if ( MACHINE == HPA_130W)
      if (uvLampOnFlag)     sysConfig.uvLampCountMin++;
      if (ozoneLampOnFlag)  sysConfig.ozoneLampCountMin++;
#endif
    }
//  }   // end of "Time Count"

  //plasma mode Interrupt
  if((currentState == STATE_STER) || (currentState == STATE_READY_STER) || (currentState == STATE_STER_STOP)) {
    if(currentState == STATE_STER) {
      if (++plasmaInfo.plasmaOneSec >= ONESEC) {
//        sterOzoneDetectFlag = TRUE;       SJM 190711 assigned but never used.
        plasmaInfo.plasmaOneSec = 0;
        if (plasmaInfo.plasmaTimer > 0 && continueOperation == FALSE)
          plasmaInfo.plasmaTimer--;
        //plasma blink
        if(plasmaBlinkOn == TRUE) {
#ifndef HUNGARIAN_ISSUE
          if(++(plasmaBlinkOnTimer) >= PLASMA_BLINK_ON_TIME) {
#else
          if(++(plasmaBlinkOnTimer) >= plasmaOnTime) {
#endif
            // SJM 200506 add '='
            plasmaBlinkOnTimer = 0;
            plasmaBlinkOnFlag = TRUE;
            plasmaBlinkOffFlag = FALSE;
            plasmaBlinkOn = FALSE;
          }
        }
        else {
#ifndef HUNGARIAN_ISSUE
          if(++(plasmaBlinkOffTimer) >= PLASMA_BLINK_OFF_TIME) {
#else
          if(++(plasmaBlinkOffTimer) >= plasmaOffTime) {
#endif
            // SJM 200506 add '='
            plasmaBlinkOffTimer = 0;
            plasmaBlinkOnFlag = FALSE;
            plasmaBlinkOffFlag = TRUE;
            plasmaBlinkOn = TRUE;
          }
        }
      }   // end of (++plasmaInfo.plasmaOneSec > ONESEC)
    }     // end of (currentState == STATE_STER)
    if(segBlinkOnTime == TRUE) {
      if(++(plasmaInfo.blinkOnTimer) >= 750) {
        plasmaInfo.blinkOnTimer = 0;
        plasmaInfo.blinkOnFlag = TRUE;
        segBlinkOnTime = FALSE;
      }
    }
    else {
      if(++(plasmaInfo.blinkOffTimer) >= 250) {
        plasmaInfo.blinkOffTimer = 0;
        plasmaInfo.blinkOffFlag = TRUE;
        segBlinkOnTime = TRUE;
      }
    }
  }       // end of 'plasma mode Interrupt'
  
  //Disinfect mode interrupt
  if(currentState == STATE_DIS || currentState == STATE_DIS_STOP || currentState == STATE_READY_DIS) {
     
     //kwon: 2024-4-11, STATE_READY_DIS 는 단지 준비 상태이고 실제로 시간을 동작시키지는 않는다. 
     //실제동작 시간을 감소하는 것은 실제 동작이 될 때 한다. currentState == STATE_DIS 가 true 일 때 동작을 하게 된다.
     
    if(++disInfo.disOneSec >= ONESEC && currentState == STATE_DIS) {
      disInfo.disOneSec = 0;
      if(disInfo.disTimer > 0)
        disInfo.disTimer--;
    }
    if(segBlinkOnTime == TRUE) {
      if(++(disInfo.blinkOnTimer) >= 750) {
        disInfo.blinkOnTimer = 0;
        disInfo.blinkOnFlag = TRUE;
        segBlinkOnTime = FALSE;
      }
    }
    else {
      if(++(disInfo.blinkOffTimer) >= 250) {
        disInfo.blinkOffTimer = 0;
        disInfo.blinkOffFlag = TRUE;
        segBlinkOnTime = TRUE;
      }
    }
  }         // END OF 'Disinfect mode interrupt'
  
  //ion mode interrupt
  if(currentState == STATE_ION || currentState == STATE_ION_STOP) {
    if(++ionInfo.ionOneSec >= ONESEC && currentState == STATE_ION) {
      ionInfo.ionOneSec = 0;
      if(ionInfo.ionTimer > 0)
        ionInfo.ionTimer--;
    }
    if(segBlinkOnTime == TRUE) {
      if(++(ionInfo.blinkOnTimer) >= 750) {
        ionInfo.blinkOnTimer = 0;
        ionInfo.blinkOnFlag = TRUE;
        segBlinkOnTime = FALSE;
      }
    }
    else {
      if(++(ionInfo.blinkOffTimer) >= 250) {
        ionInfo.blinkOffTimer = 0;
        ionInfo.blinkOffFlag = TRUE;
        segBlinkOnTime = TRUE;
      }
    }
  }     // end of "ion mode interrupt"
  
  if(voicePlayFlag == TRUE) {
    if(--g_voicePortDelay > 1) {
      GPIOA->BSRRH = GPIO_Pin_14;
    }
    else {
      GPIOA->BSRRL = GPIO_Pin_14; 
      voicePlayFlag = FALSE;
    }
  }
}

extern unsigned short m_count;
void TIM3_IRQHandler(void)
{
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
//  if (stopFlag==0)    // SJM 200819
  remoteCheck();
#if 0
  if (stopFlag) {
    if(g_remoteFlag & (REMOTE_POWER_LONG_FLAG | REMOTE_POWER_FLAG)) {
      stopFlag = 0;
      g_remoteFlag = 0;
//      currentState = STATE_READY_STER;
//      isFirstEntrance = TRUE;
      NVIC_SystemReset();
    }
  }
#endif
}

extern unsigned char rx_buffer[];
extern unsigned char rx_head;
extern unsigned char rx_tail;
extern unsigned int setRTCFlag;

void USART3_IRQHandler(void)
{
  if(USART_GetITStatus(USART3, USART_IT_RXNE)!=RESET) { // && setRTCFlag == FALSE) { 
    rx_buffer[rx_head] = USART_ReceiveData(USART3);
    //printf("receive : %c\r\n", rx_buffer[rx_head]);
    if(++rx_head>=MAX_RX_BUFFER) 
      rx_head = 0;   
     USART_ClearITPendingBit(USART3,USART_IT_RXNE);
   }
}

//void USART6_IRQHandler(void)
//{
//  printf("uart6 int");
//  if(USART_GetITStatus(USART6, USART_IT_RXNE)!=RESET) { // && setRTCFlag == FALSE) { 
//    rx_buffer[rx_head] = USART_ReceiveData(USART6);
//    printf("receive : %c\r\n", rx_buffer[rx_head]);
//    if(++rx_head>=MAX_RX_BUFFER) 
//      rx_head = 0;   
//     USART_ClearITPendingBit(USART6,USART_IT_RXNE);
//   }
//}


volatile int string_started = 0;

struct SwitchInfo switchInfo ={"",""};

void parseSwitchInfo(const char *input, struct SwitchInfo *info) {
    // 
    char *trimmedInput = strdup(input + 1); // 

    //
    char *endBracket = strrchr(trimmedInput, '}');
    
    printf("endBracket = %s \r\n" ,  endBracket);
    
    if (endBracket != NULL)
        *endBracket = '\0'; // 

    

    // 
     char *token = strtok(trimmedInput, ":");
     printf("token = %s \r\n", token );
     strcpy(info->switchType, token);
     
     token = strtok(NULL, ":");
     printf("token = %s \r\n", token );
     strcpy(info->switchState, token);
     

    free(trimmedInput); 
}

void process_received_data(char *data, int length) {

    printf("\r\nReceived string: %s\r\n", data);
    parseSwitchInfo(data, &switchInfo);
}

void USART6_IRQHandler(void) {
    if(USART_GetITStatus(USART6, USART_IT_RXNE) != RESET) {
        char received_char = USART_ReceiveData(USART6);
       
        if(received_char == '[') {
            //rx string 시작
            string_started = 1;
            rx_head = 0;
            rx_buffer[rx_head] = '\0';
        } else if(received_char == ']' && string_started) {
           //rx string 끝
            string_started = 0;            
            process_received_data((char *)rx_buffer, rx_head);
        } else if(string_started) {
            
            rx_buffer[rx_head++] = received_char;
            if(rx_head >= MAX_RX_BUFFER)
                rx_head = 0;
            rx_buffer[rx_head] = '\0';
        }

        USART_ClearITPendingBit(USART6, USART_IT_RXNE);
    }
}








#ifdef  INCLUDE_STOP_MODE
#if 0
void UART5_IRQHandler(void)
{
  unsigned char ch;
  
    if(USART_GetITStatus(UART5, USART_IT_RXNE)!=RESET) { // && setRTCFlag == FALSE) { 
      ch = USART_ReceiveData(UART5);
      printf("\r\n U5 = %x", ch);
  if (stopFlag) {
      NVIC_SystemReset();
  }
/*      rx_buffer[rx_head] = USART_ReceiveData(USART3);
      //printf("receive : %c\r\n", rx_buffer[rx_head]);
      if(++rx_head>=MAX_RX_BUFFER) 
        rx_head = 0;   
*/
      USART_ClearITPendingBit(UART5,USART_IT_RXNE);
   }
}
#endif
void EXTI2_IRQHandler(void)
{
//  unsigned char ch;
  
  EXTI_ClearITPendingBit(EXTI_Line2);
//  if (intCounter==0) {
//    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
//    prevSec = RTC_TimeStructure.RTC_Seconds;
//  }
  intCounter++;
  printf("Here %d\r\n",intCounter);
//  if ((g_remoteFlag == REMOTE_POWER_FLAG)||(g_remoteFlag == REMOTE_POWER_FLAG)) {
//    printf("POWER caught\r\n");
//  }
  if (intCounter>10) {
//    intCounter = 0;
//    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
//    curSec = RTC_TimeStructure.RTC_Seconds;
//    if ((curSec-prevSec)%60<2)
      resetFlag = TRUE;
//    else {
//      resetFlag = FALSE;
//    }
    printf("Here 3 : %d\r\n",resetFlag);
  }
//  if (stopFlag&&(intCounter>10)) {
  if ( stopFlag && resetFlag ) {
//      GPIO_WriteBit(GPIOA,GPIO_Pin_15,Bit_SET);
    printf("Here 2\r\n");
      NVIC_SystemReset();
  }
/*      rx_buffer[rx_head] = USART_ReceiveData(USART3);
      //printf("receive : %c\r\n", rx_buffer[rx_head]);
      if(++rx_head>=MAX_RX_BUFFER) 
        rx_head = 0;   
*/
}
#endif
/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
