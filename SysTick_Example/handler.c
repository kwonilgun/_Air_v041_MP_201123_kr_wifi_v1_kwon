#include "handler.h"
#include "led.h"

#define ENG_RELAY_TEST          0
#define ENG_FACTORY_RESET       1
#define ENG_OZONE_SENSOR_TEST   2
#define ENG_INIT_CONSUME        3

extern void ledAllOn();
extern void reduceOzoneLevel();
extern void normalPlasmaSter();
extern void factoryReset();

//unsigned char exMin;    SJM 190711 it is assigned but never used anywhere...???
unsigned int segmentCycle=1;
unsigned int g_RemoteOzoneSensorValue;

//unsigned int  g_countPlamsaButtonPressed;   SJM 190711 it is never used anywhere...???
extern unsigned int g_extOzoneSenseFlag;
extern unsigned char halfSecFlag;

RTC_TimeTypeDef curTime;
unsigned int tempHour, tempMin, tempRsvTime, tempRsvFlag;
#ifdef  HUNGARIAN_ISSUE
  unsigned short tempDutyRatio;
  extern unsigned int plasmaOnTime;
  extern unsigned int plasmaOffTime;
#endif
#ifdef  INCLUDE_RESERVATION_OPERATION_CONTROL
  unsigned int tempRsvOperMin = DEF_RESERVE_OPER_MIN;
#endif
unsigned char ledStatus = 0;
unsigned char continueOperation = FALSE;    // SJM 190724 unify continue operation
unsigned short ozoneRangeOver = 0;

#ifdef  INCLUDE_RETURN_TO_CONTINUOUS_MODE
unsigned char returnToContinuousOperation=FALSE;
unsigned char prevOperation=DEFAULT_STATE;
#endif

extern unsigned char maxDispPower;      // SJM 201117 used for HPA_36C only



#ifdef  INCLUDE_STRENGTHEN_OZONE
  unsigned int setTime, operatedTime;
#endif
void segmentControlIntensity(unsigned char intensity);
#ifdef INCLUDE_IWDG
extern unsigned char isWatchDog;
#endif


void copyTime()
{
    RTC_TimeShow();
    RTC_GetTime(RTC_Format_BIN, &curTime);
    printf("  The current time (Hour-Minute-Second) is :  %0.2d:%0.2d:%0.2d \n\r", 
           curTime.RTC_Hours, curTime.RTC_Minutes, curTime.RTC_Seconds);
    tempHour = (unsigned char)curTime.RTC_Hours;
    tempMin = (unsigned char)curTime.RTC_Minutes;
    printf(" tempTime = %02d:%02d",tempHour,tempMin);
}

void changeState(unsigned char state, unsigned char write)
{
  sysConfig.prevState = currentState;

  if (state==STATE_STER) {
    if (plasmaInfo.pidOn == TRUE) {
      plasmaOnTime = PLASMA_BLINK_ON_TIME;
      plasmaOffTime = PLASMA_BLINK_OFF_TIME;
    }
    else {
      plasmaOnTime = (sysConfig.dutyRatio)*2;
      plasmaOffTime = 250-plasmaOnTime;
    }
    printf("\r\n[Plasma] OnTime = %d, OffTime = %d\r\n",plasmaOnTime,plasmaOffTime);
  }

  // SJM 191119 add safety transion when 'state' is abnormal. state가 범위를 벗어나면 에러 처리
  if ((state<STATE_READY_STER)||(state>LAST_STATE)) {
    printf("\r\n [Abnormal Call : curState = %d, destState = %d\r\n", currentState, state);
    currentState = DEFAULT_STATE;
  }
  else

  currentState = state;

  isFirstEntrance = TRUE;
  printf("\r\n>> switch to State %d\r\n", state);
  if (write) systemWrite();
}

unsigned int consumableCheck()
{
  unsigned int ref = 0;
//#ifndef HPA_36C  // SJM 200413 No UV-Lamp and Ozpne-Lamp in HPA_36C
#if ( MACHINE == HPA_130W )
  if(sysConfig.ozoneLampCountMin >= CHANGE_OZONE_LAMP)  ref |= WARNING_OZONE_LAMP;
  if(sysConfig.uvLampCountMin >= CHANGE_UV_LAMP)        ref |= WARNING_UV_LAMP;
#endif
  if(sysConfig.rciOperatingMin >= CHANGE_RCI_CELL)      ref |= WARNING_RCI_CELL;
  if(sysConfig.filterCountMin >= CHANGE_FILTER)         ref |= WARNING_FILTER;
  return ref;
}

void  checkStart()
{
//  static unsigned char alreadyChecked = FALSE;

  if((g_remoteFlag==REMOTE_OK_FLAG) || (g_remoteFlag==REMOTE_OK_LONG_FLAG)||
          (g_remoteFlag==TNY_OK_FLAG) || (g_remoteFlag==TNY_OK_LONG_FLAG)) {

    g_remoteFlag = 0;

    switch (currentState) {
      case STATE_READY_STER :
        if(plasmaInfo.pidOn == TRUE)  destState = STATE_PREPARE;
        else                          destState = STATE_STER;
        break;
      case STATE_READY_DIS :
        
        //kwon : 2024-4-11
        printf("\r\n kwon:checkStart():STATE_READY_DIS;");
        destState = STATE_DIS;
        
        break;
      case STATE_READY_ION :
        destState = STATE_ION;
        break;
//      case STATE_CONSUMABLE_WARNING :
//        changeState(destState, FALSE);
//        break;
      default :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        printf("\r\n[Error State!! currentState = %d]\r\n", currentState);
        break;
    }

    if (consumableCheck()) {     // SJM 200528 add
      changeState(STATE_CONSUMABLE_WARNING,FALSE);
    }
    else {
      changeState(destState, FALSE);
    }
  }

}

void checkPowerOff(unsigned int nextState)
{

  if ((g_remoteFlag==BUTTON_POWER)||(g_remoteFlag==BUTTON_POWER_LONG)||
      (g_remoteFlag==TNY_POWER_FLAG)||(g_remoteFlag==TNY_POWER_FLAG)) {

    g_remoteFlag = 0;

    if (nextState == STATE_POWER_OFF) {
      voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
      Delay(DELAY_POWER_OFF);
    }
    changeState(nextState,TRUE);
  }
}

void checkBaseMode(unsigned char skip)
{
#ifdef  USE_TNY_1311S_REMOTE
  //Ion Mode
  if ((g_remoteFlag==BUTTON_ANION) || (g_remoteFlag==BUTTON_ANION_LONG)) {
    g_remoteFlag = 0;
    changeState(STATE_READY_ION,FALSE);
  }
  #ifdef  HUNGARIAN_ISSUE
  if (skip) return;       // SJM 200422 different control for setup mode
                          //            without clearing "g_remoteFlag = 0"
  #endif
  //��� ���
  if ((g_remoteFlag==BUTTON_DISINFECT)||(g_remoteFlag==BUTTON_DISINFECT_LONG)) {
    g_remoteFlag = 0;
    changeState(STATE_READY_DIS,FALSE);
  }
  
  //�ö�� ���
  if ((g_remoteFlag==BUTTON_PLASMA) || (g_remoteFlag==BUTTON_PLASMA_LONG)) {
    g_remoteFlag = 0;
    changeState(STATE_READY_STER,FALSE);
  }
  if ((g_remoteFlag==TNY_MODE_FLAG)||(g_remoteFlag==TNY_MODE_LONG_FLAG)) {
    g_remoteFlag = 0;
    switch (currentState) {
      case STATE_ION_STOP :
      case STATE_READY_DIS : changeState(STATE_READY_ION,FALSE);
        break;
      case STATE_STER_STOP :
      case STATE_READY_ION : changeState(STATE_READY_STER,FALSE);
        break;
      case STATE_DIS_STOP :
      case STATE_READY_STER : changeState(STATE_READY_DIS,FALSE);
        break;
      default : changeState(STATE_READY_DIS,FALSE);
        break;
    }
  }
#else // USE_TNY_1311S_REMOTE

#endif  // USE_TNY_1311S_REMOTE
}

#ifdef USE_TNY_1311S_REMOTE   // New Handlers for TNY_1311S
#if ( MACHINE == HPA_130W )
void handleConcentration()
{
  static unsigned int tempOzoneLimit;

  if(isFirstEntrance == TRUE) {
    isFirstEntrance = FALSE;
    tempOzoneLimit = sysConfig.ozoneLimit;
    printf("\r\n [STATE_CONCENTRATION : %d ] Ozone Limit = %d",currentState,
           tempOzoneLimit);
  }
  if (halfSecFlag) {
    halfSecFlag = FALSE;
    segmentControl(tempOzoneLimit);
  }
  if (g_remoteFlag) {
    switch (g_remoteFlag) {
      case REMOTE_UP_FLAG :
      case REMOTE_UP_LONG_FLAG :
        tempOzoneLimit += 5;
        if (tempOzoneLimit>= OZONE_LIMIT_MAX) {
          tempOzoneLimit = OZONE_LIMIT_MAX;
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        }
        break;
      case REMOTE_DOWN_FLAG :
      case REMOTE_DOWN_LONG_FLAG :
        tempOzoneLimit -= 5;
        if (tempOzoneLimit<= OZONE_LIMIT_MIN) {
          tempOzoneLimit = OZONE_LIMIT_MIN;
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        }
        break;
      case TNY_OK_FLAG :
      case TNY_OK_LONG_FLAG :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        sysConfig.ozoneLimit = tempOzoneLimit;
        systemWrite();            // set Ozone Limit
        printf("\r\n [Ozone Limit = %d]\r\n",sysConfig.ozoneLimit);
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        break;
      case TNY_HOME_FLAG :
      case TNY_HOME_LONG_FLAG :
      case TNY_POWER_FLAG :
      case TNY_POWER_LONG_FLAG :
        changeState(STATE_POWER_OFF,FALSE);
        break;
    }
    g_remoteFlag = 0;
  }
}
#endif // #if ( MACHINE == HPA_130W )

void handleTimeSetup()
{
  static unsigned char hourMin;
  
  if(isFirstEntrance == TRUE) {
    isFirstEntrance = FALSE;
    printf("\r\n [STATE_TIME_SETUP : %d ]",currentState);
    hourMin = 0;    // 0 = hour mode, 1 = minute mode
    copyTime();
  }
  if (halfSecFlag) {
    halfSecFlag = FALSE;
        if (hourMin) {
          ledControl(LED_RESERVE_ON,LED_ON);
          segmentControl(tempMin);
        }
        else {
          ledControl(LED_RESERVE_ON,LED_OFF);
          segmentControl(tempHour);
        }
  }
  if (g_remoteFlag) {
    switch (g_remoteFlag) {
      case TNY_RIGHT_FLAG :
      case TNY_RIGHT_LONG_FLAG :
        if (hourMin) {
          tempMin++;  
          tempMin = tempMin%60;
        }
        else {
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
          hourMin = 1;
        }
        break;
      case TNY_LEFT_FLAG :
      case TNY_LEFT_LONG_FLAG :
        if (hourMin) {
          if (tempMin==0) tempMin = 59;
          else            tempMin--;  
        }
        else {
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
          hourMin = 1;
        }
        break;
      case REMOTE_UP_FLAG :
      case REMOTE_UP_LONG_FLAG :
        if (hourMin) {
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
          hourMin = 0;
        }
        else {
          tempHour++;
          tempHour = tempHour%24;
        }
        break;
      case REMOTE_DOWN_FLAG :
      case REMOTE_DOWN_LONG_FLAG :
        if (hourMin) {
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
          hourMin = 0;
        }
        else {
          if (tempHour==0) tempHour = 23;
          else            tempHour--;  
        }
        break;
      case TNY_OK_FLAG :
      case TNY_OK_LONG_FLAG :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
          RTC_TimeShow();
          printf(" tempTime = %02d:%02d",tempHour,tempMin);
//          if (hourMin)  curTime.RTC_Minutes = tempMin;
//          else          curTime.RTC_Hours = tempHour;
          curTime.RTC_Minutes = tempMin;
          curTime.RTC_Hours = tempHour;
          printf("\n\r  The current time (Hour-Minute-Second) is :  %0.2d:%0.2d:%0.2d ", 
               curTime.RTC_Hours, curTime.RTC_Minutes, curTime.RTC_Seconds);
          RTC_SetTime(RTC_Format_BIN,&curTime);
          RTC_TimeShow();
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        break;
      case TNY_HOME_FLAG :
      case TNY_HOME_LONG_FLAG :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        currentState = STATE_READY_ION;
        break;
      case TNY_POWER_FLAG :
      case TNY_POWER_LONG_FLAG :
        changeState(STATE_POWER_OFF,FALSE);
        break;
    }
    g_remoteFlag = 0;
  }
}

