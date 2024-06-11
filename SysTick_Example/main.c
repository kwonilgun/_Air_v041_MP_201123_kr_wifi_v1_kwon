/**
  * 0.5v - �ö�� ��忡�� ������ ������ ���� ��.(���� ��忡 �°�)
  *        ���� ���� �߰�
  * 0.6v - �ö�� ��忡�� ��ü���� ���� ����.
  *        ��ü ���� �߿��� ���� ���� �۵� ����.
  *        �ö�� ��忡�� ���� ���� ���� ����(0~3 -> 0~2)
  * 0.7v - ����� ��忡�� UV Lamp�� ���۽�Ŵ
  *        �ö�� ��忡�� �⺻ ���⸦ 1�� ����, ���� ����(0~2 -> 1~3)
  * 0.8v - �ö�� ��� ���� �� ���� ��ư�� ������ ���� ���Ⱑ ǥ�� ��.
  *        �Ҹ�ǰ ��ü Warningǥ�� �� Ȯ�� ��ư�� ������ Main���� �Ѿ� ��.
  */

/* Includes ------------------------------------------------------------------*/

//git 테스트를 해 본다.  다시바꾸어본다
#include "main.h"
#include "util.h"

/** @addtogroup STM32F4xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup SysTick_Example
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
unsigned int testLedBlink = FALSE;
unsigned char rx_buffer[MAX_RX_BUFFER];
unsigned char rx_head;
unsigned char rx_tail;
unsigned char setRTCFlag;
unsigned int m_remoteState;
extern unsigned char pBuf[];
extern unsigned int pPointer;
unsigned int oldPpointer;
unsigned char printfTest;
unsigned int segBlinkOnTime;

unsigned int currentState, isFirstEntrance;   // SJM 190711 never used , isReadyEntrance;
unsigned int destState;         // SJM 201121 added for consumable
unsigned char pidDetect;
unsigned int oneSecFlag;
extern unsigned char halfSecFlag;

unsigned int g_remoteFlag;



SYSTEMINFO sysConfig;
PLASMAINFO plasmaInfo;
DISINFO disInfo;
IONINFO ionInfo;

GPIO_InitTypeDef GPIO_InitStructure;
static __IO uint32_t TimingDelay;

void USART6_SendString(USART_TypeDef* USARTx, uint8_t* string);
  
/* Private function prototypes -----------------------------------------------*/
void Delay(__IO uint32_t nTime);

/* Private functions ---------------------------------------------------------*/
extern void GetUARTData();
extern void handler();

// kwon: 2024-4-15
extern void serial_handler(struct IotCommandSet *);


extern void changeState(unsigned char state, unsigned char write);
extern void voicePlay(unsigned int voice, unsigned int delay);
extern void voicePortInit();

#ifdef  INCLUDE_IWDG
unsigned char isWatchDog = FALSE;
void IWDG_Init();
#endif  
/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
#ifdef  INCLUDE_RETURN_TO_CONTINUOUS_MODE
extern unsigned char returnToContinuousOperation;
extern unsigned char prevOperation;
extern unsigned char continueOperation;
#endif

unsigned char  maxDispPower = 4;      // SJM 201117 used for HPA_36C only


unsigned char statusDIP1 = 1;            


//kwon: 2024-4-13 
struct IotCommandSet iotCommandSet = {"","","","","",""};


//void USART6_SendString(char *str){
//  while(*str){
//      while(USART_GetFlagStatus(USART6, USART6_FLAG_TXE) == RESET);
//      USART_SendData(USART6, *str);
//      str++;
//  }
//}

int counter = 0;

// ms ������ ������ �Լ�
void delay_ms(uint32_t ms) {
    volatile uint32_t i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 3195; j++);
}

// UART3�� ���� �����ϴ� �Լ�
void UART3_Write(char c) {
    while (!(USART3->SR & USART_SR_TXE)); // ���� ���۰� ����ִ��� Ȯ��
    USART3->DR = c; // ���� ����
}

void UART6_ReceiveData(uint8_t *data, uint16_t size) {
    
    printf("UART6_ReceiveData enter \r\n");
    for (uint16_t i = 0; i < size; i++) {
         while (!(USART6->SR & USART_SR_RXNE)); // Wait until data is received
        Delay(10);
        
        data[i] = USART6->DR; // Read received data
        printf("uart6 received %s\r\n",data);
    }

  
}

