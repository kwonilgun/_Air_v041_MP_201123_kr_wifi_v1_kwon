#include "Miscellaneous.h"

/* Exported macro ------------------------------------------------------------*/
#define VOICE_PLAY(track)                   \
        do                                  \
        {                                   \
            ISD1760_setPlay(                \
                ##track##_START,            \
                ##track##_END,              \
                g_volume);                        \
        } while(0);                         \
//                15);                        \


#define VOICE_OFF()                         \
        do                                  \
        {                                   \
        } while(0);

void voicePortInit()
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable the GPIO_LED Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
  /* Configure the Relay pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void voicePlay(unsigned int voice, unsigned int delay)
{
  Delay(50);
  
  g_voicePortDelay = delay;
  printf("\r\n Miscellanous.c g_voicePortDelay : %d\r\n", g_voicePortDelay);
  Delay(10);
  voicePlayFlag = TRUE;
  
  switch(voice)
  {
  case SWITCH_POWER_ON:
    VOICE_PLAY(VOICE_POWERON);
    break;
  case SWITCH_STERILIZATION_START:
    VOICE_PLAY(VOICE_STERILIZATION_START);
    break;
  case SWITCH_STERILIZATION_STOP:
    VOICE_PLAY(VOICE_STERILIZATION_STOP);
    break;
  case SWITCH_PREPARE:
    VOICE_PLAY(VOICE_PREPARE);
    break;
  case SWITCH_ION_START:
    VOICE_PLAY(VOICE_ION_START);
    break;
  case SWITCH_PLASMA_START:
    VOICE_PLAY(VOICE_PLASMA_START);
    break;
  case SWITCH_PIR_DETECT:
    VOICE_PLAY(VOICE_PIR_DETECT);
    break;
  case SWITCH_POWER_OFF:
    VOICE_PLAY(VOICE_POWER_OFF);
    break;
  case SWITCH_KEY:
    VOICE_PLAY(VOICE_KEY);
    break;
  case SWITCH_ION_STOP:
    VOICE_PLAY(VOICE_ION_STOP);
    break;
  case SWITCH_PLASMA_STOP:
    VOICE_PLAY(VOICE_PLASMA_STOP);
    break;
  case SWITCH_DESTRUCTION_START:
    VOICE_PLAY(VOICE_DESTRUCTION_START);
    break;
  case SWITCH_DESTRUCTION_STOP:
    VOICE_PLAY(VOICE_DESTRUCTION_STOP);
    break;
  case SWITCH_OZONE_DETECT:
    VOICE_PLAY(VOICE_OZONE_DETECT);
    break;
  case SWITCH_PLASMA_MODE:
    VOICE_PLAY(VOICE_PLASMA_MODE);
    break;
  case SWITCH_ANION_MODE:
    VOICE_PLAY(VOICE_ANION_MODE);
    break;
  case SWITCH_DISINFECT_MODE:
    VOICE_PLAY(VOICE_DISINFECT_MODE);
    break;
  case SWITCH_SETUP_MODE:
    VOICE_PLAY(VOICE_SETUP_MODE);
    break;
  case SWITCH_ENGINEER_MODE:
    VOICE_PLAY(VOICE_ENGINEER_MODE);
    break;
  case SWITCH_CONSUMABLE_CHECK:
    VOICE_PLAY(VOICE_CONSUMABLE_CHECK);
    break;
  }
}

#ifdef  SJM_DEBUG     // SJM 190617 for test
void voicePlay2(unsigned int voice)
{
  Delay(50);
    
  switch(voice)
  {
  case SWITCH_POWER_ON:
    g_voicePortDelay = DELAY_POWERON;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_POWERON);
    break;
  case SWITCH_STERILIZATION_START:
    g_voicePortDelay = DELAY_STER_START;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_STERILIZATION_START);
    break;
  case SWITCH_STERILIZATION_STOP:
    g_voicePortDelay = DELAY_STER_STOP;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_STERILIZATION_STOP);
    break;
  case SWITCH_PREPARE:
    g_voicePortDelay = DELAY_PREPARE;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_PREPARE);
    break;
  case SWITCH_ION_START:
    g_voicePortDelay = DELAY_ION_START;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_ION_START);
    break;
  case SWITCH_PLASMA_START:
    g_voicePortDelay = DELAY_PLASMA_START;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_PLASMA_START);
    break;
  case SWITCH_PIR_DETECT:
    g_voicePortDelay = DELAY_PIR_DETECT;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_PIR_DETECT);
    break;
  case SWITCH_POWER_OFF:
    g_voicePortDelay = DELAY_POWER_OFF;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_POWER_OFF);
    break;
  case SWITCH_KEY:
    g_voicePortDelay = DELAY_KEY;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_KEY);
    break;
  case SWITCH_ION_STOP:
    g_voicePortDelay = DELAY_ION_STOP;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_ION_STOP);
    break;
  case SWITCH_PLASMA_STOP:
    g_voicePortDelay = DELAY_PLASMA_STOP;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_PLASMA_STOP);
    break;
  case SWITCH_DESTRUCTION_START:
    g_voicePortDelay = DELAY_DESTRUCTION_START;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_DESTRUCTION_START);
    break;
  case SWITCH_DESTRUCTION_STOP:
    g_voicePortDelay = DELAY_DESTRUCTION_STOP;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_DESTRUCTION_STOP);
    break;
  case SWITCH_OZONE_DETECT:
    g_voicePortDelay = DELAY_OZONE_DETECT;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_OZONE_DETECT);
    break;
  case SWITCH_PLASMA_MODE:
    g_voicePortDelay = DELAY_PLASMA_MODE;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_PLASMA_MODE);
    break;
  case SWITCH_ANION_MODE:
    g_voicePortDelay = DELAY_ANION_MODE;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_ANION_MODE);
    break;
  case SWITCH_DISINFECT_MODE:
    g_voicePortDelay = DELAY_DISINFECT_MODE;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_DISINFECT_MODE);
    break;
  case SWITCH_SETUP_MODE:
    g_voicePortDelay = DELAY_SETUP_MODE;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_SETUP_MODE);
    break;
  case SWITCH_ENGINEER_MODE:
    g_voicePortDelay = DELAY_ENGINEER_MODE;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_ENGINEER_MODE);
    break;
  case SWITCH_CONSUMABLE_CHECK:
    g_voicePortDelay = DELAY_CONSUMABLE_CHECK;
    printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
    Delay(10);
    voicePlayFlag = TRUE;
    VOICE_PLAY(VOICE_CONSUMABLE_CHECK);
    break;
  default :
    printf("Un-recognized Voice No. = %d\r\n", voice);
    break;
  }
}

extern unsigned int g_RemoteOzoneSensorValue;
extern unsigned int g_remoteFlag;

void readRemote()
{
  printf("\r\n [g_remoteFlag = %04x]\r\n", g_remoteFlag);
  if( g_remoteFlag & (REMOTE_OZ1_FLAG | REMOTE_OZ2_FLAG | REMOTE_OZ3_FLAG | REMOTE_OZ4_FLAG | REMOTE_OZ5_FLAG) )
  {
    switch(g_remoteFlag)
    {
      case REMOTE_OZ1_FLAG:
        g_RemoteOzoneSensorValue = 0; // SJM 190808 5->0;
        break;
      case REMOTE_OZ2_FLAG:
        g_RemoteOzoneSensorValue = 15;          
        break;
      case REMOTE_OZ3_FLAG:
        g_RemoteOzoneSensorValue = 25;          
        break;
      case REMOTE_OZ4_FLAG:
        g_RemoteOzoneSensorValue = 35;          
        break;
      case REMOTE_OZ5_FLAG:
        g_RemoteOzoneSensorValue = 45;          
        break;
      default:
        break;          
    }
    g_remoteFlag = 0;
  }
  else if( g_remoteFlag & (REMOTE_OZ6_FLAG | REMOTE_OZ7_FLAG | REMOTE_OZ8_FLAG) )
  {
    switch(g_remoteFlag)
    {
      case REMOTE_OZ6_FLAG:
        g_RemoteOzoneSensorValue =  55;          
        break;
      case REMOTE_OZ7_FLAG:
        g_RemoteOzoneSensorValue = 65;
        break;
      case REMOTE_OZ8_FLAG:
        g_RemoteOzoneSensorValue = 75;
        break;

      default:
        break;
    }
    g_remoteFlag = 0;
  }  
}  
#endif  // SJM_DEBUG