void handleEngineerSetup()
{
  static unsigned char port, onOff[MAX_RELAY_NUM];
  static unsigned int opMode;
  static unsigned char clearTarget;

  if(isFirstEntrance == TRUE) {
    isFirstEntrance = FALSE;
    printf("\r\n [STATE_ENG_SETUP : %d ]",currentState);
    opMode = ENG_RELAY_TEST;
    port = 1;       // port 1~8
    clearTarget = 1;
    voicePlay(SWITCH_ENGINEER_MODE, DELAY_ENGINEER_MODE);
    Delay(DELAY_ENGINEER_MODE);
  }
  if (halfSecFlag) {
    halfSecFlag = FALSE;
    switch (opMode) {
      case ENG_RELAY_TEST :
        segmentControl(port*10+onOff[port-1]);
        break;
      case ENG_FACTORY_RESET :
        segmentAlphaControl('F','r');
        break;
      case ENG_INIT_CONSUME :
        switch (clearTarget) {
          case 1 :  // RCI cell
            if (sysConfig.rciOperatingMin>0)
              segmentAlphaControl('A','1');
            else
              segmentAlphaControl('A','-');
            break;
          case 2 :  // Filter
            if (sysConfig.filterCountMin>0)
              segmentAlphaControl('F','1');
            else
              segmentAlphaControl('F','-');
            break;
#if ( MACHINE == HPA_130W )
          case 3 :  // OzoneLamp
            if (sysConfig.ozoneLampCountMin>0)
              segmentAlphaControl('O','1');
            else
              segmentAlphaControl('O','-');
            break;
          case 4 :  // UV Lamp
            if (sysConfig.uvLampCountMin>0)
              segmentAlphaControl('U','1');
            else
              segmentAlphaControl('U','-');
            break;
#endif
        }
        break;
#if ( MACHINE == HPA_130W )
      case ENG_OZONE_SENSOR_TEST :
        if(dOzoneSensoredValue < 99)
                segmentControl( (unsigned int)(dOzoneSensoredValue) );
        else  segmentControl(99);
        break;
#endif
    }
  }
  if (g_remoteFlag) {
    switch (g_remoteFlag) {
      case REMOTE_UP_FLAG :
      case REMOTE_UP_LONG_FLAG :
        switch (opMode) {
          case ENG_RELAY_TEST :
            if (++port>MAX_RELAY_NUM) port = 1;
            break;
          case ENG_INIT_CONSUME :
            if (++clearTarget>MAX_CONSUMABLES) clearTarget = 1;
            break;
        }
        break;
      case REMOTE_DOWN_FLAG :
      case REMOTE_DOWN_LONG_FLAG :
        switch (opMode) {
          case ENG_RELAY_TEST :
            if (--port<1) port = MAX_RELAY_NUM;
            break;
          case ENG_INIT_CONSUME :
            if (--clearTarget<1) clearTarget = MAX_CONSUMABLES;
            break;
        }
        break;
      case TNY_RIGHT_FLAG :
      case TNY_RIGHT_LONG_FLAG :
        switch (opMode) {
          case ENG_RELAY_TEST :
            onOff[port-1] ^= 1;
            relayControl(port,onOff[port-1]);
            break;
          case ENG_INIT_CONSUME :
            switch (clearTarget) {
              case 1 :  // RCI cell
                sysConfig.rciOperatingMin = 0;
                break;
              case 2 :  // Filter
                sysConfig.filterCountMin = 0;
                break;
#if ( MACHINE == HPA_130W )
              case 3 :  // OzoneLamp
                sysConfig.ozoneLampCountMin = 0;
                break;
              case 4 :  // UV Lamp
                sysConfig.uvLampCountMin = 0;
                break;
#endif
            }
            systemWrite();
            voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
            break;
        }
        break;
      case TNY_OK_LONG_FLAG :
        if (opMode == ENG_FACTORY_RESET) {
          voicePlay(SWITCH_CONSUMABLE_CHECK, DELAY_CONSUMABLE_CHECK); 
          Delay(DELAY_CONSUMABLE_CHECK);
          factoryReset();
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        }
        break;
      case TNY_HOME_LONG_FLAG :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        opMode = ENG_FACTORY_RESET;
        break;
      case TNY_SETUP_LONG_FLAG :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        opMode = ENG_RELAY_TEST;
        break;
      case TNY_TIMER_LONG_FLAG :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        opMode = ENG_INIT_CONSUME;
        break;
#if ( MACHINE == HPA_130W )
      case TNY_HUMAN_LONG_FLAG :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        opMode = ENG_OZONE_SENSOR_TEST;
        break;
#endif
      case TNY_POWER_FLAG :
      case TNY_POWER_LONG_FLAG :
        changeState(STATE_POWER_OFF,FALSE);
        break;
    }
    g_remoteFlag = 0;
  }
}

void handleBookTime()
{
  if(isFirstEntrance == TRUE) {
    isFirstEntrance = FALSE;
    printf("\r\n [STATE_BOOK_TIME : %d ]",currentState);
    tempRsvTime = plasmaInfo.rsvTime;
    tempRsvFlag = plasmaInfo.rsvOn;
    printf("\r\n[1]select rsvOn : %d\r\n", plasmaInfo.rsvOn);
    printf("[1]select rsvTime : %d\r\n", plasmaInfo.rsvTime);
  }
  if (halfSecFlag) {
    halfSecFlag = FALSE;
    ledStatus ^= 1;
    if (tempRsvFlag)  segmentControl(tempRsvTime);
    else              segmentAlphaControl('n','o');
//      ledControl(LED_RESERVE_ON, ledStatus);
  }
  if (g_remoteFlag) {
    switch (g_remoteFlag) {
      case TNY_RIGHT_FLAG :
      case TNY_RIGHT_LONG_FLAG :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        tempRsvFlag ^= 1;
        break;
      case REMOTE_UP_FLAG :
      case REMOTE_UP_LONG_FLAG :
        if (tempRsvFlag) {
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
          if (tempRsvFlag) tempRsvTime++;
          if (tempRsvTime>23) tempRsvTime = 0;
        }
        break;
      case REMOTE_DOWN_FLAG :
      case REMOTE_DOWN_LONG_FLAG :
        if (tempRsvFlag) {
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
          if (tempRsvTime==0) tempRsvTime = 23;
          else                tempRsvTime--;
        }
        break;
      case TNY_OK_FLAG :
      case TNY_OK_LONG_FLAG :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        plasmaInfo.rsvOn = tempRsvFlag;
        plasmaInfo.rsvTime = tempRsvTime;  
        printf("[2]select rsvOn : %d\r\n", plasmaInfo.rsvOn);
        printf("[2]select rsvTime : %d\r\n", plasmaInfo.rsvTime);
        systemWrite();              // Save Reservation Time
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        break;
      case TNY_HOME_FLAG :
      case TNY_HOME_LONG_FLAG :
//        g_remoteFlag = 0;
        currentState = STATE_READY_STER;
//        return;
        break;
      case TNY_POWER_FLAG :
      case TNY_POWER_LONG_FLAG :
//        g_remoteFlag = 0;
        changeState(STATE_POWER_OFF,FALSE);
//        return;
        break;
    }
    g_remoteFlag = 0;
  }
}

void handleBookConfig()
{
  static unsigned char configMode;
  
  if(isFirstEntrance == TRUE) {
    isFirstEntrance = FALSE;
    printf("\r\n [STATE_BOOK_CONFIG : %d ]",currentState);
    tempRsvOperMin = sysConfig.revOperMin;
#ifdef  HUNGARIAN_ISSUE
    tempDutyRatio = sysConfig.dutyRatio;
    printf("[cur Duty] =  %d\r\n", sysConfig.dutyRatio);
#endif
    configMode = SET_RESERVE_OPER_MIN;
    printf("\r\n 1 mode=%d, opMin=%d, dutyRatio=%d",configMode,
               tempRsvOperMin, tempDutyRatio);
  }
  if (halfSecFlag) {
    halfSecFlag = FALSE;
    switch (configMode) {
      case SET_DUTY_RATIO :
        segmentAlphaControl('d',tempDutyRatio+'0');
        break;
      case SET_RESERVE_OPER_MIN :
      default :
        segmentControl(tempRsvOperMin/10);
        break;
    }
  }
  if (g_remoteFlag) {
    switch (g_remoteFlag) {
      case REMOTE_UP_FLAG :
      case REMOTE_UP_LONG_FLAG :
        switch (configMode) {
          case SET_DUTY_RATIO :
            tempDutyRatio ++;
            if (tempDutyRatio>MAX_DUTY_RATIO) 
              tempDutyRatio = MIN_DUTY_RATIO;
            break;
          case SET_RESERVE_OPER_MIN :
          default :
            tempRsvOperMin += DEL_RESERVE_OPER_MIN;
            if (tempRsvOperMin>MAX_RESERVE_OPER_MIN) 
              tempRsvOperMin = MIN_RESERVE_OPER_MIN;
            break;
        }
        printf("\r\n 2 mode=%d, opMin=%d, dutyRatio=%d",configMode,
               tempRsvOperMin, tempDutyRatio);
        break;
      case REMOTE_DOWN_FLAG :
      case REMOTE_DOWN_LONG_FLAG :
        switch (configMode) {
          case SET_DUTY_RATIO :
            tempDutyRatio--;
            if (tempDutyRatio<MIN_DUTY_RATIO) 
              tempDutyRatio = MAX_DUTY_RATIO;
            break;
          case SET_RESERVE_OPER_MIN :
          default :
            tempRsvOperMin -= DEL_RESERVE_OPER_MIN;
            if (tempRsvOperMin<MIN_RESERVE_OPER_MIN) 
              tempRsvOperMin = MAX_RESERVE_OPER_MIN;
            break;
        }
        printf("\r\n 3 mode=%d, opMin=%d, dutyRatio=%d",configMode,
               tempRsvOperMin, tempDutyRatio);
        break;
      case TNY_OK_FLAG :
      case TNY_OK_LONG_FLAG :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        switch (configMode) {
          case SET_DUTY_RATIO :
            sysConfig.dutyRatio = tempDutyRatio;
            printf("[2]select dutyRatio : %d\r\n", sysConfig.dutyRatio);
            systemWrite();              // Save Duty Ratio
            break;
          case SET_RESERVE_OPER_MIN :
          default :
            sysConfig.revOperMin = tempRsvOperMin;
            printf("[2]select rsvOpMin : %d\r\n", sysConfig.revOperMin);
            systemWrite();              // Save Reservation Operating Minutes
            break;
        }
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        printf("\r\n 4 mode=%d, opMin=%d, dutyRatio=%d",configMode,
               tempRsvOperMin, tempDutyRatio);
        break;
      case TNY_TIMER_FLAG :
      case TNY_TIMER_LONG_FLAG :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        configMode = SET_RESERVE_OPER_MIN;
        printf("\r\n 5 mode=%d, opMin=%d, dutyRatio=%d",configMode,
               tempRsvOperMin, tempDutyRatio);
        break;
      case TNY_HUMAN_FLAG :
      case TNY_HUMAN_LONG_FLAG :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        configMode = SET_DUTY_RATIO;
        printf("\r\n 6 mode=%d, opMin=%d, dutyRatio=%d",configMode,
               tempRsvOperMin, tempDutyRatio);
        break;
      case TNY_HOME_FLAG :
      case TNY_HOME_LONG_FLAG :
        currentState = STATE_READY_STER;
        break;
      case TNY_POWER_FLAG :
      case TNY_POWER_LONG_FLAG :
        changeState(STATE_POWER_OFF,FALSE);
        break;
    }
    g_remoteFlag = 0;
  }
}
#endif  // USE_TNY_1311S_REMOTE

void handlePowerOff()
{
#ifdef  USE_TNY_1311S_REMOTE
  static unsigned char readyUpgrade, readySetup, engPW;
#endif
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_POWER_OFF : %d ]", currentState);
    
   //kwon : 2024-04-08
    printf("\r\n 1. handlePowerOff g_remoteFlag = %x", g_remoteFlag);
      
      
      
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug
    control_relayAllOff();
    ledAllOff();
    systemWrite();        // before Power-Off

    pidDetect = FALSE;

    readyUpgrade = readySetup = engPW = 0;

    if (isWatchDog)
      ledControl(LED_POWER_ON,LED_ON);

    isWatchDog = FALSE;


  }
  
  if (halfSecFlag) {
    halfSecFlag = FALSE;
    if (plasmaInfo.rsvOn) {
      ledStatus ^= 1;
      ledControl(LED_RESERVE_ON, ledStatus);
    }
  }
#ifdef  USE_TNY_1311S_REMOTE
  
  if (g_remoteFlag) {
  
    switch (g_remoteFlag) {
      case TNY_SETUP_LONG_FLAG :
        if (readyUpgrade) {
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
          UpgradeEEPWrite(UpgradeEEPdata);
          NVIC_SystemReset();
        }
        else {
          readyUpgrade = 1;
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
          segmentAlphaControl('U',' ');
        }
        readySetup = engPW = 0;
        break;
      case TNY_HUMAN_LONG_FLAG :
        if (readySetup) {
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
          changeState(STATE_CONCENTRATION,FALSE);
        }
        else {
          readySetup = 1;
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
          segmentAlphaControl('C','o');
        }
        readyUpgrade = engPW = 0;
        break;
      case TNY_RIGHT_LONG_FLAG :
        engPW = 1;
        readyUpgrade = readySetup = 0;
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        segmentAlphaControl('E',' ');
        break;
      case REMOTE_UP_LONG_FLAG :
        if (engPW==1) {
          engPW = 2;
          readyUpgrade = readySetup = 0;
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
          segmentAlphaControl('E','n');
        }
        else {
          engPW = readyUpgrade = readySetup = 0;
          segmentAlphaControl(' ',' ');
        }
        readyUpgrade = readySetup = 0;
        break;
      case TNY_OK_LONG_FLAG :
        if (engPW==2) {
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
          changeState(STATE_ENGINEER_SETUP,FALSE);
        }
        else 
          segmentAlphaControl(' ',' ');
        readyUpgrade = readySetup = engPW = 0;
        break;
      case REMOTE_POWER_FLAG :
      case REMOTE_POWER_LONG_FLAG :
      case TNY_POWER_FLAG :
      case TNY_POWER_LONG_FLAG :
        
        //kwon  : 2024-04-08
        printf("\r\nhandlerPowerOff TNY_POWER_LONG_FLAG");
        
        changeState(STATE_INIT,FALSE);

        break;
      default  : // clear Flags and 7-segment display
        segmentAlphaControl(' ',' ');
        readyUpgrade = readySetup = engPW = 0;
        break;
    }
    
    g_remoteFlag = 0;
    
     //kwon �߰� : 2024-04-08
    printf("\r\n 2. handlePowerOff g_remoteFlag = %x \r\n", g_remoteFlag);
    
    
  }