// ���� �������� �����͸� ����ϴ� �Լ�
void printBinary(uint8_t data) {
    for (int i = 7; i >= 0; i--) {
        if (data & (1 << i))
            UART3_Write('1');
        else
            UART3_Write('0');
    }
    
     UART3_Write('\n'); // �� �ٲ� ���� ����
}


//kwon: uart6 실제적으로 usart6로 데이터를 전송한다.
void USART6_SendString(USART_TypeDef* USARTx, uint8_t* string)
{
  
  printf("\r\n USART6_SendString tx= %s", string);
    while (*string)
    {
        // Wait until transmit data register is empty
     
      // printf("%c", *string);
      // printf("\r\n  send char hex = %x", *string);


        // Send a character to usart6
      USART_SendData(USARTx, *string & 0xff) ;
      Delay(5);

       while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET) {}
        string++;

        
      //  printf("\r\n data sent success\r\n");

        
    }
    printf("\r\n");
}

void USART6_Transmit(uint8_t data) {
    while (!(USART6->SR & USART_SR_TXE));
    USART6->DR = (data & (uint16_t)0x01FF);
}



//kwon: 2024-4-9 : uart6를 통해서 받은 코맨드를 처리
void serial_input_cmd_handler() {
  
  
  //swtichInfo  
  
   if(strlen(switchInfo.switchType) >0 ){
          
            printf("\r\n main.c switch type = %s", switchInfo.switchType);
            printf("\r\n main.c switch state = %s", switchInfo.switchState);


            if(strcmp(switchInfo.switchType, "wifi")  == 0 && strcmp(switchInfo.switchState, "on") == 0) {
               printf("\r\n wifi ready on from serial-6\r\n");
               
              strcpy(iotCommandSet.wifi, "on");
                      
            }
          
            else if(strcmp(switchInfo.switchType, "power")  == 0 && strcmp(switchInfo.switchState, "on") == 0) {
               printf("\r\n rx power on from serial-6\r\n");
               
              //  g_remoteFlag = TNY_POWER_FLAG;
              // iotCommandSet.power = 'on';
              strcpy(iotCommandSet.mode, "4");
                      
            }
            else if(strcmp(switchInfo.switchType, "power")  == 0 && strcmp(switchInfo.switchState, "off") == 0) {
               printf("\r\n rx power off from serial-6\r\n");
               
              //  g_remoteFlag = TNY_POWER_FLAG;
              // iotCommandSet.power = 'off';
              strcpy(iotCommandSet.mode, "5");
                      
            }

            else if(strcmp(switchInfo.switchType, "stop")  == 0 && strcmp(switchInfo.switchState, "on") == 0) {
               printf("\r\n rx running stop from serial-6\r\n");
               
              //  g_remoteFlag = TNY_POWER_FLAG;
              // iotCommandSet.power = 'off';
              strcpy(iotCommandSet.mode, "6");
                      
            }

            else if(strcmp(switchInfo.switchType, "status")  == 0 && strcmp(switchInfo.switchState, "status") == 0) {
               printf("\r\n rx status check call from serial-6\r\n");
               
              //  g_remoteFlag = TNY_POWER_FLAG;
              // iotCommandSet.power = 'off';
              strcpy(iotCommandSet.mode, "7");
                      
            }

            else if(strcmp(switchInfo.switchType, "status")  == 0 && strcmp(switchInfo.switchState, "sysinfo") == 0) {
               printf("\r\n rx read system info from serial-6\r\n");
               
              //  g_remoteFlag = TNY_POWER_FLAG;
              // iotCommandSet.power = 'off';
              strcpy(iotCommandSet.mode, "8");
                      
            }

            else if(strcmp(switchInfo.switchType, "setup")  == 0 && strcmp(switchInfo.switchState, "voice") == 0) {
               printf("\r\n rx setup voice 9 from serial-6\r\n");
               
              strcpy(iotCommandSet.mode, "9");
                      
            }

            else if(strcmp(switchInfo.switchType, "prepare")  == 0 && strcmp(switchInfo.switchState, "voice") == 0) {
               printf("\r\n rx prepare voice 10 from serial-6\r\n");
               
              strcpy(iotCommandSet.mode, "10");
                      
            }
                      
           
            else if(strcmp(switchInfo.switchType, "\"start\"")  == 0 ){
              printf("\r\n rx start: from serial-6\r\n");
              // printf("\r\n switchInfo.switchState= %s", switchInfo.switchState);

              // int action, duration, wind_speed;
              char action_str[10], duration_str[10], wind_speed_str[10]; // 변환된 문자열을 저장할 배열

              // 함수 호출
              processStartState(switchInfo.switchState, action_str, duration_str, wind_speed_str);

              strcpy(iotCommandSet.mode, action_str);
              strcpy(iotCommandSet.duration, duration_str);    
              strcpy(iotCommandSet.wind, wind_speed_str);   
               
            }
            
            strcpy(switchInfo.switchState, "");
            strcpy(switchInfo.switchType, "");
            
        }
}