#else
  checkPowerOff(STATE_INIT);
#endif
}

void handleConsumableWarning()
{
  static unsigned int warningType[WARNING_NUMBER];
  static unsigned int warningNum;
  static unsigned int warning;
  static unsigned int warningCount;
  static unsigned char resetPreFlag;
  static unsigned char checkWarning;
  static unsigned char warnFilter,warnRCI;
#if ( MACHINE == HPA_130W )
  static unsigned char warnUV,warnOzone;
#endif
  
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_CONSUMABLE_WARNING : %d ]", currentState);
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug
    //LED control
    control_consumableCheckLed();
    //FND control
//    segmentControl(0);
    segmentAlphaControl('E','-');
    //warning check
    resetPreFlag = FALSE;
    checkWarning = TRUE;
    warnFilter = warnRCI = FALSE;
#if ( MACHINE == HPA_130W )
    warnUV = warnOzone = FALSE;
#endif
    voicePlay(SWITCH_CONSUMABLE_CHECK, DELAY_CONSUMABLE_CHECK);
    Delay(DELAY_CONSUMABLE_CHECK);
    printf("Consumable Warning mode\r\n");
  }
  
  //blink
  {
    if (checkWarning) {     // update Warning Table
      checkWarning = FALSE;
      warning = consumableCheck();
      if (warning==0) {
        changeState(STATE_INIT,FALSE);
//        changeState(destState,FALSE);     // SJM 201122 bugFix
        return;
      }
      for(int i = warningNum = 0 ; i < WARNING_NUMBER; i++) {
        printf("compare[%d] 0x%x & 0x%x\r\n", i, warning, (1 << i));
        if(warning & (1 << i)) {
//        warningType[i] = (1 << i);
          warningType[warningNum++] = (1 << i);
//        warningNum++;
          switch (i) {
            case 0 : warnOzone = TRUE;  break;
            case 1 : warnUV = TRUE;     break;
            case 2 : warnRCI = TRUE;    break;
            case 3 : warnFilter = TRUE; break;
          }
          printf("warning : 0x%x\r\n", (1 << i));
        }
      }
      warningCount = 0;
      printf("warningType[%d] : 0x%x\r\n", warningNum, warning);
    }
    if(sysConfig.blinkOffFlag == TRUE) {
      sysConfig.blinkOffFlag = FALSE;
//#ifndef HPA_36C  // SJM 200413 No UV-Lamp and Ozpne-Lamp in HPA_36C
#if ( MACHINE == HPA_130W)
      if(warningType[warningCount] & WARNING_OZONE_LAMP)    segmentAlphaControl('E','O');
      else if(warningType[warningCount] & WARNING_UV_LAMP)  segmentAlphaControl('E','U');
      else 
#endif
        if(warningType[warningCount] & WARNING_RCI_CELL) segmentAlphaControl('E','A');
      else if(warningType[warningCount] & WARNING_FILTER)   segmentAlphaControl('E','F');
      if(++warningCount >= warningNum)    warningCount = 0;
    }
    if(sysConfig.blinkOnFlag == TRUE) {
      segmentOff();
      sysConfig.blinkOnFlag = FALSE;
    }
  }
  
  //reset preKey
#ifdef  USE_TNY_1311S_REMOTE
  if ((g_remoteFlag==BUTTON_HUMAN_LONG)||(g_remoteFlag==TNY_SETUP_LONG_FLAG)) {
#else
//  if  (g_remoteFlag==REMOTE_UP_LONG_FLAG) {    // SJM 201115 FIX_REMOTE_KEY
  if (g_remoteFlag==BUTTON_HUMAN_LONG) {    // SJM 201121 change button
#endif
    g_remoteFlag = 0;
    voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
    resetPreFlag = TRUE;
    printf("preKey On!!\r\n");
  }
//#ifndef HPA_36C  // SJM 200413 No UV-Lamp and Ozpne-Lamp in HPA_36C
#if ( MACHINE == HPA_130W)
  //uvlamp reset
#ifdef  USE_TNY_1311S_REMOTE
  if ((g_remoteFlag==BUTTON_DISINFECT_LONG)||(g_remoteFlag==REMOTE_UP_LONG_FLAG)) {
#else
//  if (g_remoteFlag==REMOTE_WIND_LONG_FLAG) {  // SJM 201115 FIX_REMOTE_KEY
  if (g_remoteFlag==BUTTON_DISINFECT_LONG) {  // SJM 201121 change button
#endif
    g_remoteFlag = 0;
    if (resetPreFlag == TRUE) {
      voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
      if (warnUV) {
        resetPreFlag = warnUV = FALSE;
        sysConfig.uvLampCountMin = 0;
        systemWrite();            //uvlamp reset
        checkWarning = TRUE;
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        printf("reset UV Lamp!!\r\n");
      }
    }
  }
  //ozonelamp reset
#ifdef  USE_TNY_1311S_REMOTE
  if ((g_remoteFlag==BUTTON_PLASMA_LONG)||(g_remoteFlag==REMOTE_DOWN_LONG_FLAG)) {
#else
//  if (g_remoteFlag==REMOTE_ION_LONG_FLAG) {   // SJM 201115 FIX_REMOTE_KEY
  if (g_remoteFlag==BUTTON_PLASMA_LONG) {  // SJM 201121 change button
#endif
    g_remoteFlag = 0;
    if(resetPreFlag == TRUE) {
      voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
      if (warnOzone) {
        resetPreFlag = warnOzone = FALSE;
        sysConfig.ozoneLampCountMin = 0;
        systemWrite();            //ozonelamp reset
        checkWarning = TRUE;
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        printf("reset Ozone Lamp!!\r\n");
      }
    }
  }
#endif  // HPA_36C #if ( MACHINE == HPA_130W)
  //RCI cell reset
#ifdef  USE_TNY_1311S_REMOTE
  if ((g_remoteFlag==BUTTON_ANION_LONG)||(g_remoteFlag==TNY_RIGHT_LONG_FLAG)) {
#else
//  if (g_remoteFlag==REMOTE_PLASMA_LONG_FLAG) {  // SJM 201115 FIX_REMOTE_KEY
  if (g_remoteFlag==BUTTON_ANION_LONG) {  // SJM 201121 change button
#endif
    g_remoteFlag = 0;
    if(resetPreFlag == TRUE) {
      voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
      if (warnRCI) {
        resetPreFlag = warnRCI = FALSE;
        sysConfig.rciOperatingMin = 0;
        systemWrite();            //RCI cell reset
        checkWarning = TRUE;
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        printf("reset RCI Cell!!\r\n");
      }
    }
  }
  //filter reset
#ifdef  USE_TNY_1311S_REMOTE
  if ((g_remoteFlag==BUTTON_TIME_LONG)||(g_remoteFlag==TNY_LEFT_LONG_FLAG)) {
#else
//  if (g_remoteFlag==REMOTE_MENU_LONG_FLAG) {    // SJM 201115 FIX_REMOTE_KEY
  if (g_remoteFlag==BUTTON_TIME_LONG) {  // SJM 201121 change button
#endif
    g_remoteFlag = 0;
    if(resetPreFlag == TRUE) {
      voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
      if (warnFilter) {
        resetPreFlag = warnFilter = FALSE;
        sysConfig.filterCountMin = 0;
        systemWrite();            //filter reset
        checkWarning = TRUE;
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        printf("filter reset!!\r\n");
      }
    }
  }
//  checkBaseMode(0);
//  checkStart();                     // SJM 201120 replaced from checkBaseMode(0);
#ifdef USE_TNY_1311S_REMOTE
  if((g_remoteFlag==REMOTE_OK_FLAG) || (g_remoteFlag==REMOTE_OK_LONG_FLAG)||
          (g_remoteFlag==TNY_OK_FLAG) || (g_remoteFlag==TNY_OK_LONG_FLAG)) {
#else
  if(g_remoteFlag & (BUTTON_RIGHT | BUTTON_RIGHT_LONG)) {
#endif
    g_remoteFlag = 0;
    changeState(destState,FALSE);
  }
  checkPowerOff(STATE_POWER_OFF);   // SJM 190723 otherwise it can't be off!!!
}

void handleInit()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_INIT : %d ]", currentState);
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug
    //�ʱ� voice
    voicePlay(SWITCH_POWER_ON, DELAY_POWERON);
    //S/W ����
    segmentControl(SW_VERSION);
    //LED ����
    control_initLed();
    WhiteLed();
    //eeprom data loading.
    systemRead();//
    

    //changeState(STATE_MONEY_STANDBY,FALSE);

    pidDetect = FALSE;

    segmentAlphaControl('L',LANGUAGE+'0');
    Delay(700);
  

    switch (sysConfig.prevState) {
      case STATE_READY_STER :
      case STATE_STER :
      case STATE_PREPARE :
      case STATE_DESTRUCTION :
      case STATE_STER_STOP :  destState = STATE_READY_STER;
                              break;
        
      case STATE_READY_ION :
      case STATE_ION :
      case STATE_ION_STOP :   destState = STATE_READY_ION;
                              break;
      default :               destState = STATE_READY_DIS;
                              break;
    }

    //kwon: 2024-4-9
    printf("\r\nhandleInit sysConfig.prevState = %x \r\n", sysConfig.prevState);
    printf("\r\nhandleInit destState = %x \r\n", destState);
    
    
    if (consumableCheck()) {                // SJM 201120 added
      changeState(STATE_CONSUMABLE_WARNING,FALSE);
    }
    else changeState(destState,FALSE);
 
  }
}

void handleReadyPlasmaSter()
{
  static unsigned char editMode = 0;  // 0=Time, 1=Intensity

  static unsigned char pwrPIDon = 3;  // plasmaInfo.pwr when PID turns ON!
 
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_READY_STER : %d ]", currentState);
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug

    voicePlay(SWITCH_PLASMA_MODE, DELAY_PLASMA_MODE);
    Delay(DELAY_PLASMA_MODE);

    prevOperation = STATE_READY_STER;   // SJM 200506 debugged for normal(manual) operation
    //led Font on
    control_ledFont();
    ledControl(LED_PLASMA_ON, LED_ON);
    //relay all off
    control_relayAllOff();
    // Default setting & LED display
    editMode = 0;
    plasmaInfo.pidOn = TRUE;
    plasmaInfo.pwr = MAX_PLASMA_PWR;

    pwrPIDon = 3;

	
    maxDispPower = plasmaInfo.pwr + 2;


    plasmaInfo.plasmaTimer = 30 * 60;
  
  
    continueOperation = FALSE;
    ledControl(LED_PID_ON, plasmaInfo.pidOn);
    ledControl(LED_RESERVE_ON, plasmaInfo.rsvOn);

     //kwon:2024-4-10
    printf("\r\n 1-1. handleReadyPlasmaSter : plasmaInfo.plasmaTimer = %d",plasmaInfo.plasmaTimer );
    printf("\r\n 1-2. handleReadyPlasmaSter : plasmaInfo.plasmaTimer/600 = %d",plasmaInfo.plasmaTimer/600 );
    
    
    segmentControl(plasmaInfo.plasmaTimer/600);


    if(plasmaInfo.rsvOn == TRUE) {
//      printf("select Mode : %d\r\n", plasmaInfo.modeSelect);  SJM 190715 useless??
      printf("\r\nselect rsvOn : %d\r\n", plasmaInfo.rsvOn);
      printf("\r\nselect rsvTime : %d\r\n", plasmaInfo.rsvTime);
//      plasmaInfo.modeSelect = SETTING_NONE;                   SJM 190715 useless??
    }


  }

  if (halfSecFlag) {
    halfSecFlag = FALSE;
    ledStatus ^= 1;
    ledControl(LED_PID_ON, plasmaInfo.pidOn);
//    ledControl(LED_PID_FONT, plasmaInfo.pidOn);
    switch (editMode) {
      case 0 :  // Time Adjust
//#ifndef HPA_36C

        switch (plasmaInfo.pwr) {
          case 1 :
            ledControl(LED_POWER_FONT, LED_ON);
            ledControl(LED_POWER_ON, LED_OFF);
            break;
          case 2 :
            ledControl(LED_POWER_FONT, LED_OFF);
            ledControl(LED_POWER_ON, LED_ON);
            break;
          case 3 :
            ledControl(LED_POWER_FONT, LED_ON);
            ledControl(LED_POWER_ON, LED_ON);
            break;
        }
        ledControl(LED_RESERVE_FONT, ledStatus);
        
        

        if (continueOperation)  segmentAlphaControl('O','n');

        else {
          printf("\r\n 1-3. handleReadyPlasmaSter : plasmaInfo.plasmaTimer/600 = %d",plasmaInfo.plasmaTimer/600 );
          segmentControl(plasmaInfo.plasmaTimer/600);
          
        }
  
        break;
      case 1 :  // Intensity Adjust


        ledControl(LED_POWER_FONT, ledStatus);
        ledControl(LED_POWER_ON, ledStatus);
        ledControl(LED_RESERVE_FONT, LED_OFF);
        segmentControlIntensity(plasmaInfo.pwr);

        break;
    }
  }

  //Power setting

  if ((g_remoteFlag==BUTTON_INTENSITY)||(g_remoteFlag==BUTTON_INTENSITY_LONG)||
      (g_remoteFlag==TNY_RIGHT_FLAG)||(g_remoteFlag==TNY_LEFT_FLAG)) {

//    g_remoteFlag = 0;   SJM 201117 move to below

    printf("\r\n 3. handleReadyPlasmaSter : edit = %d", editMode );
    if (editMode==0) {
      editMode = 1;
    }
    else {
      switch (g_remoteFlag) {
        case BUTTON_INTENSITY : 
        case BUTTON_INTENSITY_LONG : 
        case TNY_RIGHT_FLAG : 
          if(++plasmaInfo.pwr > MAX_PLASMA_PWR)  plasmaInfo.pwr = 1;
          break;
        case TNY_LEFT_FLAG : 
          if(--plasmaInfo.pwr < 1)  plasmaInfo.pwr = MAX_PLASMA_PWR;
          break;
      }
      maxDispPower = plasmaInfo.pwr + 2;
    }
    g_remoteFlag = 0;   // SJM 201117 move to here
    voicePlay(SWITCH_KEY, DELAY_KEY);
//    segmentControl(plasmaInfo.pwr);
//    plasmaInfo.modeSelect = SETTING_POWER;      SJM 190715 useless??
    printf("power setting\r\n");
  }
  //Time setting
  if (g_remoteFlag==BUTTON_TIME) {
    g_remoteFlag = 0;

    editMode = 0;
    voicePlay(SWITCH_KEY, DELAY_KEY);
  }


  //PID setting
  
  if((g_remoteFlag==REMOTE_PLASMA_FLAG)||(g_remoteFlag==REMOTE_PLASMA_LONG_FLAG)||
     (g_remoteFlag==TNY_HUMAN_FLAG)||(g_remoteFlag==TNY_HUMAN_LONG_FLAG)) {
  
    g_remoteFlag = 0;
  
    voicePlay(SWITCH_KEY, DELAY_KEY);
    plasmaInfo.pidOn ^= 1;
    // SJM 200508 PID & continueOperation are exclusive.

    if (plasmaInfo.pidOn) continueOperation = FALSE;
  
    if (plasmaInfo.pidOn) {
      continueOperation = FALSE;
      plasmaInfo.pwr = pwrPIDon;
    }
    else {
      pwrPIDon = plasmaInfo.pwr;  // backup pwr for PID_ON
      plasmaInfo.pwr = 1;     // SJM 200922 default for power is 1 for PID_OFF
    }
  
    ledControl(LED_PID_ON,plasmaInfo.pidOn);
  } // End of PID setting
  
  //Time setting

  if ((g_remoteFlag==BUTTON_TIME_LONG)||(g_remoteFlag==REMOTE_UP_LONG_FLAG)) {

    g_remoteFlag = 0;
  
    editMode = 0;
    continueOperation = TRUE;
    // SJM 200508 PID & continueOperation are exclusive.
    plasmaInfo.pidOn = FALSE;
    
    pwrPIDon = plasmaInfo.pwr;  // backup pwr for PID_ON
    plasmaInfo.pwr = 1;     // SJM 200922 default for power is 1 for PID_OFF
    
    ledControl(LED_PID_ON,plasmaInfo.pidOn);
    voicePlay(SWITCH_KEY, DELAY_KEY);
  }

  
  //up Key
  if (g_remoteFlag==REMOTE_UP_FLAG) {

    g_remoteFlag = 0;
  
    if (editMode) {   // Intensity Adjust
  
      editMode = 0;
  
    }
    else {            // Time Adjust
      continueOperation = FALSE;

      if(plasmaInfo.plasmaTimer < MAX_PLASMA_OP_TIME * 60)
        plasmaInfo.plasmaTimer += DEL_PLASMA_OP_TIME * 60;

//    segmentControl(plasmaInfo.plasmaTimer / 60);
    }
    voicePlay(SWITCH_KEY, DELAY_KEY);
  }
  
  //down key
  if((g_remoteFlag==REMOTE_DOWN_FLAG)||(g_remoteFlag==REMOTE_DOWN_LONG_FLAG)) {
    g_remoteFlag = 0;
  
    if (editMode) {   // Intensity Adjust

      
      
      printf("\r\n 5. handleReadyPlasmaSter : REMOTE_DOWN_FLAG, eidtmode = 1" );
      
      editMode = 0;
      
  
    }
    else {            // Time Adjust
      continueOperation = FALSE;
      
      

      printf("\r\n 6-0. handleReadyPlasmaSter : REMOTE_DOWN_FLAG, eidtmode = 0" );
      if(plasmaInfo.plasmaTimer > MIN_PLASMA_OP_TIME * 60)
        plasmaInfo.plasmaTimer -= DEL_PLASMA_OP_TIME * 60;


    }
    voicePlay(SWITCH_KEY, DELAY_KEY);
  }

  if ((g_remoteFlag==TNY_TIMER_FLAG)||(g_remoteFlag==TNY_TIMER_LONG_FLAG)) {
    g_remoteFlag = 0;
    changeState(STATE_BOOK_TIME,FALSE);
  }
//  if ((g_remoteFlag==TNY_SETUP_FLAG)||(g_remoteFlag==TNY_SETUP_LONG_FLAG)) {
  if (g_remoteFlag==TNY_SETUP_LONG_FLAG) {
    g_remoteFlag = 0;
    changeState(STATE_BOOK_CONFIG,FALSE);
  }

  checkStart();
  checkBaseMode(0);
  checkPowerOff(STATE_POWER_OFF);
}

void handlePrepare()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_PREPARE : %d ]", currentState);
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug
    voicePlay(SWITCH_PLASMA_START, DELAY_PLASMA_START);
    Delay(DELAY_PLASMA_START);
    voicePlay(SWITCH_PREPARE, DELAY_PREPARE);
    Delay(DELAY_PREPARE);
    //segment Control
    prepareTimer = PREPARATION_TIME;    // SJM 190710 original was 30(sec);
    segmentControl(prepareTimer);
  }
  
  //blink
  {
    if(sysConfig.blinkOffFlag == TRUE)
    {
      ledControl(LED_PLASMA_ON, LED_ON);
      ledControl(LED_DIS_ON, LED_ON);
      ledControl(LED_ION_ON, LED_ON);
      
      segmentControl(prepareTimer);
      
      voicePlay(SWITCH_KEY, DELAY_KEY);
      
      sysConfig.blinkOffFlag = FALSE;
    }
    if(sysConfig.blinkOnFlag == TRUE)
    {
      ledControl(LED_PLASMA_ON, LED_OFF);
      ledControl(LED_DIS_ON, LED_OFF);
      ledControl(LED_ION_ON, LED_OFF);
      
      segmentOff();
      sysConfig.blinkOnFlag = FALSE;
    }
  }
  
  //time out -> plasma Mode Start
  if(prepareTimer <= 0) {
//    currentState = STATE_STER;
//    isFirstEntrance = TRUE;
    changeState(STATE_STER,FALSE);    // SJM 200924 change to changeState()
#ifdef INCLUDE_STOP_MODE
    setIdleTimer();
#endif
  }
  //stop key -> Plasma mode Need to be Fixed REMOTE_STOP_FLAG
#ifdef  USE_TNY_1311S_REMOTE
  if((g_remoteFlag==REMOTE_OK_LONG_FLAG)||(g_remoteFlag==REMOTE_OK_FLAG)||
     (g_remoteFlag==TNY_OK_LONG_FLAG)||(g_remoteFlag==TNY_OK_FLAG)) {
#else
  if((g_remoteFlag==REMOTE_OK_LONG_FLAG)||(g_remoteFlag==REMOTE_OK_FLAG)) {
//  if((g_remoteFlag==REMOTE_POWER_LONG_FLAG)||(g_remoteFlag==REMOTE_POWER_FLAG)) {
#endif
    g_remoteFlag = 0;
#ifdef INCLUDE_STOP_MODE
    idleTimeSec = 0;
#endif
    //voice -> ����� �ߴ� ����
//    currentState = STATE_READY_STER;
//    isFirstEntrance = TRUE;
    changeState(STATE_READY_STER,FALSE);  // SJM 201120 change to changeState()
  }     
  checkPowerOff(STATE_POWER_OFF);
}

void handlePlasmaSter()
{

  unsigned char remainedHour,remainedMinutes;

  
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_STER : %d ]", currentState);
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug
    control_relayAllOff();              // SJM 200506 added because of
                            // transition form continuous mode.
    segmentCycle = 1;
    pidDetect = FALSE;                  // SJM 190820 add initialize code
    plasmaBlinkOn = TRUE;
    plasmaBlinkOnTimer = 0;
    plasmaBlinkOffTimer = 0;

    ozoneRangeOver = 0;

    //voice play
    voicePlay(SWITCH_PLASMA_START, DELAY_PLASMA_START);
    Delay(DELAY_PLASMA_START);
    //relay control
    control_sterOn();
    //led control
    control_sterStartLedOn();
    //���� ������ ���
    if(plasmaInfo.isScheduled) {    // SJM 190724 'Scheduld' start

      continueOperation = FALSE;    // SJM 200507 
      plasmaInfo.pidOn = TRUE;
    
        plasmaInfo.plasmaTimer = sysConfig.revOperMin * 60;
//        plasmaInfo.pidOn = TRUE;
    

      plasmaInfo.isScheduled = FALSE;
      plasmaInfo.pwr = MAX_PLASMA_PWR;
      maxDispPower = plasmaInfo.pwr + 2;
    }

    setTime = plasmaInfo.plasmaTimer;
    printf("\r\n handlePlasmaSter: [Plasma] setTime = %d(%d)\r\n",
               setTime, plasmaInfo.plasmaTimer);
    
    if(plasmaInfo.pidOn == TRUE) {
      ledControl(LED_PID_ON, LED_ON);
    }
  }   // End of isFirstEntrance

  if (halfSecFlag) {
    halfSecFlag = FALSE;

    remainedHour = plasmaInfo.plasmaTimer/3600;
    remainedMinutes  = (plasmaInfo.plasmaTimer/60)%60;
  
    // Display Part
    switch (segmentCycle) {
      case 1 :  // SJM 190810 display remained time
        if(continueOperation) segmentAlphaControl('O','n');
        else                  segmentAlphaControl(remainedHour+'0','H');
        break;
      case 2 :  // SJM 190810 display remained time
        if(continueOperation) segmentAlphaControl('O','n');
        else                  segmentControl(remainedMinutes);
        break;


      case 3 :  // SJM 190810 display local sensor value
        if(dOzoneSensoredValue < 99)
          segmentControl( (unsigned int)(dOzoneSensoredValue) );
        else  segmentControl(99);
        break;

    }
    if (++segmentCycle>STER_INFO_DISP_CYCLE) segmentCycle = 1;
    ledStatus ^= 1;
    ledControl(LED_PLASMA_ON, ledStatus);
    // End of Display Part

    // Control Part
//#ifndef HPA_36C

  
    if (plasmaInfo.pidOn) {
     
      printf("\r\n kwon: plasmaInfo.pidOn = %c", plasmaInfo.pidOn);

      normalPlasmaSter();
    }
    else {
      // SJM 190809 check if ozone level exceeds the limit
      if( (g_RemoteOzoneSensorValue > OZONE_RISK_LEVEL) || (dOzoneSensoredValue > OZONE_RISK_LEVEL) ) {
        reduceOzoneLevel();         // SJM 190809 add to immediate action to reduce ozone level
        ozoneRangeOver++;
        printf("{OzoneTooHigh : Local = %d , Remote = %d, Over %d\r\n",dOzoneSensoredValue, g_RemoteOzoneSensorValue,ozoneRangeOver);
        if (ozoneRangeOver>=OZONE_OVER_LIMIT) {
          voicePlay(SWITCH_OZONE_DETECT, DELAY_OZONE_DETECT);
          Delay(DELAY_OZONE_DETECT);
  
          operatedTime = setTime - plasmaInfo.plasmaTimer;
          printf("\r\n ******** kwon: [Plasma] opTime = %d(%d) = %d - %d\r\n",
               operatedTime, operatedTime/60, setTime, plasmaInfo.plasmaTimer);
    
          changeState(STATE_DESTRUCTION,TRUE);
        }
      }
      // SJM 190809 or ozone level is in safe range ==> Normal Operation
      else if ((g_RemoteOzoneSensorValue<=OZONE_SAFE_LEVEL)&&(dOzoneSensoredValue<=OZONE_SAFE_LEVEL) ) {
        ozoneRangeOver = 0;
        normalPlasmaSter();
        //control_sterOn();
      }
      // SJM 190809 in this case we should reduce the ozone level ( in case of PIR sensor disabled )
      else { // if (plasmaInfo.pidOn == FALSE){
        if (ozoneRangeOver) ozoneRangeOver--;
        reduceOzoneLevel();
        printf("{OzoneControlMode : Local = %d , Remote = %d\r\n",dOzoneSensoredValue, g_RemoteOzoneSensorValue);
      }
    }
  


  }
 
  //time over
  if(plasmaInfo.plasmaTimer <= 0 && continueOperation == FALSE) {
    printf("\r\n kwon: timeover: plasmaInfo.plasmaTimer = %d", plasmaInfo.plasmaTimer);

    operatedTime = setTime;

    printf("\r\nkwon: time over: [Plasma] opTime = %d(%d) = %d - %d\r\n",
               operatedTime, operatedTime/60, setTime, plasmaInfo.plasmaTimer);

    changeState(STATE_DESTRUCTION,TRUE);
    printf("Time Over!!\r\n");
  }

  //PIR control
  if(plasmaInfo.pidOn == TRUE) {
    if(pidDetect == TRUE) {
      voicePlay(SWITCH_PIR_DETECT, DELAY_PIR_DETECT);
      Delay(DELAY_PIR_DETECT);
      pidDetect = FALSE;

      operatedTime = setTime - plasmaInfo.plasmaTimer;
        printf("\r\n kwon: PIR control: [Plasma] opTime = %d(%d) = %d - %d\r\n",
               operatedTime, operatedTime/60, setTime, plasmaInfo.plasmaTimer);

      changeState(STATE_DESTRUCTION,TRUE);
    }
  }
  //0.8v -> ���� ��ư�� ������ ���� ���� ���°� ǥ�õȴ�.
  // SJM 201117 Can't display current Intensity when used with TNY_1311S ==> No Problem
  if((g_remoteFlag==REMOTE_ION_FLAG)||(g_remoteFlag==REMOTE_ION_LONG_FLAG)) {
    g_remoteFlag = 0;

    segmentControlIntensity(plasmaInfo.pwr);
    Delay(800);

    if(continueOperation) segmentAlphaControl('O','n');
    else    segmentControl(plasmaInfo.plasmaTimer / 60);
  }
  //STOP

  if ((g_remoteFlag==REMOTE_OK_LONG_FLAG)||(g_remoteFlag==REMOTE_OK_FLAG)||
     (g_remoteFlag==TNY_OK_FLAG)||(g_remoteFlag==TNY_OK_LONG_FLAG)) {

    g_remoteFlag = 0;

    changeState(STATE_STER_STOP,TRUE);

    printf("go ster stop\r\n");
  }
  checkPowerOff(STATE_POWER_OFF);
}
  