int main(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable the GPIO_LED Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  if (SysTick_Config(SystemCoreClock / 1000)) { 
    /* Capture error */ 
    while (1);
  }

  TIM_Config();
  relayControlInit();   //relay port Init
                        // SJM 200820 Move to here because of MainPowerControl
  remoteTimerInit();
  remoteInit();
  Uart_init();
  
  //kwon ozs port uart6 init
  Uart6_init();
  

  pirInit();
  sEE_Init();    //24LC512 Init

  keyPortInit();

  ledPortInit();
  ISD1760_init();
  voicePortInit(); //voice Port init

  ozoneSensorInit();    // SJM 200820 ADC is used for Vbat in HPA_36C
  ADC_SoftwareStartConv(ADC1);
  
  GPIOB->BSRRL = GPIO_Pin_8;
  Delay(10);
  
  rtc_check();
  rtc_init();
  
  printfTest = FALSE;

  printf("\r\n  Wall Type  : HPA-130W\r\n");

  printf("\r\n  kwonilgun :version: 2024-4-30-test-wa \r\n");

  printf("\r\n   Ver %d (Language:%d)\r\n", SW_VERSION,LANGUAGE);
  printf("\r\n  HealthWell Medical Inc. 0401-01 \r\n");
  
  
   // Send "Hello, World!" through USART6
 
   

 
  /* Check if the system has resumed from WWDG reset */
  if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET) { 
    /* WWDGRST flag set */
    /* Turn on LED1 */
    isWatchDog = TRUE;
    //STM_EVAL_LEDOn(LED1);
    printf("\r\n IWDG Reset occured!\r\n");
    /* Clear reset flags */
    RCC_ClearFlag();
  }
  else{
    isWatchDog = FALSE;
  }
  IWDG_Init();


  changeState(STATE_POWER_OFF, FALSE);  // SJM 201120 change to changeState()
  //system loading
  systemRead();
 
  
  
  
  while (1) {
    
    IWDG_ReloadCounter();
    
    //uart3 입력을 처리한다. shell.c에서 입력된 데이타를 사용한다.
    GetUARTData();

    handler();

    if (g_keyFlag) {
      if ((g_keyFlag&KEY_UPGRADE)==KEY_UPGRADE) {
        voicePlay(SWITCH_KEY, DELAY_KEY);
        Delay(DELAY_KEY);
        UpgradeEEPWrite(UpgradeEEPdata);
        NVIC_SystemReset();
        g_keyFlag = 0;      // SJM 200427 clear flag only when expected key.
                            // Fortunately, here is the only place to check key....
      }
    }
    if(testLedBlink == TRUE) {
      testLedBlink = FALSE;
      GPIOB->ODR ^= GPIO_Pin_8;
      //GPIOA->ODR ^= GPIO_Pin_14;
    }

    //kwon :2024-4-9 ,uart6로 들어온 명령을 처리 
    serial_input_cmd_handler();

    
    
    
    // kwon : 2024-4-15, uart6 command hander
    // 2024-5-5: wifi ready on 추가 
    // 2024-5-29: comm error fix, change iotCommandSet.mode, 명령체계를 변경했다. 가능한 간단하게 체계를 변경함. 통신 에러가 발생해서 보강
    if(strcmp(iotCommandSet.wifi, "on") == 0 && strlen(iotCommandSet.mode) > 0){

      printf("\r\n\r\n<<......serial command start ....>>");
      serial_handler(&iotCommandSet);
      
      //초기화
      strcpy(iotCommandSet.start, "");
      strcpy(iotCommandSet.power, "");
      strcpy(iotCommandSet.mode, "");
      strcpy(iotCommandSet.wind, "");
      strcpy(iotCommandSet.duration, "");
    }
    
    //kwon: 2024-5-6, wifi가 on이 아니면 확인을 체크한다. 
    // 부팅 시에 8266 모듈이 wifi가 연결이 되었는지 확인을 한다.
    if(halfSecFlag == TRUE) {
        if(!(strcmp(iotCommandSet.wifi, "on") == 0) ) {
              uint8_t str[] = "[hello 8266]";  
              USART6_SendString(USART6, (uint8_t *) str);
            }
    }


    if(currentState == STATE_POWER_OFF || currentState == STATE_READY_STER || currentState == STATE_STER_STOP ||
       currentState == STATE_READY_ION || currentState == STATE_ION_STOP || 
       currentState == STATE_READY_DIS || currentState == STATE_DIS_STOP ||
       (((currentState==STATE_DIS)||(currentState==STATE_ION))&&(continueOperation))) {
  
      //1초마다 실행
      if(oneSecFlag == TRUE) {


          //kwon: 2024-5-6, wifi가 on이 아니면 확인을 체크한다. 
          // 부팅 시에 8266 모듈이 wifi가 연결이 되었는지 확인을 한다.
          // if(!(strcmp(iotCommandSet.wifi, "on") == 0) ) {
          //   uint8_t str[] = "[hello 8266]";  
          //   USART6_SendString(USART6, (uint8_t *) str);
          // }
          
          
        
          oneSecFlag = FALSE;
          RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
          (void)RTC->DR;
          if(plasmaInfo.rsvOn == TRUE) {
            if ((RTC_TimeStructure.RTC_Hours == plasmaInfo.rsvTime)&&
                (RTC_TimeStructure.RTC_Minutes == 0)&&(RTC_TimeStructure.RTC_Seconds == 0)) {

              switch (currentState) {
                case STATE_DIS       :  if (continueOperation) {
                                          returnToContinuousOperation = TRUE;
                                          prevOperation = STATE_DIS;
                                        }
                                        else {
                                          returnToContinuousOperation = FALSE;
                                          prevOperation = STATE_READY_DIS;
                                        }
                                        break;
                case STATE_ION       :  if (continueOperation) {
                                          returnToContinuousOperation = TRUE;
                                          prevOperation = STATE_ION;
                                        }
                                        else {
                                          returnToContinuousOperation = FALSE;
                                          prevOperation = STATE_READY_ION;
                                        }
                                        break;
                case STATE_INIT      :  // fall-through
                case STATE_POWER_OFF :  prevOperation = STATE_POWER_OFF;
                                        returnToContinuousOperation = FALSE;
                                        break;
                case STATE_READY_ION :  // fall-through
                case STATE_ION_STOP :   prevOperation = STATE_READY_ION;
                                        returnToContinuousOperation = FALSE;
                                        break;
                case STATE_READY_STER : // fall-through
                case STATE_STER_STOP :  prevOperation = STATE_READY_STER;
                                        returnToContinuousOperation = FALSE;
                                        break;
                case STATE_READY_DIS :  // fall-through
                case STATE_DIS_STOP :   
                default :               prevOperation = STATE_READY_DIS;
                                        returnToContinuousOperation = FALSE;
                                        break;
              }

   
              changeState(STATE_STER,TRUE);    // SJM 201113 FALSE-->TRUE
                                               // to detect EEPROM Error
              plasmaInfo.isScheduled = TRUE;
            }
          }
      }
    }

    if(pPointer != oldPpointer) {
      printf("%c", pBuf[oldPpointer]);
      if(++oldPpointer > MAX_SIZE_PRINT)
        oldPpointer = 0;
    }
  }
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}