void handlePlasmaSterStop()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_STER_STOP : %d ]", currentState);
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug
    control_relayAllOff();
    //LED and FND control
    control_sterStopLedOn();
    if (continueOperation)
      segmentAlphaControl('O','n');
    else
#ifndef REDUCE_TIME_TO_TEST
      segmentControl(plasmaInfo.plasmaTimer / 60);
#else
      segmentControl(plasmaInfo.plasmaTimer);
#endif
    systemWrite();                  // Plasma Ster Stop
    voicePlay(SWITCH_PLASMA_STOP, DELAY_PLASMA_STOP);
    Delay(DELAY_PLASMA_STOP);       // SJM 200421 added
    printf("Ster Stop!!\r\n");
  }
  
  //blink
  {
    if(plasmaInfo.blinkOffFlag == TRUE)
    {
      ledControl(LED_PLASMA_ON, LED_ON);
      plasmaInfo.blinkOffFlag = FALSE;
    }
    if(plasmaInfo.blinkOffFlag == TRUE)
    {
      ledControl(LED_PLASMA_ON, LED_OFF);
      plasmaInfo.blinkOnFlag = FALSE;
    }
  }
  
  //start
#ifdef  USE_TNY_1311S_REMOTE
  if ((g_remoteFlag==REMOTE_OK_LONG_FLAG)||(g_remoteFlag==REMOTE_OK_FLAG)||
     (g_remoteFlag==TNY_OK_FLAG)||(g_remoteFlag==TNY_OK_LONG_FLAG)) {
#else
  if((g_remoteFlag==REMOTE_OK_LONG_FLAG)||(g_remoteFlag==REMOTE_OK_FLAG)) {
#endif
    g_remoteFlag = 0;
#ifdef INCLUDE_STOP_MODE
    idleTimeSec = 0;
#endif
    voicePlay(SWITCH_KEY, DELAY_KEY);
    changeState(STATE_STER,FALSE);
  }
  checkBaseMode(0);
  checkPowerOff(STATE_POWER_OFF);
}

void handleDestruction()
{
#ifdef  OZONE_TIME_TEST
  unsigned char remainedHour,remainedMinutes;
#endif
  
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_DESTRUCTION : %d ]", currentState);
    isFirstEntrance = FALSE;  
    RTC_TimeShow();         // SJM 200506 for Debug

    control_relayAllOff();
    //voice Play(����� ���� ��)
    voicePlay(SWITCH_DESTRUCTION_START, DELAY_DESTRUCTION_START);
    Delay(DELAY_DESTRUCTION_START);
    //relay Control
    control_relayDestruction();

    //Led Control
    control_desLed();
    segmentCycle = 1;
    maxDispPower = 4;
//    ledAllOff();    
  
      destructionTimer = operatedTime/60;
      if (destructionTimer<30)      destructionTimer=30;
      else if (destructionTimer>120) destructionTimer=120;
      
      destructionTimer *= 60;

      printf("\r\n[Destruction] timer = %d (%d)\r\n",destructionTimer,operatedTime);
  }

  if (halfSecFlag) {
    halfSecFlag = FALSE;

    remainedHour = destructionTimer/3600;
    remainedMinutes  = (destructionTimer/60)%60;

    // Display Part
    switch (segmentCycle) {
      case 1 :  // SJM 190810 display remained time
        segmentAlphaControl(remainedHour+'0','H');
        break;
      case 2 :  // SJM 190810 display remained time
        segmentControl(remainedMinutes);
        break;

      case 3 :  // SJM 190810 display local sensor value
        if(dOzoneSensoredValue < 99)
          segmentControl( (unsigned int)(dOzoneSensoredValue) );
        else  segmentControl(99);
        break;

    }
    if (++segmentCycle>STER_INFO_DISP_CYCLE) segmentCycle = 1;
    ledStatus ^= 1;
    ledControl(LED_PLASMA_ON, ledStatus);
    ledControl(LED_DIS_ON, ledStatus);
    ledControl(LED_ION_ON, ledStatus);
  }
  
  //time over -> �ö�� Ready

  if(destructionTimer <= 0) {
    if (returnToContinuousOperation)
      continueOperation = TRUE;
    changeState(prevOperation,TRUE);
  }

  //��ž Ű -> �ö�� Ready
#ifdef  USE_TNY_1311S_REMOTE
  if ((g_remoteFlag==REMOTE_OK_LONG_FLAG)||(g_remoteFlag==REMOTE_OK_FLAG)||
     (g_remoteFlag==TNY_OK_FLAG)||(g_remoteFlag==TNY_OK_LONG_FLAG)) {
#else
  if((g_remoteFlag==REMOTE_OK_LONG_FLAG)||(g_remoteFlag==REMOTE_OK_FLAG)) {
#endif
    g_remoteFlag = 0;
    control_relayAllOff();
    //voice Play(����� �ߴ� ��)
    voicePlay(SWITCH_DESTRUCTION_STOP, DELAY_DESTRUCTION_STOP);
    Delay(DELAY_DESTRUCTION_STOP);
    changeState(STATE_READY_STER,TRUE);
  }
  //���� Ű -> Power Off
  checkPowerOff(STATE_POWER_OFF);
}

void handleReadyDisinfect()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_READY_DIS : %d ]", currentState);
    
    //kwon: 2024-4-9 
    printf("\r\n 1. handleReadyDisinfect: g_remoteFlag %x \r\n", g_remoteFlag);
    
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug
    //relay Control
    control_relayAllOff();
    //LED control
    control_disLed();
    //DIS FND
    continueOperation = FALSE;
    disInfo.disTimer = 60 * 60;
    segmentControl(disInfo.disTimer / 60);
    voicePlay(SWITCH_DISINFECT_MODE, DELAY_DISINFECT_MODE);
    Delay(DELAY_DISINFECT_MODE);
  }
  
  
  
  //시간 업
  if(g_remoteFlag==REMOTE_UP_FLAG) {  // SJM 201117 UP_LONG ==> "On"

    g_remoteFlag = 0;

    voicePlay(SWITCH_KEY, DELAY_KEY);
    if(disInfo.disTimer < 60 * 60)  disInfo.disTimer += 5 * 60;  
    continueOperation = FALSE;
    segmentControl(disInfo.disTimer / 60);
  }
  //시간 down
  if((g_remoteFlag==REMOTE_DOWN_FLAG)||(g_remoteFlag==REMOTE_DOWN_LONG_FLAG)) {
    g_remoteFlag = 0;
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    
    //kwon : 2024-4-9 
    printf("\r\n 2. handleReadyDisInfect - disInfo.disTimer = %d ", disInfo.disTimer); 
      
    if(disInfo.disTimer > 5 * 60) disInfo.disTimer -= 5 * 60;
    
    //kwon : 2024-4-9 
    printf("\r\n 3. handleReadyDisInfect - disInfo.disTimer = %d ", disInfo.disTimer); 
      
      
    continueOperation = FALSE;
    segmentControl(disInfo.disTimer / 60);
  }


  if ((g_remoteFlag==REMOTE_WIND_LONG_FLAG)||(g_remoteFlag==REMOTE_UP_LONG_FLAG)) {

    g_remoteFlag = 0;

    //continue
    continueOperation = TRUE;
    segmentAlphaControl('O', 'n');
  }
  //start
  checkStart();
  checkBaseMode(0);  
  checkPowerOff(STATE_POWER_OFF);
}


void handleSerialReadyDisinfect(int setTime){

  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_SERIAL_READY_DIS : %d ]", currentState);
    printf("\r\n [ set time : %d ]", setTime);
     
    isFirstEntrance = FALSE;
    
    control_relayAllOff();
    //LED control
    control_disLed();
    continueOperation = FALSE;
    disInfo.disTimer = setTime;
    segmentControl(disInfo.disTimer / 60);
    voicePlay(SWITCH_DISINFECT_MODE, DELAY_DISINFECT_MODE);
    Delay(DELAY_DISINFECT_MODE);
  }
  
  
  checkStart();
  checkBaseMode(0);
  checkPowerOff(STATE_POWER_OFF);


}

//��ո��
void handleDisinfect()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n handleDisinfect: [ STATE_DIS : %d ]", currentState);
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug
    control_relayAllOff();              // SJM 200506 added because of
                            // transition form continuous mode.
    voicePlay(SWITCH_STERILIZATION_START, DELAY_STER_START);
    Delay(DELAY_STER_START);
    //LED display
    control_disLed();
    maxDispPower = 4;
    //relay control
    control_disRelayOn();
  }
  
  //blink
  {
    if(disInfo.blinkOffFlag == TRUE)
    {
      ledControl(LED_DIS_ON, LED_ON);

      if (continueOperation) segmentAlphaControl('O', 'n');
      else          segmentControl(disInfo.disTimer / 60);
      disInfo.blinkOffFlag = FALSE;
    }
    if(disInfo.blinkOnFlag == TRUE)
    {
      ledControl(LED_DIS_ON, LED_OFF);

      segmentOff();
      
      disInfo.blinkOnFlag = FALSE;
    }
  }
  
  //Disinfect Stop

  if ((g_remoteFlag==REMOTE_OK_LONG_FLAG)||(g_remoteFlag==REMOTE_OK_FLAG)||
     (g_remoteFlag==TNY_OK_FLAG)||(g_remoteFlag==TNY_OK_LONG_FLAG)) {

    g_remoteFlag = 0;
    changeState(STATE_DIS_STOP,TRUE);
  }
  
  //time over
  if(disInfo.disTimer <= 0 && continueOperation == FALSE)
    changeState(STATE_READY_DIS,TRUE);


  checkPowerOff(STATE_POWER_OFF);
}

void handleDisinfectStop()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_DIS_STOP : %d ]", currentState);
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug

    //LED display
    control_disLed();
    //relay control
    control_relayAllOff();    // SJM 190711 instead of control_disRelayOff();
    systemWrite();            // Disinfection Stop
    //FND control
    if (continueOperation)  segmentAlphaControl('O', 'n');
    else            segmentControl(disInfo.disTimer / 60);

    //voice play
    voicePlay(SWITCH_STERILIZATION_STOP, DELAY_STER_STOP);
    Delay(DELAY_STER_STOP);

  }
  
  //blink
  {
    if(disInfo.blinkOffFlag == TRUE)
    {
      ledControl(LED_DIS_ON, LED_ON);
      
      disInfo.blinkOffFlag = FALSE;
    }
    if(disInfo.blinkOnFlag == TRUE)
    {
      ledControl(LED_DIS_ON, LED_OFF);
      
      disInfo.blinkOnFlag = FALSE;
    }
  }
  
  //restart
  if ((g_remoteFlag==REMOTE_OK_LONG_FLAG)||(g_remoteFlag==REMOTE_OK_FLAG)||
     (g_remoteFlag==TNY_OK_FLAG)||(g_remoteFlag==TNY_OK_LONG_FLAG)) {

    g_remoteFlag = 0;
//    isFirstEntrance = TRUE;
//    currentState = STATE_DIS;
    changeState(STATE_DIS,FALSE);  // SJM 201120 change to changeState()
  }
  checkBaseMode(0);
  checkPowerOff(STATE_POWER_OFF);
}

//���̿� ���
void handleReadyIon()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_READY_ION : %d ]", currentState);
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug
//#ifdef VOICE_FIRST
//    voicePlay(SWITCH_ANION_MODE, DELAY_ANION_MODE);
//    Delay(DELAY_ANION_MODE);
//#endif
    //LED control
    control_ionLed();
    maxDispPower = 4;
    //relay Control
    control_relayAllOff();
//    pidLEDOnFlag = FALSE;     SJM 190711 always assigned to 'FALSE' & never used
    //FND display
    ionInfo.ionTimer = 60 * 60;
    continueOperation = FALSE;
    segmentControl(ionInfo.ionTimer / 60);
//#ifndef VOICE_FIRST
    voicePlay(SWITCH_ANION_MODE, DELAY_ANION_MODE);
    Delay(DELAY_ANION_MODE);
//#endif
  }

  if (halfSecFlag) {
    halfSecFlag = FALSE;
    if (continueOperation)  segmentAlphaControl('O', 'n');
    else                    segmentControl(ionInfo.ionTimer / 60);
  }
  //time up
#ifdef  USE_TNY_1311S_REMOTE
  if(g_remoteFlag==REMOTE_UP_FLAG) {  // SJM 201117 UP_LONG ==> "On"
#else
  if((g_remoteFlag==REMOTE_UP_FLAG)||(g_remoteFlag==REMOTE_UP_LONG_FLAG)) {
#endif
    g_remoteFlag = 0;
#ifdef INCLUDE_STOP_MODE
    idleTimeSec = 0;
#endif
    voicePlay(SWITCH_KEY, DELAY_KEY);
    if(ionInfo.ionTimer < 95 * 60)  ionInfo.ionTimer += 5 * 60;  
    continueOperation = FALSE;
//    segmentControl(ionInfo.ionTimer / 60);  SJM 201118 move to display part
  }
  //time down
  if((g_remoteFlag==REMOTE_DOWN_FLAG)||(g_remoteFlag==REMOTE_DOWN_LONG_FLAG)) {
    g_remoteFlag = 0;
#ifdef INCLUDE_STOP_MODE
    idleTimeSec = 0;
#endif
    voicePlay(SWITCH_KEY, DELAY_KEY);
    if(ionInfo.ionTimer > 5 * 60) ionInfo.ionTimer -= 5 * 60;  
    continueOperation = FALSE;
//    segmentControl(ionInfo.ionTimer / 60);  SJM 201118 move to display part
  }
  //time continue
#ifdef  USE_TNY_1311S_REMOTE
  if ((g_remoteFlag==REMOTE_WIND_LONG_FLAG)||(g_remoteFlag==REMOTE_UP_LONG_FLAG)) {
#else
  if(g_remoteFlag==REMOTE_WIND_LONG_FLAG) {
#endif
    g_remoteFlag = 0;
#ifdef INCLUDE_STOP_MODE
    idleTimeSec = 0;
#endif
    printf("continue mode!!\r\n");
    continueOperation = TRUE;
//    segmentAlphaControl('O', 'n');          SJM 201118 move to display part
  }
  
  if(g_remoteFlag==BUTTON_CONFIRM_LONG) {
      g_remoteFlag = 0;
      changeState(STATE_SETUP_MODE,FALSE);
  }
#ifdef  USE_TNY_1311S_REMOTE
  if ((g_remoteFlag==TNY_TIMER_LONG_FLAG)||(g_remoteFlag==TNY_TIMER_LONG_FLAG)) {
      g_remoteFlag = 0;
      changeState(STATE_TIME_SETUP,FALSE);
  }
#endif
  checkStart();
  checkBaseMode(0);
  checkPowerOff(STATE_POWER_OFF);
}

void handleIon()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_ION : %d ]", currentState);
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug
    control_relayAllOff();              // SJM 200506 added because of
                            // transition form continuous mode.

    voicePlay(SWITCH_ION_START, DELAY_ION_START);
    Delay(DELAY_ION_START);
    //led control
    control_ionLed();
    //relay control
    control_IonOn();
    //FND control
    if (continueOperation)  segmentAlphaControl('O', 'n');
    else            segmentControl(ionInfo.ionTimer / 60);

  }
  
  //blink
  {
    if(ionInfo.blinkOffFlag == TRUE) {
      ledControl(LED_ION_ON, LED_ON);
      if (continueOperation)  segmentAlphaControl('O', 'n');
      else            segmentControl(ionInfo.ionTimer / 60);
      ionInfo.blinkOffFlag = FALSE;
    }
    if(ionInfo.blinkOnFlag == TRUE) {
      ledControl(LED_ION_ON, LED_OFF);
      segmentOff();
      ionInfo.blinkOnFlag = FALSE;
    }
  }
  
  //time Over
  if(ionInfo.ionTimer <= 0 && continueOperation == FALSE)
    // relay OFF???
    changeState(STATE_READY_ION,TRUE);
  //ion Stop
#ifdef  USE_TNY_1311S_REMOTE
  if ((g_remoteFlag==REMOTE_OK_LONG_FLAG)||(g_remoteFlag==REMOTE_OK_FLAG)||
     (g_remoteFlag==TNY_OK_FLAG)||(g_remoteFlag==TNY_OK_LONG_FLAG)) {
#else
  if((g_remoteFlag==REMOTE_OK_LONG_FLAG)||(g_remoteFlag==REMOTE_OK_FLAG)) {
#endif
    g_remoteFlag = 0;
//    voicePlay(SWITCH_KEY, DELAY_KEY);
    changeState(STATE_ION_STOP,TRUE);
  }
  checkPowerOff(STATE_POWER_OFF);
}

void handleIonStop()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_ION_STOP : %d ]", currentState);
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug

    //led control
    control_ionLed(); 
    //relay control
    control_relayAllOff();    // SJM 190711 instead of control_IonOff();
    systemWrite();            // Ion Stop
    //FND control
    if (continueOperation)  segmentAlphaControl('O', 'n');
    else            segmentControl(ionInfo.ionTimer / 60);
#ifdef VOICE_FIRST
    voicePlay(SWITCH_ION_STOP, DELAY_ION_STOP);
    Delay(DELAY_ION_STOP);
#endif    
  }

  //blink
  {
    if(ionInfo.blinkOffFlag == TRUE) {
      ledControl(LED_ION_ON, LED_ON);
      ionInfo.blinkOffFlag = FALSE;
    }
    if(ionInfo.blinkOnFlag == TRUE) {
      ledControl(LED_ION_ON, LED_OFF);
      ionInfo.blinkOnFlag = FALSE;
    }
  }
  
  //restart
#ifdef  USE_TNY_1311S_REMOTE
  if ((g_remoteFlag==REMOTE_OK_LONG_FLAG)||(g_remoteFlag==REMOTE_OK_FLAG)||
     (g_remoteFlag==TNY_OK_FLAG)||(g_remoteFlag==TNY_OK_LONG_FLAG)) {
#else
  if((g_remoteFlag==REMOTE_OK_LONG_FLAG)||(g_remoteFlag==REMOTE_OK_FLAG)) {
#endif
    g_remoteFlag = 0;
//    isFirstEntrance = TRUE;
//    currentState = STATE_ION;
    changeState(STATE_ION,FALSE);  // SJM 201120 change to changeState()
  }

  checkBaseMode(0);
  checkPowerOff(STATE_POWER_OFF);
}

void ledSequence(unsigned char step)
{
    if (step<(LED_PID_ON*4)) {
      ledControl(step/4+1,(step+1)%2);
    }
    else if (step< 80){
      if (step%2) ledAllOff();
      else        ledAllOn();
    }
    else if (step<90) {
      segmentControl((step%10)*11);
    }
    else {
      switch (step) {
        case 90 :
          segmentAlphaControl('E','O');
          break;
        case 91 :
          segmentAlphaControl('O','U');
          break;
        case 92 :
          segmentAlphaControl('U','F');
          break;
        case 93 :
          segmentAlphaControl('F','n');
          break;
        case 94 :
          segmentAlphaControl('n','-');
          break;
        case 95 :
          segmentAlphaControl('-','E');
          break;
        case 96 :
          segmentOff();
          break;
        default :
          break;
      }
    }
}

void handleEngineerMode()
{
  static unsigned char port, onOff[MAX_RELAY_NUM]; //, opMode;
  static unsigned char m_usbUpgrade;
  static unsigned int opMode;
//#ifdef  HPA_36C
#if ( MACHINE == HPA_36C)
  static unsigned int ledStep;
#elif ( MACHINE == HPA_130W )
  static unsigned int ozoneStep;
#endif
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_ENGINEER_MODE : %d ]", currentState);
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug
    //led control
//    control_engineerLed();
    ledAllOff();
    //segment control
    segmentControl(SW_VERSION);
    //relay all off
    control_relayAllOff();
//#ifdef HPA_36C
#if ( MACHINE == HPA_36C)
    ledStep = 0;
#elif ( MACHINE == HPA_130W)
    ozoneStep = 0;
#endif
    opMode = 0;   // 0-relay on/Off
    for (port=0; port<MAX_RELAY_NUM; port++) onOff[port] = 0;
    port = 0;
    m_usbUpgrade = FALSE;
    voicePlay(SWITCH_ENGINEER_MODE, DELAY_ENGINEER_MODE);
    Delay(DELAY_ENGINEER_MODE);
    printf("\r\n Engineer Mode\r\n");
  }
  
  switch (opMode) {
    case 0 :    // relay on/Off Test
      if ((g_remoteFlag==BUTTON_UP)||(g_remoteFlag==BUTTON_UP_LONG)) {
        g_remoteFlag = 0; m_usbUpgrade = FALSE;
        ledControl(port+1,LED_OFF);
        port++;
        if (port>=MAX_RELAY_NUM) port = 0;
        ledControl(port+1,LED_ON);
      }
      else if ((g_remoteFlag==BUTTON_DOWN)||(g_remoteFlag==BUTTON_DOWN_LONG)) {
        g_remoteFlag = 0; m_usbUpgrade = FALSE;
        ledControl(port+1,LED_OFF);
        if (port==0) port = MAX_RELAY_NUM-1;
        else          port--;
        ledControl(port+1,LED_ON);
      }
      else if ((g_remoteFlag==BUTTON_RIGHT)||(g_remoteFlag==BUTTON_RIGHT_LONG)) {
        g_remoteFlag = 0; m_usbUpgrade = FALSE;
        onOff[port] ^= 1;
        relayControl(port+1,onOff[port]);
      }
      segmentControl((port+1)*10+onOff[port]);
      break;
    case 1 :    // sequential led on/off test --> ozone limit 200511 upon JHK's request(Spain)
      if (halfSecFlag) {
        halfSecFlag = FALSE;
//  #ifndef HPA_36C
  #if ( MACHINE == HPA_130W)
      if ((g_remoteFlag==BUTTON_UP)||(g_remoteFlag==BUTTON_UP_LONG)) {
        g_remoteFlag = 0; m_usbUpgrade = FALSE;
        sysConfig.ozoneLimit += 5;
        if (sysConfig.ozoneLimit>= OZONE_LIMIT_MAX) {
          sysConfig.ozoneLimit = OZONE_LIMIT_MAX;
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        }
      }
      else if ((g_remoteFlag==BUTTON_DOWN)||(g_remoteFlag==BUTTON_DOWN_LONG)) {
        g_remoteFlag = 0; m_usbUpgrade = FALSE;
        sysConfig.ozoneLimit -= 5;
        if (sysConfig.ozoneLimit<= OZONE_LIMIT_MIN) {
          sysConfig.ozoneLimit = OZONE_LIMIT_MIN;
          voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        }
      }
      else if ((g_remoteFlag==BUTTON_CONFIRM)||(g_remoteFlag==BUTTON_CONFIRM_LONG)) {
        g_remoteFlag = 0; m_usbUpgrade = FALSE;
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        systemWrite();            // set Ozone Limit
        printf("\r\n [Ozone Limit = %d]\r\n",sysConfig.ozoneLimit);
      }
      segmentControl(sysConfig.ozoneLimit);
//  #else     // 200511 SJM change to ozone limit upon JHK's request(Spain)
  #elif ( MACHINE == HPA_36C)
        printf("\r\n {Here!!! opMode = %d, ledStep = %d\r\n", opMode, ledStep);
        ledSequence(ledStep);
        ledStep++;
        if (ledStep>=97) ledStep = 0;
  #endif // HPA_36C
      }
      break;
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
    case 2 :    // Ozone Sensor Display
      if (halfSecFlag) {
        halfSecFlag = FALSE;
      printf("[Eng] Local = %d , Remote = %d\r\n",dOzoneSensoredValue, g_RemoteOzoneSensorValue);
        // Disp Ozone Sensor Value
        if (ozoneStep) {
          ozoneStep = 0;
          if( g_extOzoneSenseFlag == TRUE) {
             g_extOzoneSenseFlag = FALSE;
             segmentControl(g_RemoteOzoneSensorValue);
          }
          else {
            segmentAlphaControl('-','-');
          }
        }
        else {
          ozoneStep++;
          if(dOzoneSensoredValue < 99)
//                segmentControl( (unsigned int)(dOzoneSensoredValue -1) );
                segmentControl( (unsigned int)(dOzoneSensoredValue) );
          else  segmentControl(99);
        }
      }
      break;
#endif
  }

  if (g_remoteFlag & (BUTTON_TIME | BUTTON_TIME_LONG)) {
    g_remoteFlag = 0; m_usbUpgrade = FALSE;
    opMode = 0;       // relay on/Off test
    printf("\r\n opMode1 = %d\r\n", opMode);
  }
  if ((g_remoteFlag==BUTTON_INTENSITY)||(g_remoteFlag==BUTTON_INTENSITY_LONG)) {
    g_remoteFlag = 0; m_usbUpgrade = FALSE;
    opMode = 1;       // LED test
    printf("\r\n opMode2 = %d\r\n", opMode);
  }
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  if ((g_remoteFlag==BUTTON_HUMAN)||(g_remoteFlag==BUTTON_HUMAN_LONG)) {
    g_remoteFlag = 0; m_usbUpgrade = FALSE;
    opMode = 2;       // Ozone Sensor test
    printf("\r\n opMode2 = %d\r\n", opMode);
  }
#endif
  // S/W upgrade!!!
  if (g_remoteFlag==BUTTON_ANION_LONG) {     // prepare to Upgrade
    g_remoteFlag = 0;
    m_usbUpgrade = TRUE;
    //voice play(KEY)
    voicePlay(SWITCH_KEY, DELAY_KEY);
  }
  if (g_remoteFlag==BUTTON_DISINFECT_LONG) {
    g_remoteFlag = 0;
    if (m_usbUpgrade == TRUE) {
      //voice play(KEY)
      voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
      voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
      UpgradeEEPWrite(UpgradeEEPdata);
      NVIC_SystemReset();
    }
  }
  if (g_remoteFlag==BUTTON_PLASMA_LONG) {
    g_remoteFlag = 0;
    if (m_usbUpgrade==TRUE) {
      m_usbUpgrade = FALSE;
      voicePlay(SWITCH_CONSUMABLE_CHECK, DELAY_CONSUMABLE_CHECK); 
      Delay(DELAY_CONSUMABLE_CHECK);
//#ifndef HPA_36C  // SJM 200413 No UV-Lamp and Ozpne-Lamp in HPA_36C
#if ( MACHINE == HPA_130W)
      sysConfig.uvLampCountMin = 0;
      sysConfig.ozoneLampCountMin = 0;
      // SJM 200511 added Initialization
      sysConfig.ozoneLimit = OZONE_LIMIT_DEFAULT;
#endif
      sysConfig.filterCountMin = 0;
      sysConfig.rciOperatingMin = 0;
      sysConfig.prevState = DEFAULT_STATE;
      plasmaInfo.rsvOn = FALSE;
      plasmaInfo.rsvTime = 1;
      plasmaInfo.pidOn = TRUE;
      plasmaInfo.pwr = MAX_PLASMA_PWR;
      plasmaInfo.isScheduled = FALSE;
      
      systemWrite();          // Factory Reset by Remote Controller
    }
  }
      
  checkPowerOff(STATE_POWER_OFF);
}

void handleSetup()
{
  static unsigned char hourMin, settingMode,engModeReady;
  
  if (isFirstEntrance) {
    printf("\r\n [ STATE_SETUP_MODE : %d ]", currentState);
    isFirstEntrance = FALSE;
    RTC_TimeShow();         // SJM 200506 for Debug
#ifdef VOICE_FIRST
    voicePlay(SWITCH_SETUP_MODE, DELAY_SETUP_MODE);
    Delay(DELAY_SETUP_MODE);
#endif
//    ledStatus = 0;
    engModeReady = FALSE;
    settingMode = SET_TIME;
    hourMin = 0;    // 0 = hour mode, 1 = minute mode
    copyTime();
    tempRsvTime = plasmaInfo.rsvTime;
    tempRsvFlag = plasmaInfo.rsvOn;
    printf("[1]select rsvOn : %d\r\n", plasmaInfo.rsvOn);
    printf("[1]select rsvTime : %d\r\n", plasmaInfo.rsvTime);
#ifdef  HUNGARIAN_ISSUE
    tempDutyRatio = sysConfig.dutyRatio;
    printf("[cur Duty] =  %d\r\n", sysConfig.dutyRatio);
#endif
    ledAllOff();
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
    ledControl(LED_POWER_SIG1,LED_ON);
    ledControl(LED_POWER_SIG2,LED_ON);
    ledControl(LED_SETTING_SIG1,LED_ON);
    ledControl(LED_SETTING_SIG2,LED_ON);
#endif
#ifndef VOICE_FIRST
    voicePlay(SWITCH_SETUP_MODE, DELAY_SETUP_MODE);
    Delay(DELAY_SETUP_MODE);
#endif
  }
  
  if (halfSecFlag) {
    halfSecFlag = FALSE;
    ledStatus ^= 1;
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
    switch (settingMode) {
      case SET_PLASMA_RESERVATION :
        ledControl(LED_RESERVE_FONT,LED_OFF);
        ledControl(LED_RESERVE_ON,LED_OFF);
        ledControl(LED_PLASMA_FONT, ledStatus);
        ledControl(LED_PLASMA_ON,tempRsvFlag);
        ledControl(LED_POWER_FONT, LED_OFF);
        ledControl(LED_POWER_ON, LED_OFF);
  #ifdef  HUNGARIAN_ISSUE
        ledControl(LED_PID_FONT, LED_OFF);
        ledControl(LED_PID_ON, LED_OFF);
  #endif
        if (tempRsvFlag)  segmentControl(tempRsvTime);
        else              segmentAlphaControl('n','o');
        break;
  #ifdef  INCLUDE_RESERVATION_OPERATION_CONTROL
      case SET_RESERVE_OPER_MIN :
        ledControl(LED_RESERVE_FONT,LED_OFF);
        ledControl(LED_RESERVE_ON,LED_OFF);
        ledControl(LED_PLASMA_FONT, ledStatus);
        ledControl(LED_PLASMA_ON,plasmaInfo.rsvOn);
        ledControl(LED_POWER_FONT, ledStatus);
        ledControl(LED_POWER_ON, LED_ON);
    #ifdef  HUNGARIAN_ISSUE
        ledControl(LED_PID_FONT, LED_OFF);
        ledControl(LED_PID_ON, LED_OFF);
    #endif
        segmentControl(tempRsvOperMin/10);
      break;
  #endif // INCLUDE_RESERVATION_OPERATION_CONTROL
  #ifdef  HUNGARIAN_ISSUE
      case SET_DUTY_RATIO :
        ledControl(LED_RESERVE_FONT,LED_OFF);
        ledControl(LED_RESERVE_ON,LED_OFF);
        ledControl(LED_PLASMA_FONT, ledStatus);
        ledControl(LED_PLASMA_ON,plasmaInfo.rsvOn);
        ledControl(LED_POWER_FONT, LED_OFF);
        ledControl(LED_POWER_ON, LED_OFF);
        ledControl(LED_PID_FONT, ledStatus);
        ledControl(LED_PID_ON, LED_ON);
        segmentAlphaControl('d',tempDutyRatio+'0');
      break;
  #endif // HUNGARIAN_ISSUE
      case SET_TIME :
      default :
        ledControl(LED_RESERVE_FONT,ledStatus);
        ledControl(LED_PLASMA_FONT, LED_OFF);
        ledControl(LED_PLASMA_ON,plasmaInfo.rsvOn);
        ledControl(LED_POWER_FONT, LED_OFF);
        ledControl(LED_POWER_ON, LED_OFF);
  #ifdef  HUNGARIAN_ISSUE
        ledControl(LED_PID_FONT, LED_OFF);
        ledControl(LED_PID_ON, LED_OFF);
  #endif
        if (hourMin) {
          ledControl(LED_RESERVE_ON,LED_ON);
          segmentControl(tempMin);
        }
        else {
          ledControl(LED_RESERVE_ON,LED_OFF);
          segmentControl(tempHour);
        }
        break;
    }
//#else	// HPA_36C
#elif ( MACHINE == HPA_36C)
  switch (settingMode) {
    case SET_PLASMA_RESERVATION :
      ledControl(LED_PLASMA_ON,tempRsvFlag);
      if (tempRsvFlag)  segmentControl(tempRsvTime);
      else              segmentAlphaControl('n','o');
      break;
	#ifdef  INCLUDE_RESERVATION_OPERATION_CONTROL
    case SET_RESERVE_OPER_MIN :
      ledControl(LED_PLASMA_ON,tempRsvFlag);
//      ledControl(LED_POWER_ON,LED_ON);
      segmentControl(tempRsvOperMin/10);
      break;
	#endif
    case SET_TIME :
    default :
      if (hourMin) {
//      ledConrol(LED_RESERVE_ON,LED_OFF);
        segmentControl(tempMin);
      }
      else {
//      ledConrol(LED_RESERVE_ON,LED_ON);
        segmentControl(tempHour);
      }
      break;
  }
#endif	// HPA_36C
  }
  if ((g_remoteFlag==BUTTON_CONFIRM)||(g_remoteFlag==BUTTON_CONFIRM_LONG)) {
    if ((engModeReady==TRUE)&&(g_remoteFlag==BUTTON_CONFIRM_LONG)) {
      // enter Engineer Mode
      g_remoteFlag = 0;
//      voicePlay(SWITCH_KEY, DELAY_KEY);
//      Delay(DELAY_KEY);
      changeState(STATE_ENGINEER_MODE,FALSE);
    }
    else {    // Normal operation
      g_remoteFlag = 0; engModeReady = FALSE;
      switch (settingMode) {
        case SET_PLASMA_RESERVATION :
          plasmaInfo.rsvOn = tempRsvFlag;
          plasmaInfo.rsvTime = tempRsvTime;  
          printf("[2]select rsvOn : %d\r\n", plasmaInfo.rsvOn);
          printf("[2]select rsvTime : %d\r\n", plasmaInfo.rsvTime);
          systemWrite();              // Save Reservation Time
          break;
#ifdef  INCLUDE_RESERVATION_OPERATION_CONTROL
        case SET_RESERVE_OPER_MIN :
          sysConfig.revOperMin = tempRsvOperMin;
          printf("[2]select rsvOpMin : %d\r\n", sysConfig.revOperMin);
          systemWrite();              // Save Reservation Operating Minutes
          break;
#endif
#ifdef  HUNGARIAN_ISSUE
        case SET_DUTY_RATIO :
          sysConfig.dutyRatio = tempDutyRatio;
          printf("[2]select dutyRatio : %d\r\n", sysConfig.dutyRatio);
          systemWrite();              // Save Duty Ratio
          break;
#endif
        case SET_TIME :
        default :
          RTC_TimeShow();
          printf(" tempTime = %02d:%02d",tempHour,tempMin);
          if (hourMin)  curTime.RTC_Minutes = tempMin;
          else          curTime.RTC_Hours = tempHour;
          printf("\n\r  The current time (Hour-Minute-Second) is :  %0.2d:%0.2d:%0.2d ", 
               curTime.RTC_Hours, curTime.RTC_Minutes, curTime.RTC_Seconds);
          RTC_SetTime(RTC_Format_BIN,&curTime);
          RTC_TimeShow();
          break;
      }
      voicePlay(SWITCH_KEY, DELAY_KEY);
      Delay(DELAY_KEY);
    }
  }
  if ((g_remoteFlag==BUTTON_UP)||(g_remoteFlag==BUTTON_UP_LONG)) {
    g_remoteFlag = 0; engModeReady = FALSE;
    switch (settingMode) {
      case SET_PLASMA_RESERVATION :
        tempRsvTime++;
        tempRsvTime = tempRsvTime%24;
        break;
#ifdef  INCLUDE_RESERVATION_OPERATION_CONTROL
      case SET_RESERVE_OPER_MIN :
        tempRsvOperMin += DEL_RESERVE_OPER_MIN;
        if (tempRsvOperMin>MAX_RESERVE_OPER_MIN) 
          tempRsvOperMin = MIN_RESERVE_OPER_MIN;
        break;
#endif
#ifdef  HUNGARIAN_ISSUE
      case SET_DUTY_RATIO :
        tempDutyRatio ++;
        if (tempDutyRatio>MAX_DUTY_RATIO) 
          tempDutyRatio = MIN_DUTY_RATIO;
        break;
#endif
      case SET_TIME :
      default :
        if (hourMin) {
          tempMin++;  
          tempMin = tempMin%60;
        }
        else {
          tempHour++;
          tempHour = tempHour%24;
        }
        break;
    }
  } 
  if ((g_remoteFlag==BUTTON_DOWN)||(g_remoteFlag==BUTTON_DOWN_LONG)) {
    g_remoteFlag = 0; engModeReady = FALSE;
    switch (settingMode) {
      case SET_PLASMA_RESERVATION :
        if (tempRsvTime==0) tempRsvTime = 23;
        else                tempRsvTime--;
        break;
#ifdef  INCLUDE_RESERVATION_OPERATION_CONTROL
      case SET_RESERVE_OPER_MIN :
        tempRsvOperMin -= DEL_RESERVE_OPER_MIN;
        if (tempRsvOperMin<MIN_RESERVE_OPER_MIN) 
          tempRsvOperMin = MAX_RESERVE_OPER_MIN;
        break;
#endif
#ifdef  HUNGARIAN_ISSUE
      case SET_DUTY_RATIO :
        tempDutyRatio--;
        if (tempDutyRatio<MIN_DUTY_RATIO) 
          tempDutyRatio = MAX_DUTY_RATIO;
        break;
#endif
      case SET_TIME :
      default :
        if (hourMin) {
          if (tempMin==0) tempMin = 59;
          else            tempMin--;  
        }
        else {
          if (tempHour==0) tempHour = 23;
          else            tempHour--;  
        }
        break;
    }
  } 
  if ((g_remoteFlag==BUTTON_RIGHT)||(g_remoteFlag==BUTTON_RIGHT_LONG)) {
    g_remoteFlag = 0; engModeReady = FALSE;
    voicePlay(SWITCH_KEY, DELAY_KEY);
    Delay(DELAY_KEY);
    switch (settingMode) {
      case SET_PLASMA_RESERVATION :
        tempRsvFlag ^= 1;   // toggle On-Off
        break;
#ifdef  INCLUDE_RESERVATION_OPERATION_CONTROL
      case SET_RESERVE_OPER_MIN :   // no Action
        break;
#endif
#ifdef  HUNGARIAN_ISSUE
      case SET_DUTY_RATIO :         // no Action
        break;
#endif
      case SET_TIME :       // fall-through
      default :
        hourMin ^= 1;       // toggle btw hour & min
        copyTime();         // refresh time from RTC
        ledControl(LED_RESERVE_ON,hourMin);
        break;
    }
  }
  if ((g_remoteFlag==BUTTON_PLASMA)||(g_remoteFlag==BUTTON_PLASMA_LONG)) {
    g_remoteFlag = 0; engModeReady = FALSE;
    settingMode = SET_PLASMA_RESERVATION;
    tempRsvTime = plasmaInfo.rsvTime;
    tempRsvFlag = plasmaInfo.rsvOn;
      printf("[3]select rsvOn : %d\r\n", plasmaInfo.rsvOn);
      printf("[3]select rsvTime : %d\r\n", plasmaInfo.rsvTime);
    voicePlay(SWITCH_KEY, DELAY_KEY);
    Delay(DELAY_KEY);
  }
  if ((g_remoteFlag==BUTTON_TIME)||(g_remoteFlag==BUTTON_TIME_LONG)) {
    g_remoteFlag = 0; engModeReady = FALSE;
    settingMode = SET_TIME;
    hourMin = 0;   // re-initialize
    copyTime();     // refresh time from RTC
    ledControl(LED_RESERVE_ON,hourMin);
    voicePlay(SWITCH_KEY, DELAY_KEY);
    Delay(DELAY_KEY);
  }
#ifdef  INCLUDE_RESERVATION_OPERATION_CONTROL
  if ((g_remoteFlag==BUTTON_INTENSITY)||(g_remoteFlag==BUTTON_INTENSITY_LONG)) {
    g_remoteFlag = 0; engModeReady = FALSE;
    settingMode = SET_RESERVE_OPER_MIN;
    tempRsvOperMin = sysConfig.revOperMin;
    printf("[3]select rsvOpMin : %d\r\n", tempRsvOperMin);
    voicePlay(SWITCH_KEY, DELAY_KEY);
    Delay(DELAY_KEY);
  }
#endif
#ifdef  HUNGARIAN_ISSUE
  if ((g_remoteFlag==BUTTON_DISINFECT)||(g_remoteFlag==BUTTON_DISINFECT_LONG)) {
    g_remoteFlag = 0; engModeReady = FALSE;
    settingMode = SET_DUTY_RATIO;
    tempDutyRatio = sysConfig.dutyRatio;
    printf("[3]select dutyRatio : %d\r\n", tempDutyRatio);
    voicePlay(SWITCH_KEY, DELAY_KEY);
    Delay(DELAY_KEY);
  }
#endif
  if ( g_remoteFlag==BUTTON_HUMAN_LONG ) {       // prepare to enter Engineer Mode
    g_remoteFlag = 0;
    engModeReady = TRUE;
    voicePlay(SWITCH_KEY, DELAY_KEY);
    Delay(DELAY_KEY);
  } 
  checkBaseMode(1);
  checkPowerOff(STATE_POWER_OFF);
}


void serial_disinfect(struct IotCommandSet *command){
    printf("\r\n serial_disinfect duration = %s", command->duration);
    control_relayAllOff();
    //LED control
    control_disLed();
    continueOperation = FALSE;
    disInfo.disTimer = atoi(command->duration) * 60;
    segmentControl(disInfo.disTimer / 60);
    voicePlay(SWITCH_DISINFECT_MODE, DELAY_DISINFECT_MODE);
    Delay(DELAY_DISINFECT_MODE);
    changeState(STATE_DIS, FALSE);
}


void serial_ion(struct IotCommandSet *command){//cho: 2024-4-15
    printf("\r\n serial_ion duration = %s", command->duration);
    control_relayAllOff();
    //LED control
    control_disLed();
    continueOperation = FALSE;
    ionInfo.ionTimer = atoi(command->duration) * 60;
    segmentControl(ionInfo.ionTimer / 60);   //kwon bug fix
    voicePlay(SWITCH_ANION_MODE, DELAY_ANION_MODE);
    Delay(DELAY_ANION_MODE);
    changeState(STATE_ION, FALSE);
}

void serial_plasma(struct IotCommandSet *command){//cho: 2024-4-15

    static unsigned char pwrPIDon = 3;

    printf("\r\n serial_plasma duration = %s", command->duration);
    // control_relayAllOff();
    // //LED control
    // control_disLed();
    // continueOperation = FALSE;
    // disInfo.disTimer = atoi(command->duration) * 60;
    // segmentControl(disInfo.disTimer / 60);
    // voicePlay(SWITCH_PLASMA_MODE, DELAY_PLASMA_MODE);
    // Delay(DELAY_PLASMA_MODE);

    printf("\r\n [ serial_plasma STATE_READY_STER : %d ]", currentState);
    
    voicePlay(SWITCH_PLASMA_MODE, DELAY_PLASMA_MODE);
    Delay(DELAY_PLASMA_MODE);

    prevOperation = STATE_READY_STER;   // SJM 200506 debugged 
    //relay all off
    control_relayAllOff();
    // Default setting & LED display
   
    plasmaInfo.pidOn = TRUE;
    plasmaInfo.pwr = MAX_PLASMA_PWR;
    pwrPIDon = 3;
    maxDispPower = plasmaInfo.pwr + 2;
    plasmaInfo.plasmaTimer = 2 * 60;
  
  
    continueOperation = FALSE;

     //kwon:2024-4-10
    printf("\r\n 1-1. handleReadyPlasmaSter : plasmaInfo.plasmaTimer = %d",plasmaInfo.plasmaTimer );
    printf("\r\n 1-2. handleReadyPlasmaSter : plasmaInfo.plasmaTimer/600 = %d",plasmaInfo.plasmaTimer/600 );
    
    
    if(plasmaInfo.rsvOn == TRUE) {
//      printf("select Mode : %d\r\n", plasmaInfo.modeSelect);  SJM 190715 useless??
      printf("\r\nselect rsvOn : %d\r\n", plasmaInfo.rsvOn);
      printf("\r\nselect rsvTime : %d\r\n", plasmaInfo.rsvTime);
//      plasmaInfo.modeSelect = SETTING_NONE;                   SJM 190715 useless??
    }

    changeState(STATE_STER, FALSE);
}

void serial_handler(struct IotCommandSet *command){

  printf("\r\n\r\n******** serial_handler start ***************\r\n"); 
  printf("\r\nserial_handler command power = %s\r\n", command->power); 
  printf("\r\nserial_handler command mode = %s\r\n\r\n", command->mode);
  printf("\r\nserial_handler command wind = %s\r\n\r\n", command->wind);
  printf("\r\nserial_handler command duration  = %s\r\n\r\n", command->duration);

 
  if(strcmp(command->mode, "1") == 0)//plasma mode  //cho: 2024-4-15
  {
    printf("\r\n plasma mode enter 1....");
    serial_plasma(command);
    // strcpy(command->mode, "2");
    // printf("\r\n disinfect mode enter ....");
    // serial_disinfect(command);
    // strcpy(command->mode, "3");
    // printf("\r\n ion mode enter ....");
    // serial_ion(command); 
  }

  else if(strcmp(command->mode, "2") == 0) //disinfect mode  //cho: 2024-4-15
  {
   
    
      printf("\r\n disinfect mode start ....");
      serial_disinfect(command);
    
    
  }

  else if(strcmp(command->mode, "3") == 0)
  {
    printf("\r\n ion mode enter 3....");
    serial_ion(command);
  }

  else if(strcmp(command->mode, "4") == 0){
    printf("\r\rn power on mode 4....");
  }

  else if(strcmp(command->mode, "5") == 0){
    printf("\r\rn power off mode 5....");

    //현재의 상태가 Power off 상태가 아니면, power off를 한다.
    if(currentState != STATE_POWER_OFF){
      printf("\r\n current state isn't power off");
        g_remoteFlag = 0;
        voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
        Delay(DELAY_POWER_OFF);
        
        //eeprom write 필요.
        changeState(STATE_POWER_OFF,TRUE);
    }
    else{
      printf("\r\n error: current state is power off");
    }                                                                                                                                                                                                                                                  
  }

  else if(strcmp(command->mode, "6") == 0){
    printf("\r\n stop mode 6....");

    if(currentState == STATE_DIS){
      printf("\r\n current state is STATE_DIS");
        g_remoteFlag = 0;
        //eeprom write 필요.
        changeState(STATE_DIS_STOP, FALSE);
    }
    else if(currentState == STATE_ION){
      printf("\r\n current state is STATE_ION");
        g_remoteFlag = 0;
        //eeprom write 필요.
        changeState(STATE_ION_STOP, FALSE);
    }
    else{
      printf("\r\n current state error..... ");
    }
  }
    

  else //error mode //cho: 2024-4-15
  {
    printf("\r\n mode error ....");
  }
}


void handler()
{
  switch(currentState) {
    case STATE_POWER_OFF:
      // printf("\r\n STATE_POWER_OFF - [ Current State = %d, entrance = %d ]", currentState, isFirstEntrance);
      handlePowerOff();
      break;
    case STATE_INIT:
      handleInit();
      break;
    case STATE_READY_STER:
      handleReadyPlasmaSter();
      break;
    case STATE_STER:
      handlePlasmaSter();
      break;
    case STATE_STER_STOP:
      handlePlasmaSterStop();
      break;
    case STATE_READY_DIS:
      handleReadyDisinfect();
      break;

    //kwon: 2024-4-11 
    case STATE_SERIAL_READY_DIS:
      handleSerialReadyDisinfect(30 * 60);
      break;


    case STATE_DIS:
      handleDisinfect();
      break;
    case STATE_DIS_STOP:
      handleDisinfectStop();
      break;
    case STATE_READY_ION:
      handleReadyIon();
      break;
    case STATE_ION:
      handleIon();
      break;
    case STATE_ION_STOP:
      handleIonStop();
      break;
    case STATE_CONSUMABLE_WARNING:
      handleConsumableWarning();
      break;
    case STATE_DESTRUCTION:
      handleDestruction();
      break;
    case STATE_PREPARE:
      handlePrepare();
      break;
    case STATE_ENGINEER_MODE:
      handleEngineerMode();
      break;
    case STATE_SETUP_MODE:
      handleSetup();
      break;
#ifdef USE_TNY_1311S_REMOTE
  #if (MACHINE==HPA_130W)
    case STATE_CONCENTRATION :
      handleConcentration();
      break;
  #endif
    case STATE_TIME_SETUP :
      handleTimeSetup();
      break;
    case STATE_ENGINEER_SETUP :
      handleEngineerSetup();
      break;
    case STATE_BOOK_TIME :
      handleBookTime();
      break;
    case STATE_BOOK_CONFIG :
      handleBookConfig();
      break;
#endif // USE_TNY_1311S_REMOTE
//#ifdef  MONEY_STERILIZER
#if ( MACHINE == MONEY_STERILIZER )
    case STATE_MONEY_STANDBY:
      handleMoneyReady();
      break;
    case STATE_MONEY_STER:
      handleMoneySterilization();
      break;
    case STATE_MONEY_STOP:
      handleMoneySterStop();
      break;
    case STATE_MONEY_DECOMP:
      handleMoneyDecomposition();
      break;
#endif
  }
}
