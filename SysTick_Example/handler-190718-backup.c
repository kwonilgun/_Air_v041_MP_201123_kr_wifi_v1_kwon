#include "handler.h"

extern void ledAllOn();

//unsigned char exMin;    SJM 190711 it is assigned but never used anywhere...???
unsigned int segmentCycle=1;
unsigned int g_RemoteOzoneSensorValue;

//unsigned int  g_countPlamsaButtonPressed;   SJM 190711 it is never used anywhere...???
extern unsigned int g_extOzoneSenseFlag;
extern unsigned char halfSecFlag;

RTC_TimeTypeDef curTime;
unsigned int tempHour, tempMin, tempRsvTime, tempRsvFlag;
unsigned char ledStatus = 0;
//unsigned int ledStep, opMode;

void changeState(unsigned char state)
{
  currentState = state;
  isFirstEntrance = TRUE;
  printf("\r\n>> switch to State %d\r\n", state);
}

void  checkStart()
{
  if(g_remoteFlag & (BUTTON_RIGHT | BUTTON_RIGHT_LONG)) {
    g_remoteFlag = 0;
    isFirstEntrance = TRUE;
    switch (currentState) {
      case STATE_READY_STER :
        if(plasmaInfo.pidOn == TRUE)  currentState = STATE_PREPARE;
        else                          currentState = STATE_STER;
//      currentState = STATE_STER_TIME_SET;   SJM 190715 remove STATE_STER_TIME_SET
        plasmaInfo.Mode = PLASMA_MODE_START;
        break;
      case STATE_READY_DIS :
        currentState = STATE_DIS;
        break;
      case STATE_READY_ION :
        currentState = STATE_ION;
        break;
      default :
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
        printf("\r\n[Error State!! currentState = %d]\r\n", currentState);
        break;
    }
  }
}
#ifdef  UNIFY_SOURCE
void checkPowerOff(unsigned int nextState)
{
  if(g_remoteFlag & (BUTTON_POWER | BUTTON_POWER_LONG)) {
    g_remoteFlag = 0;
    if (nextState == STATE_POWER_OFF) {
      voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
      Delay(DELAY_POWER_OFF);
    }
    changeState(nextState);  
  }
  if(g_keyFlag & KEY_PWR_FLAG) {
    g_keyFlag = 0; 
    changeState(nextState);  
  }
}
#endif
#ifdef  INCLUDE_CHECK_BASE
void checkBaseMode()
{
  //Ion Mode
  if(g_remoteFlag & (BUTTON_ANION | BUTTON_ANION_LONG)) {
    g_remoteFlag = 0;
    changeState(STATE_READY_ION);
  }  
  //��� ���
  if(g_remoteFlag & (BUTTON_DISINFECT | BUTTON_DISINFECT_LONG)) {
    g_remoteFlag = 0;
    changeState(STATE_READY_DIS);
  }
/*  SJM 190710 do not use key anymore
  if(g_keyFlag & KEY_O3_FLAG) {
    g_keyFlag = 0;
//    voicePlay(SWITCH_KEY, DELAY_KEY);
    currentState = STATE_READY_DIS;
    isFirstEntrance = TRUE;
  }*/
  //�ö�� ���
  if(g_remoteFlag & (BUTTON_PLASMA | BUTTON_PLASMA_LONG)) {
    g_remoteFlag = 0;
    changeState(STATE_READY_STER);
  }
}
#endif
void handlePowerOff()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_POWER_OFF : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    control_relayAllOff();
    ledAllOff();
//    segmentOff();       SJM 190715 included in ledAllOff()
    systemWrite();
//    pidLEDOnFlag = FALSE;     SJM 190711 always assigned to 'FALSE' & never used
    pidDetect = FALSE;
//    if(plasmaInfo.rsvOn == TRUE)
//      ledControl(LED_RESERVE_ON, LED_ON);
#ifdef  AUTO_POWER_ON
    currentState = STATE_INIT;  
    isFirstEntrance = TRUE;
#endif
  }
  
  if (halfSecFlag) {
    halfSecFlag = FALSE;
    if (plasmaInfo.rsvOn) {
      ledStatus ^= 1;
      ledControl(LED_RESERVE_ON, ledStatus);
    }
  }

#ifdef  UNIFY_SOURCE
  checkPowerOff(STATE_INIT);
#else  
  if(g_remoteFlag & (REMOTE_POWER_LONG_FLAG | REMOTE_POWER_FLAG))
  {
    g_remoteFlag = 0;
    
    currentState = STATE_INIT;  
    isFirstEntrance = TRUE;
  }  
  if(g_keyFlag & KEY_PWR_FLAG)
  {
    g_keyFlag = 0;
    
    currentState = STATE_INIT;  
    isFirstEntrance = TRUE;
  }
#endif  
}

void handleConsumableWarning()
{
  static unsigned int warningType[WARNING_NUMBER];
  static unsigned int warningNum;
  static unsigned int warning;
  static unsigned int warningCount;
  static unsigned char resetPreFlag;
  
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_CONSUMABLE_WARNING : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    //LED control
    control_consumableCheckLed();
    //FND control
    segmentControl(0);
    //warning check
    warning = consumableCheck();
    for(int i = 0; i < WARNING_NUMBER; i++) {
      printf("compare[%d] 0x%x & 0x%x\r\n", i, warning, (1 << i));
      if(warning & (1 << i)) {
        warningType[i] = (1 << i);
        warningNum++;
        printf("warning : 0x%x\r\n", (1 << i));
      }
    }
    printf("warningType[%d] : 0x%x\r\n", warningNum, warning);
    resetPreFlag = FALSE;
    printf("Consumable Warning mode\r\n");
  }
  
  //blink
  {
    if(sysConfig.blinkOffFlag == TRUE) {
      if(++warningCount > warningNum)    warningCount = 0;
      if(warningType[warningCount] & WARNING_OZONE_LAMP) segmentAlphaControl('E','O');
      else if(warningType[warningCount] & WARNING_UV_LAMP) segmentAlphaControl('E','U');
/* 2016.12.13. DH Kwon Requested 
      else if(warningType[warningCount] & WARNING_FILTER) segmentAlphaControl('E','F');*/      
      sysConfig.blinkOffFlag = FALSE;
    }
    if(sysConfig.blinkOnFlag == TRUE) {
      segmentOff();
      sysConfig.blinkOnFlag = FALSE;
    }
  }
  
  //reset preKey
  if(g_remoteFlag & (REMOTE_UP_LONG_FLAG))
  {
    g_remoteFlag = 0;
#ifdef  INCLUDE_REMOTE_ACK
    voicePlay(SWITCH_KEY, DELAY_KEY);    
#endif
    resetPreFlag = TRUE;
    printf("preKey On!!\r\n");
  }
  
  //uvlamp reset
  if(g_remoteFlag & (REMOTE_WIND_LONG_FLAG)) {
    g_remoteFlag = 0;
    if(resetPreFlag == TRUE) {
#ifdef  INCLUDE_REMOTE_ACK
      voicePlay(SWITCH_KEY, DELAY_KEY);    
#endif
      resetPreFlag = FALSE;
      sysConfig.uvLampCountHour = 0;
      sysConfig.uvLampCountMin = 0;
      systemWrite();
      printf("reset uvlamp!!\r\n");
    }
  }
  if(g_remoteFlag & (REMOTE_MENU_LONG_FLAG | REMOTE_MENU_FLAG)) {
    g_remoteFlag = 0;
#ifdef  INCLUDE_REMOTE_ACK
    voicePlay(SWITCH_KEY, DELAY_KEY);    
#endif    
    currentState = STATE_READY_STER;
    isFirstEntrance = TRUE;
  }
  
  //ozonelamp reset
  if(g_remoteFlag & (REMOTE_ION_LONG_FLAG)) {
    g_remoteFlag = 0;
    if(resetPreFlag == TRUE) {
#ifdef  INCLUDE_REMOTE_ACK
      voicePlay(SWITCH_KEY, DELAY_KEY);    
#endif
      resetPreFlag = FALSE;
      sysConfig.ozoneLampCountHour = 0;
      sysConfig.ozoneLampCountMin = 0;
      systemWrite();
      printf("reset ozonelamp!!\r\n");
    }
  }
#ifndef REMOVE_FILTER_COUNT  
  //filter reset
  if(g_remoteFlag & (REMOTE_PLASMA_LONG_FLAG)) {
    g_remoteFlag = 0;
    if(resetPreFlag == TRUE) {
  #ifdef  INCLUDE_REMOTE_ACK
      voicePlay(SWITCH_KEY, DELAY_KEY);    
  #endif
      resetPreFlag = FALSE;
      sysConfig.filterCountHour = 0;
      sysConfig.filterCountMin = 0;
      systemWrite();
      printf("filter reset!!\r\n");
    }
  }
#endif
#ifdef  UNIFY_SOURCE
  checkPowerOff(STATE_INIT);
#else
  //reboot
  if(g_remoteFlag & (REMOTE_POWER_LONG_FLAG | REMOTE_POWER_FLAG)) {
    g_remoteFlag = 0;
    currentState = STATE_INIT;  
    isFirstEntrance = TRUE;
  }
  if(g_keyFlag & KEY_PWR_FLAG) {
    g_keyFlag = 0; 
    currentState = STATE_INIT;  
    isFirstEntrance = TRUE;
  }
#endif
}

void handleInit()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_INIT : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    //eeprom data loading.
    systemRead();
    //�ʱ� voice
    voicePlay(SWITCH_POWER_ON, DELAY_POWERON);
    //S/W ����
    segmentControl(SW_VERSION);
    //LED ����
    control_initLed();
    pidDetect = FALSE;
    WhiteLed();
    
    //�Ҹ�ǰ üũ.
    if(consumableCheck() > 0) {
      currentState = STATE_CONSUMABLE_WARNING;
    }
    else {
      if(sysConfig.modeFlag == MODE_STER) {
        currentState = STATE_READY_STER;
//        plasmaInfo.modeSelect = SETTING_NONE;     SJM 190715 useless??
      }
      else if(sysConfig.modeFlag == MODE_DIS) {
        currentState = STATE_READY_DIS;
      }
      else if(sysConfig.modeFlag == MODE_ION) {
        currentState = STATE_READY_ION;
      }
      else {
    #ifndef  CHANGE_DEFAULAT_MODE
        currentState = STATE_READY_STER;
    #else
        currentState = STATE_READY_DIS;
    #endif        
      }
    }
    isFirstEntrance = TRUE;
  }
}

void handleReadySter()
{
  static unsigned char editMode = 0;  // 0=Time, 1=Intensity
  
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_READY_STER : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    //led Font on
    control_ledFont();
    ledControl(LED_PLASMA_ON, LED_ON);
    //relay all off
    control_relayAllOff();
    // Default setting & LED display
    editMode = 0;
    plasmaInfo.pidOn = TRUE;
    plasmaInfo.pwr = 3;
    plasmaInfo.plasmaTimer = 30 * 60;
    plasmaInfo.continuation = FALSE;
    ledControl(LED_PID_ON, plasmaInfo.pidOn);
    ledControl(LED_RESERVE_ON, plasmaInfo.rsvOn);
    segmentControl(plasmaInfo.plasmaTimer/60);

    if(plasmaInfo.rsvOn == TRUE) {
//      printf("select Mode : %d\r\n", plasmaInfo.modeSelect);  SJM 190715 useless??
      printf("select rsvOn : %d\r\n", plasmaInfo.rsvOn);
      printf("select rsvTime : %d\r\n", plasmaInfo.rsvTime);
//      plasmaInfo.modeSelect = SETTING_NONE;                   SJM 190715 useless??
    }
#ifdef  ENGLISH_VOICE
    voicePlay(SWITCH_PLASMA_MODE, DELAY_PLASMA_MODE);
    Delay(DELAY_PLASMA_MODE);
#endif
  }

  if (halfSecFlag) {
    halfSecFlag = FALSE;
    ledStatus ^= 1;
    ledControl(LED_PID_ON, plasmaInfo.pidOn);
    ledControl(LED_PID_FONT, plasmaInfo.pidOn);
    switch (editMode) {
      case 0 :  // Time Adjust
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
        if (plasmaInfo.continuation)  segmentAlphaControl('O','n');
        else                          segmentControl(plasmaInfo.plasmaTimer/60);
        break;
      case 1 :  // Intensity Adjust
        ledControl(LED_POWER_FONT, ledStatus);
        ledControl(LED_POWER_ON, ledStatus);
        ledControl(LED_RESERVE_FONT, LED_OFF);
        segmentControl(plasmaInfo.pwr);
        break;
    }
  }

  //Power setting
  if(g_remoteFlag & (BUTTON_INTENSITY | BUTTON_INTENSITY)) {
    g_remoteFlag = 0;
    if (editMode==0) {
      editMode = 1;
      voicePlay(SWITCH_KEY, DELAY_KEY);
    }
    else {
      if(++plasmaInfo.pwr > 3)  plasmaInfo.pwr = 1;
    }
//    segmentControl(plasmaInfo.pwr);
//    plasmaInfo.modeSelect = SETTING_POWER;      SJM 190715 useless??
    printf("power setting\r\n");
  }
  //PID setting
  if(g_remoteFlag & (REMOTE_PLASMA_FLAG | REMOTE_PLASMA_LONG_FLAG)) {
    g_remoteFlag = 0;
    voicePlay(SWITCH_KEY, DELAY_KEY);
    plasmaInfo.pidOn ^= 1;
    ledControl(LED_PID_ON,plasmaInfo.pidOn);
  }
  //Time setting
  if (g_remoteFlag & ( BUTTON_TIME )) {
    g_remoteFlag = 0;
    editMode = 0;
    voicePlay(SWITCH_KEY, DELAY_KEY);
  }
  //Continuation Mode
  if (g_remoteFlag & ( BUTTON_TIME_LONG )) {
    g_remoteFlag = 0;
    editMode = 0;
    plasmaInfo.continuation = TRUE;
    voicePlay(SWITCH_KEY, DELAY_KEY);
  }
  //up Key
  if(g_remoteFlag & (REMOTE_UP_FLAG | REMOTE_UP_LONG_FLAG)) {
    g_remoteFlag = 0;
    if (editMode) {   // Intensity Adjust
      if(++plasmaInfo.pwr > 3)  plasmaInfo.pwr = 1;
    }
    else {            // Time Adjust
      plasmaInfo.continuation = FALSE;
      if(plasmaInfo.plasmaTimer < 60 * 60)
        plasmaInfo.plasmaTimer += 5 * 60;
      voicePlay(SWITCH_KEY, DELAY_KEY);
//    segmentControl(plasmaInfo.plasmaTimer / 60);
    }
  }
  //down key
  if(g_remoteFlag & (REMOTE_DOWN_FLAG | REMOTE_DOWN_LONG_FLAG)) {
    g_remoteFlag = 0;
    if (editMode) {   // Intensity Adjust
      if(--plasmaInfo.pwr < 1)  plasmaInfo.pwr = 3;
    }
    else {            // Time Adjust
      plasmaInfo.continuation = FALSE;
      if(plasmaInfo.plasmaTimer > 5 * 60)
        plasmaInfo.plasmaTimer -= 5 * 60;
      voicePlay(SWITCH_KEY, DELAY_KEY);
//    segmentControl(plasmaInfo.plasmaTimer / 60);
    }
  }
  
  checkStart();
  checkBaseMode();
  checkPowerOff(STATE_POWER_OFF);
}

void handlePrepare()
{
  if(isFirstEntrance == TRUE)
  {
    printf("\r\n [ STATE_PREPARE : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    
    voicePlay(SWTICH_PREPARE, DELAY_PREPARE);
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
  if(prepareTimer <= 0)
  {
    currentState = STATE_STER;
    isFirstEntrance = TRUE;
  }
  
  //stop key -> Plasma mode Need to be Fixed REMOTE_STOP_FLAG
  if(g_remoteFlag & (REMOTE_POWER_LONG_FLAG | REMOTE_POWER_FLAG))
  {
    g_remoteFlag = 0;
    
    //voice -> ����� �ߴ� ����
    
    currentState = STATE_READY_STER;
    isFirstEntrance = TRUE;
  }
     
  //Power off key
#ifdef  UNIFY_SOURCE
  checkPowerOff(STATE_POWER_OFF);
#else  
  if(g_remoteFlag & (REMOTE_POWER_LONG_FLAG | REMOTE_POWER_FLAG))
  {
    g_remoteFlag = 0;
    
    //voice -> Power Off
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    Delay(DELAY_POWER_OFF);
    
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
  
  if(g_keyFlag & KEY_PWR_FLAG)
  {
    g_keyFlag = 0; 
    
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
#endif  // UNIFY_SOURCE  
}

void handleSter()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_STER : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    segmentCycle = 1;
    plasmaBlinkOn = TRUE;
    plasmaBlinkOnTimer = 0;
    plasmaBlinkOffTimer = 0;
#ifdef  OZONE_DEPENDENT_DESTRUCTION     // SJM 190711 add '#ifdef' because of g_prvOzoneValue
    g_prvOzoneValue = FALSE;
#endif
    //voice play
    voicePlay(SWITCH_PLASMA_START, DELAY_PLASMA_START);
    Delay(DELAY_PLASMA_START);
    //relay control
    control_sterOn();
    //led control
    control_sterStartLedOn();
    //���� ������ ���
    if(plasmaInfo.Mode == PLASMA_MODE_RESERVE) {
      plasmaInfo.pwr = 3;
      plasmaInfo.plasmaTimer = 95 * 60;
      plasmaInfo.pidOn = TRUE;
    }
    
    if(plasmaInfo.pidOn == TRUE) {
      ledControl(LED_PID_ON, LED_ON);
    }
    /*
    //time setting
    if(plasmaInfo.Mode == PLASMA_MODE_START)
    {
      plasmaInfo.plasmaTimer = 30 * 60; //30min  
      segmentControl(plasmaInfo.plasmaTimer);
    }
    else if(plasmaInfo.Mode == PLASMA_MODE_STOP)
    {
      //nothing
      segmentControl(plasmaInfo.plasmaTimer / 60);
    }*/
  }
  
  //Get Remote Ozone Sensor Value
  if( g_remoteFlag & (REMOTE_OZ1_FLAG | REMOTE_OZ2_FLAG | REMOTE_OZ3_FLAG | REMOTE_OZ4_FLAG | REMOTE_OZ5_FLAG) ) {
    switch(g_remoteFlag) {
      case REMOTE_OZ1_FLAG:
        g_RemoteOzoneSensorValue = 5;
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
  else if( g_remoteFlag & (REMOTE_OZ6_FLAG | REMOTE_OZ7_FLAG | REMOTE_OZ8_FLAG) ) {
    switch(g_remoteFlag) {
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
#ifdef  CHECK_OZONE_LIMIT
    CheckRoomTempEEPWrite(CheckOZLimitEEPData);
#endif
    g_remoteFlag = 0;
  }  
  
  //blink
  {
    if(plasmaInfo.blinkOffFlag == TRUE) {
      //led control
      ledControl(LED_PLASMA_ON, LED_ON);
      //segment control
//      if(plasmaInfo.continuation == FALSE) {    // SJM 190716 if TRUE ==> no display???
        if(segmentCycle == 1) {
          if(plasmaInfo.continuation) segmentAlphaControl('O','n');
          else         segmentControl(plasmaInfo.plasmaTimer / 60);
        }
        else if(segmentCycle == 2) {
          if(dOzoneSensoredValue < 99) {  
            segmentControl( (unsigned int)(dOzoneSensoredValue -1) );
            Delay(500);
          }
          else {
            segmentControl(99);
            Delay(500);    
          }
        }
        else if(segmentCycle == 3) {
          if( g_extOzoneSenseFlag == TRUE) {
             g_extOzoneSenseFlag = FALSE;
             if( g_RemoteOzoneSensorValue == 5) {
               segmentControl(00);
               //g_RemoteOzoneSensorValue = 0;
             }
             else {
               segmentControl(g_RemoteOzoneSensorValue);
               //g_RemoteOzoneSensorValue = 0;
             }
          }
          else {
            segmentAlphaControl('-','-');
          }
          segmentCycle = 0;
        }
        segmentCycle++;
//      }                                           // SJM 190716 if TRUE ==> no display???
/*      else                                        // these 2 lines were originally commented.
        segmentAlphaControl('O','n');
*/      
      plasmaInfo.blinkOffFlag = FALSE;
    }
    if(plasmaInfo.blinkOnFlag == TRUE) {
      ledControl(LED_PLASMA_ON, LED_OFF);
      segmentOff();
      plasmaInfo.blinkOnFlag = FALSE;
    }
  }

  //1 Sec ozone detect
/* 
  if( g_EnhancedPlasmaSelect == TRUE) // Enhanced Mode
  {
    //printf("\r\n 1 - dOzoneSensoredValue = %d", dOzoneSensoredValue);

    if( g_RemoteOzoneSensorValue < 50 )
    {
      g_prvOzoneValue = FALSE;
      control_sterOn();      
    }
    else
    {
      g_prvOzoneValue = TRUE;
#ifndef REPLACE_AC_FAN1_TO_PLASMA
      relayControl(RELAY_PLASMA, RELAY_OFF);           //PLASMA1
#endif
      relayControl(RELAY_PLASMA2, RELAY_OFF);          //PLASMA2  
        
      //��ü ���� �߿��� ���� ������ �۵����� �ʴ´�.
      if(plasmaInfo.pidOn == FALSE)
      {
#ifndef REPLACE_AC_FAN1_TO_PLASMA
      relayControl(RELAY_PLASMA, RELAY_OFF);           //PLASMA1
#endif
        relayControl(RELAY_PLASMA2, RELAY_OFF);          //PLASMA2  
      }
    }
  }
*/  
//  else  // Normal Mode
  {
    //printf("\r\n 2 - dOzoneSensoredValue = %d", dOzoneSensoredValue);
    //if( dOzoneSensoredValue < 50)
    {
        if(plasmaBlinkOffFlag == TRUE)
        {
          control_sterOn();
          plasmaBlinkOffFlag = FALSE;
          RTC_TimeShow();
          printf("\r\n Plasma --> ON : %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
        }
        
        if(plasmaBlinkOnFlag == TRUE)
        {
/*
          if(plasmaInfo.pwr == 2)
          {
#ifndef REPLACE_AC_FAN1_TO_PLASMA
            relayControl(RELAY_PLASMA2, RELAY_OFF);          //PLASMA2
#else
            ;
#endif
          }
          else if(plasmaInfo.pwr == 3)
          {
#ifndef REPLACE_AC_FAN1_TO_PLASMA
            relayControl(RELAY_PLASMA, RELAY_OFF);           //PLASMA1
#endif
            relayControl(RELAY_PLASMA2, RELAY_OFF);          //PLASMA2
          }
*/
          relayControl(RELAY_OZONE_LAMP, RELAY_OFF);       //OZONE Lamp
          relayControl(RELAY_PLASMA2, RELAY_ON);          //PLASMA2

          RTC_TimeShow();
          printf("\r\n Plasma --> OFF : %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);          
          
          plasmaBlinkOnFlag = FALSE;
        }
        
        #ifdef  STAND_TYPE_ENABLE    
        if(fanBlinkOffFlag == TRUE)
        {
          relayControl(RELAY_AC_FAN1, RELAY_ON);
          fanBlinkOffFlag = FALSE;
        }
        
        if(fanBlinkOnFlag == TRUE)
        {
          relayControl(RELAY_AC_FAN1, RELAY_OFF);
          fanBlinkOnFlag = FALSE;        
        }
        #endif
    }
/*    
    else
    {
        control_relayAllOff();
    }
*/
  }
 
/*  
  /////////////////////////////////////////////////////////
  ///////////plasma control 1min on -> 10sec off///////////
//  if(getOzoneSensor() == FALSE)
  if(g_remoteFlag & (REMOTE_OZ1_FLAG) )
  {
    g_remoteFlag = 0;
    segmentControl(45);
    Delay(100);
    printf("Ozone Not DetectFlag SET 6\r\n");    
    if(plasmaBlinkOffFlag == TRUE)
    {
//      printf("relay On!![%d]\r\n", plasmaInfo.pwr);
      printf("Ozone Not DetectFlag SET 6\r\n");
      //relay control
      control_sterOn();
      
      plasmaBlinkOffFlag = FALSE;
    }
    
    if(plasmaBlinkOnFlag == TRUE)
    {
      printf("relay Off!![%d]\r\n", plasmaInfo.pwr);
      
      if(plasmaInfo.pwr == 2)
      {
        relayControl(RELAY_PLASMA, RELAY_OFF);           //PLASMA1
      }
      else if(plasmaInfo.pwr == 3)
      {
        relayControl(RELAY_PLASMA, RELAY_OFF);           //PLASMA1
        relayControl(RELAY_PLASMA2, RELAY_OFF);          //PLASMA2
      }
      
      plasmaBlinkOnFlag = FALSE;
    }

#ifdef  STAND_TYPE_ENABLE    
    if(fanBlinkOffFlag == TRUE)
    {
      relayControl(RELAY_AC_FAN1, RELAY_ON);
      fanBlinkOffFlag = FALSE;
    }
    
    if(fanBlinkOnFlag == TRUE)
    {
      relayControl(RELAY_AC_FAN1, RELAY_OFF);
      fanBlinkOnFlag = FALSE;        
    }
#endif
    
  }
*/  
  //////////////////////////////////////////////////////

#ifdef  ADD_REMOTE_OZONE_SENSOR  
  if( (g_RemoteOzoneSensorValue > 50) || (dOzoneSensoredValue > 50) )
  {
    voicePlay(SWITCH_OZONE_DETECT, DELAY_OZONE_DETECT);
    Delay(DELAY_OZONE_DETECT);
    
    currentState = STATE_DESTRUCTION;
    isFirstEntrance = TRUE;
  }
#endif
  
  //time over
  if(plasmaInfo.plasmaTimer <= 0 && plasmaInfo.continuation == FALSE)
  {
    plasmaInfo.Mode = PLASMA_MODE_READY;
    currentState = STATE_DESTRUCTION;
    isFirstEntrance = TRUE;
    
    systemWrite();
    
    printf("Time Over!!\r\n");
  }

  //PIR control
  if(plasmaInfo.pidOn == TRUE)
  {
    if(pidDetect == TRUE)
    {
   
      voicePlay(SWITCH_PIR_DETECT, DELAY_PIR_DETECT);
      Delay(DELAY_PIR_DETECT);
      
      pidDetect = FALSE;
      currentState = STATE_DESTRUCTION;
      isFirstEntrance = TRUE;
    }
  }
 
  //0.8v -> ���� ��ư�� ������ ���� ���� ���°� ǥ�õȴ�.
  if(g_remoteFlag & (REMOTE_ION_FLAG | REMOTE_ION_LONG_FLAG))
  {
    g_remoteFlag = 0;
   
    segmentControl(plasmaInfo.pwr);
    Delay(800);
    
    if(plasmaInfo.continuation == FALSE)
        segmentControl(plasmaInfo.plasmaTimer / 60);
      else
        segmentAlphaControl('O','n');
  }
  
  //���� Ű
#ifdef  UNIFY_SOURCE
  checkPowerOff(STATE_POWER_OFF);
#else
  if(g_remoteFlag & (REMOTE_POWER_LONG_FLAG | REMOTE_POWER_FLAG)) {
    g_remoteFlag = 0;
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
  if(g_keyFlag & KEY_PWR_FLAG) {
    g_keyFlag = 0; 
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
#endif  
  //STOP
  if(g_remoteFlag & (REMOTE_OK_LONG_FLAG | REMOTE_OK_FLAG))
  {
    g_remoteFlag = 0;

    plasmaInfo.Mode = PLASMA_MODE_STOP;
    isFirstEntrance = TRUE;
    currentState = STATE_STER_STOP;
    
    printf("go ster stop\r\n");
  }
#if 0  // SJM 190711 never used
  //Time discount
  if(exMin != (plasmaInfo.plasmaTimer / 60))
  {
    exMin = plasmaInfo.plasmaTimer / 60;
  }
#endif
#if 0     // SJM 190710 duplicated  
  if(g_keyFlag & KEY_PWR_FLAG)
  {
    g_keyFlag = 0; 
    
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
#endif  
}
  
void handleSterStop()
{
  if(isFirstEntrance == TRUE)
  {
    printf("\r\n [ STATE_STER_STOP : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    
    control_relayAllOff();
    
    voicePlay(SWITCH_PLASMA_STOP, DELAY_PLASMA_STOP);
    
    plasmaInfo.Mode = PLASMA_MODE_STOP;
    
    //LED and FND control
    control_sterStopLedOn();
    segmentControl(plasmaInfo.plasmaTimer / 60);
    
    systemWrite();
    
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
  if(g_remoteFlag & (REMOTE_OK_LONG_FLAG | REMOTE_OK_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    isFirstEntrance = TRUE;
    currentState = STATE_STER;
  }
#ifdef  INCLUDE_CHECK_BASE
  checkBaseMode();
#else  
  //Ion Mode
  if(g_remoteFlag & (REMOTE_TIMER_FLAG | REMOTE_TIMER_LONG_FLAG))
  {
    g_keyFlag = 0;
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    currentState = STATE_READY_ION;
    isFirstEntrance = TRUE;
  }
  
  //��� ���
  if(g_remoteFlag & (REMOTE_O3_FLAG | REMOTE_O3_LONG_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    currentState = STATE_READY_DIS;
    isFirstEntrance = TRUE;
  }

  if(g_keyFlag & KEY_O3_FLAG)
  {
    g_keyFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    currentState = STATE_READY_DIS;
    isFirstEntrance = TRUE;
  }
  
  //�ö�� ���
  if(g_remoteFlag & (REMOTE_STERILIZATION_FLAG | REMOTE_STERILIZATION_LONG_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    currentState = STATE_READY_STER;
    isFirstEntrance = TRUE;
  }

  if(g_keyFlag & KEY_STER_FLAG)
  {
    g_keyFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    currentState = STATE_READY_STER;
    isFirstEntrance = TRUE;
  }
#endif  // INCLUDE_CHECK_BASE  
  //���� Ű
#ifdef  UNIFY_SOURCE
  checkPowerOff(STATE_POWER_OFF);
#else
  if(g_remoteFlag & (REMOTE_POWER_LONG_FLAG | REMOTE_POWER_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
  
  if(g_keyFlag & KEY_PWR_FLAG)
  {
    g_keyFlag = 0; 
    
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
#endif
}

void handleDestruction()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_DESTRUCTION : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;  
    //voice Play(����� ���� ��)
    voicePlay(SWITCH_DESTRUCTION_START, DELAY_DESTRUCTION_START);
    Delay(DELAY_DESTRUCTION_START);
    //relay Control
    control_relayAllOff();
    control_relayDestruction();
    //Led Control
    control_desLed();
//    ledAllOff();    
    //Time Setting -> 30min
#ifdef  OZONE_DEPENDENT_DESTRUCTION
    if( g_prvOzoneValue == FALSE )  destructionTimer = 10 * 60;
    else                            destructionTimer = 20 * 60;
#else
    destructionTimer = 30 * 60;
#endif
    //segment Control
    segmentControl(destructionTimer / 60);
  }
  
  //blink
  {
    if(sysConfig.blinkOffFlag == TRUE)
    {
      ledControl(LED_PLASMA_ON, LED_ON);
      ledControl(LED_DIS_ON, LED_ON);
      ledControl(LED_ION_ON, LED_ON);
      
      segmentControl(destructionTimer / 60);
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
  
  {
     //REDLed();
  }
  
#ifdef  STAND_TYPE_ENABLE    
    if(fanBlinkOffFlag == TRUE)
    {
      relayControl(RELAY_AC_FAN1, RELAY_ON);
      fanBlinkOffFlag = FALSE;
      RTC_TimeShow();
      printf("\n\r FAN_1_ON : %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
    }
    
    if(fanBlinkOnFlag == TRUE)
    {
      relayControl(RELAY_AC_FAN1, RELAY_OFF);
      fanBlinkOnFlag = FALSE;        
      RTC_TimeShow();
      printf("\n\r FAN_1_OFF : %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
    }
#endif  
  
  //���� Ű -> Power Off
#ifdef  UNIFY_SOURCE
  checkPowerOff(STATE_POWER_OFF);
#else
  if(g_remoteFlag & (REMOTE_POWER_LONG_FLAG | REMOTE_POWER_FLAG))
  {
    g_remoteFlag = 0;
    
    //voice Play(Power Off ��)
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    Delay(DELAY_POWER_OFF);
    
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }

  if(g_keyFlag & KEY_PWR_FLAG)
  {
    g_keyFlag = 0; 
    
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
#endif  
  //��ž Ű -> �ö�� Ready
  if(g_remoteFlag & (REMOTE_OK_FLAG | REMOTE_OK_LONG_FLAG))
  {
    g_remoteFlag = 0;
    
    //voice Play(����� �ߴ� ��)
    voicePlay(SWITCH_DESTRUCTION_STOP, DELAY_DESTRUCTION_STOP);
    Delay(DELAY_DESTRUCTION_STOP);
    
    currentState = STATE_READY_STER;
    isFirstEntrance = TRUE;
  }
  
  
  //time over -> �ö�� Ready
  if(destructionTimer <= 0)
  {
    currentState = STATE_READY_STER;
    isFirstEntrance = TRUE;
  }
}

void handleReadyDisinfect()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_READY_DIS : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    //LED control
    control_disLed();
    //relay Control
    control_relayAllOff();
//    pidLEDOnFlag = FALSE;     SJM 190711 always assigned to 'FALSE' & never used
    //DIS FND
    disInfo.continuation = FALSE;
    disInfo.disTimer = 60 * 60;
    segmentControl(disInfo.disTimer / 60);
#ifdef  ENGLISH_VOICE    
    voicePlay(SWITCH_DISINFECT_MODE, DELAY_DISINFECT_MODE);
    Delay(DELAY_DISINFECT_MODE);
#endif
  }
  
  //start
  checkStart();
#if 0
  if(g_remoteFlag & (REMOTE_OK_LONG_FLAG | REMOTE_OK_FLAG)) {
    g_remoteFlag = 0;
    isFirstEntrance = TRUE;
    currentState = STATE_DIS;
  }
  if(g_keyFlag & KEY_O3_FLAG) {
    g_keyFlag = 0;
    isFirstEntrance = TRUE;
    currentState = STATE_DIS;
  }
#endif
#ifdef  INCLUDE_CHECK_BASE
  checkBaseMode();
#else  
  //�ö�� ���
  if(g_remoteFlag & (REMOTE_STERILIZATION_FLAG | REMOTE_STERILIZATION_LONG_FLAG)) {
    g_remoteFlag = 0;
    voicePlay(SWITCH_KEY, DELAY_KEY);
    currentState = STATE_READY_STER;
    isFirstEntrance = TRUE;
  }
  if(g_keyFlag & KEY_STER_FLAG) {
    g_keyFlag = 0;
    voicePlay(SWITCH_KEY, DELAY_KEY);
    currentState = STATE_READY_STER;
    isFirstEntrance = TRUE;
  }  
  //���̿� ���
  if(g_remoteFlag & (REMOTE_TIMER_FLAG | REMOTE_TIMER_LONG_FLAG)) {
    g_remoteFlag = 0;
    voicePlay(SWITCH_KEY, DELAY_KEY);
    currentState = STATE_READY_ION;
    isFirstEntrance = TRUE;
  }
#endif  // INCLUDE_CHECK_BASE  
  //����
#ifdef  UNIFY_SOURCE
  checkPowerOff(STATE_POWER_OFF);
#else
  if(g_remoteFlag & (REMOTE_POWER_LONG_FLAG | REMOTE_POWER_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }

  if(g_keyFlag & KEY_PWR_FLAG)
  {
    g_keyFlag = 0; 
    
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
#endif  
  //�ð� ��
  if(g_remoteFlag & (REMOTE_UP_FLAG | REMOTE_UP_LONG_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    if(disInfo.disTimer < 60 * 60)
    {
      disInfo.disTimer += 5 * 60;  
    }
    
    disInfo.continuation = FALSE;
    
    segmentControl(disInfo.disTimer / 60);
  }
  
  //�ð� �ٿ�
  if(g_remoteFlag & (REMOTE_DOWN_FLAG | REMOTE_DOWN_LONG_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    if(disInfo.disTimer > 5 * 60)
    {
      disInfo.disTimer -= 5 * 60;
    }
    
    disInfo.continuation = FALSE;
    
    segmentControl(disInfo.disTimer / 60);
  }

  if(g_keyFlag & KEY_TIME_FLAG)
  {
    g_keyFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
      
    switch (disInfo.disTimer / 60)
    {
      case 30:
        disInfo.disTimer = 59*60;
        break;
      case 60:
        disInfo.disTimer = 30*60;        
        break;
      default :
        disInfo.disTimer = 30*60;
        break;
    }  
    
    disInfo.continuation = FALSE;
    
    segmentControl(disInfo.disTimer / 60);
  }
  
  if(g_remoteFlag & REMOTE_WIND_LONG_FLAG)
  {
    g_remoteFlag = 0;
    
    //continue
    disInfo.continuation = TRUE;
    
    segmentAlphaControl('O', 'n');
  }
}

//��ո��
void handleDisinfect()
{
  if(isFirstEntrance == TRUE)
  {
    printf("\r\n [ STATE_DIS : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    
    voicePlay(SWITCH_STERILIZATION_START, DELAY_STER_START);
    Delay(DELAY_STER_START);
    //LED display
    control_disLed();
    
    //relay control
    control_disRelayOn();

#ifdef  STAND_TYPE_ENABLE
    fanBlinkOn = TRUE;
    fanBlinkOnTimer = 0;
    fanBlinkOffTimer = 0;
#endif

  }
  
  //blink
  {
    if(disInfo.blinkOffFlag == TRUE)
    {
      ledControl(LED_DIS_ON, LED_ON);
      
      if(disInfo.continuation == FALSE)
        segmentControl(disInfo.disTimer / 60);
      else
        segmentAlphaControl('O', 'n');
      
      disInfo.blinkOffFlag = FALSE;
    }
    if(disInfo.blinkOnFlag == TRUE)
    {
      ledControl(LED_DIS_ON, LED_OFF);
      segmentOff();
      
      disInfo.blinkOnFlag = FALSE;
    }
  }
#ifdef  STAND_TYPE_ENABLE    
    if(fanBlinkOffFlag == TRUE)
    {
      relayControl(RELAY_AC_FAN1, RELAY_ON);
      fanBlinkOffFlag = FALSE;
//      RTC_TimeShow();
//      printf("\n\r FAN_1_ON : %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
    }
    
    if(fanBlinkOnFlag == TRUE)
    {
      relayControl(RELAY_AC_FAN1, RELAY_OFF);
      fanBlinkOnFlag = FALSE;        
//      RTC_TimeShow();
//      printf("\n\r FAN_1_OFF : %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
    }
#endif
  
  //Disinfect Stop
  if(g_remoteFlag & (REMOTE_OK_LONG_FLAG | REMOTE_OK_FLAG))
  {
    g_remoteFlag = 0;
    
    isFirstEntrance = TRUE;
    currentState = STATE_DIS_STOP;
  }

  if(g_keyFlag & KEY_O3_FLAG)
  {
    g_keyFlag = 0;
    
    isFirstEntrance = TRUE;
    currentState = STATE_DIS_STOP;
  }
  
  //time over
  if(disInfo.disTimer <= 0 && disInfo.continuation == FALSE)
  {
    systemWrite();
    
    currentState = STATE_READY_DIS;
    isFirstEntrance = TRUE;
  }
  
  //Power Off
#ifdef  UNIFY_SOURCE
  checkPowerOff(STATE_POWER_OFF);
#else
  if(g_remoteFlag & (REMOTE_POWER_LONG_FLAG | REMOTE_POWER_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
  

  if(g_keyFlag & KEY_PWR_FLAG)
  {
    g_keyFlag = 0; 
    
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }  
#endif  
#if 0  // SJM 190711 never used
  //Time update
  if(exMin != (disInfo.disTimer / 60))
  {
    exMin = disInfo.disTimer / 60;
  }
#endif
}

void handleDisinfectStop()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_DIS_STOP : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    
    //voice play
    voicePlay(SWITCH_STERILIZATION_STOP, DELAY_STER_STOP);
    Delay(DELAY_STER_STOP);
    
    //LED display
    control_disLed();
    
    //relay control
    control_relayAllOff();    // SJM 190711 instead of control_disRelayOff();
    systemWrite();
    
    //FND control
    if(disInfo.continuation == FALSE)
      segmentControl(disInfo.disTimer / 60);
    else
      segmentAlphaControl('O', 'n');
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
  
  //Power Off
#ifdef  UNIFY_SOURCE
  checkPowerOff(STATE_POWER_OFF);
#else
  if(g_remoteFlag & (REMOTE_POWER_LONG_FLAG | REMOTE_POWER_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
  
  if(g_keyFlag & KEY_PWR_FLAG)
  {
    g_keyFlag = 0; 
    
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }  
#endif
  //restart
  if(g_remoteFlag & (REMOTE_OK_LONG_FLAG | REMOTE_OK_FLAG))
  {
    g_remoteFlag = 0;
    
    isFirstEntrance = TRUE;
    currentState = STATE_DIS;
  }

  if(g_keyFlag & KEY_O3_FLAG)
  {
    g_keyFlag = 0;
    
    isFirstEntrance = TRUE;
    currentState = STATE_DIS;
  }
#ifdef  INCLUDE_CHECK_BASE
  checkBaseMode();
#else  
  //�ö�� ���
  if(g_remoteFlag & (REMOTE_STERILIZATION_FLAG | REMOTE_STERILIZATION_LONG_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    currentState = STATE_READY_STER;
    isFirstEntrance = TRUE;
  }

  if(g_keyFlag & KEY_STER_FLAG)
  {
    g_keyFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    currentState = STATE_READY_STER;
    isFirstEntrance = TRUE;
  }
  
  //���̿� ���
  if(g_remoteFlag & (REMOTE_TIMER_FLAG | REMOTE_TIMER_LONG_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    currentState = STATE_READY_ION;
    isFirstEntrance = TRUE;
  }
  
  //��� ���
  if(g_remoteFlag & (REMOTE_O3_FLAG | REMOTE_O3_LONG_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    currentState = STATE_READY_DIS;
    isFirstEntrance = TRUE;
  }
#endif  // INCLUDE_CHECK_BASE
}

//���̿� ���
void handleReadyIon()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_READY_ION : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    //LED control
    control_ionLed();
    //relay Control
    control_relayAllOff();
//    pidLEDOnFlag = FALSE;     SJM 190711 always assigned to 'FALSE' & never used
    //FND display
    ionInfo.ionTimer = 60 * 60;
    ionInfo.continuation = FALSE;
    segmentControl(ionInfo.ionTimer / 60);
#ifdef  ENGLISH_VOICE    
    voicePlay(SWITCH_ANION_MODE, DELAY_ANION_MODE);
    Delay(DELAY_ANION_MODE);
#endif
  }

#ifdef  INCLUDE_CHECK_BASE
  checkBaseMode();
#else  
  //�ö�� ���
  if(g_remoteFlag & (REMOTE_STERILIZATION_FLAG | REMOTE_STERILIZATION_LONG_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    currentState = STATE_READY_STER;
    isFirstEntrance = TRUE;
  }

  if(g_keyFlag & KEY_STER_FLAG)
  {
    g_keyFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    currentState = STATE_READY_STER;
    isFirstEntrance = TRUE;
  }
  
  //�ҵ� ���
  if(g_remoteFlag & (REMOTE_O3_FLAG | REMOTE_O3_LONG_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    currentState = STATE_READY_DIS;
    isFirstEntrance = TRUE;
  }

  if(g_keyFlag & KEY_O3_FLAG)
  {
    g_keyFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    currentState = STATE_READY_DIS;
    isFirstEntrance = TRUE;
  }
#endif  
  //start
  checkStart();
#if 0
  if(g_remoteFlag & (REMOTE_OK_LONG_FLAG | REMOTE_OK_FLAG))
  {
    g_remoteFlag = 0;
    
    isFirstEntrance = TRUE;
    currentState = STATE_ION;
  }
#endif
  //power off
#ifdef  UNIFY_SOURCE
  checkPowerOff(STATE_POWER_OFF);
#else
  if(g_remoteFlag & (REMOTE_POWER_LONG_FLAG | REMOTE_POWER_FLAG))
  {
    g_remoteFlag = 0; 
    
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }

  if(g_keyFlag & KEY_PWR_FLAG)
  {
    g_keyFlag = 0;
    
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
#endif  
  //time up
  if(g_remoteFlag & (REMOTE_UP_FLAG | REMOTE_UP_LONG_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    if(ionInfo.ionTimer < 95 * 60)
    {
      ionInfo.ionTimer += 5 * 60;  
    }
    
    ionInfo.continuation = FALSE;
    
    segmentControl(ionInfo.ionTimer / 60);
  }
  
  //time down
  if(g_remoteFlag & (REMOTE_DOWN_FLAG | REMOTE_DOWN_LONG_FLAG))
  {
    g_remoteFlag = 0;
    
    voicePlay(SWITCH_KEY, DELAY_KEY);
    
    if(ionInfo.ionTimer > 5 * 60)
    {
      ionInfo.ionTimer -= 5 * 60;  
    }
    
    ionInfo.continuation = FALSE;
    
    segmentControl(ionInfo.ionTimer / 60);
  }
  
  //time continue
  if(g_remoteFlag & (REMOTE_WIND_LONG_FLAG))
  {
    g_remoteFlag = 0;
    
    printf("continue mode!!\r\n");
    
    ionInfo.continuation = TRUE;
    
    segmentAlphaControl('O', 'n');
  }
  
  if(g_remoteFlag & (REMOTE_MENU_LONG_FLAG)) {
      g_remoteFlag = 0;
      changeState(STATE_SETUP_MODE);
//      currentState = STATE_ENGINEER_MODE;
//      isFirstEntrance = TRUE;
  }
}

void handleIon()
{
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_ION : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    //led control
    control_ionLed();
    //relay control
    control_IonOn();
    //FND control
    if(ionInfo.continuation == FALSE)
      segmentControl(ionInfo.ionTimer / 60);
    else
      segmentAlphaControl('O', 'n');
    voicePlay(SWITCH_ION_START, DELAY_ION_START);
    Delay(DELAY_ION_START);
#if 0  // SJM 190711 never used    
    exMin = ionInfo.ionTimer / 60;
#endif
#ifdef  STAND_TYPE_ENABLE
    fanBlinkOn = TRUE;
    fanBlinkOnTimer = 0;
    fanBlinkOffTimer = 0;
#endif
  }
  
  //blink
  {
    if(ionInfo.blinkOffFlag == TRUE) {
      ledControl(LED_ION_ON, LED_ON);
      if(ionInfo.continuation == FALSE)
        segmentControl(ionInfo.ionTimer / 60);
      else
        segmentAlphaControl('O', 'n');
      ionInfo.blinkOffFlag = FALSE;
    }
    if(ionInfo.blinkOnFlag == TRUE) {
      ledControl(LED_ION_ON, LED_OFF);
      segmentOff();
      ionInfo.blinkOnFlag = FALSE;
    }
  }

#ifdef  STAND_TYPE_ENABLE    
    if(fanBlinkOffFlag == TRUE) {
      relayControl(RELAY_AC_FAN1, RELAY_ON);
      fanBlinkOffFlag = FALSE;
//      RTC_TimeShow();
//      printf("\n\r FAN_1_ON : %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
    }
    if(fanBlinkOnFlag == TRUE) {
      relayControl(RELAY_AC_FAN1, RELAY_OFF);
      fanBlinkOnFlag = FALSE;        
//      RTC_TimeShow();
//      printf("\n\r FAN_1_OFF : %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
    }
#endif
  
  //time Over
  if(ionInfo.ionTimer <= 0 && ionInfo.continuation == FALSE) {
    systemWrite();
    currentState = STATE_READY_ION;
    isFirstEntrance = TRUE;
  }
  //ion Stop
  if(g_remoteFlag & (REMOTE_OK_LONG_FLAG | REMOTE_OK_FLAG)) {
    g_remoteFlag = 0;
    voicePlay(SWITCH_KEY, DELAY_KEY);
    isFirstEntrance = TRUE;
    currentState = STATE_ION_STOP;
  }
  //Power Off
#ifdef  UNIFY_SOURCE
  checkPowerOff(STATE_POWER_OFF);
#else
  if(g_remoteFlag & (REMOTE_POWER_LONG_FLAG | REMOTE_POWER_FLAG)) {
    g_remoteFlag = 0;
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
  if(g_keyFlag & KEY_PWR_FLAG) {
    g_keyFlag = 0; 
    voicePlay(SWITCH_POWER_OFF, DELAY_POWER_OFF);
    // SJM 190626 where is Delay??
    currentState = STATE_POWER_OFF;
    isFirstEntrance = TRUE;
  }
#endif  
#if 0  // SJM 190711 never used
  //Time update
  if(exMin != (ionInfo.ionTimer / 60))
  {
    exMin = ionInfo.ionTimer / 60;
  }
#endif
}

void handleIonStop()
{
  if(isFirstEntrance == TRUE)
  {
    printf("\r\n [ STATE_ION_STOP : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    
    voicePlay(SWITCH_ION_STOP, DELAY_ION_STOP);
    Delay(DELAY_ION_STOP);
    
    //led control
    control_ionLed(); 
    
    //relay control
    control_relayAllOff();    // SJM 190711 instead of control_IonOff();
    
    systemWrite();
    
    //FND control
    if(ionInfo.continuation == FALSE)
      segmentControl(ionInfo.ionTimer / 60);
    else
      segmentAlphaControl('O', 'n');
#if 0  // SJM 190711 never used
    exMin = ionInfo.ionTimer / 60;
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
  if(g_remoteFlag & (REMOTE_OK_LONG_FLAG | REMOTE_OK_FLAG))
  {
    g_remoteFlag = 0;
    
    isFirstEntrance = TRUE;
    currentState = STATE_ION;
  }

  checkBaseMode();
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
  static unsigned int ledStep, opMode;
//  static unsigned char ledStep;
  
  if(isFirstEntrance == TRUE) {
    printf("\r\n [ STATE_ENGINEER_MODE : %d ]\r\n", currentState);
    isFirstEntrance = FALSE;
    //led control
//    control_engineerLed();
    ledAllOff();
    //segment control
    segmentControl(SW_VERSION);
    //relay all off
    control_relayAllOff();
    ledStep = 0;
    opMode = 0;   // 0-relay on/Off
    for (port=0; port<MAX_RELAY_NUM; port++) onOff[port] = 0;
    port = 0;
    m_usbUpgrade = FALSE;
#ifdef  ENGLISH_VOICE    
    voicePlay(SWITCH_SETUP_MODE, DELAY_SETUP_MODE);
    Delay(DELAY_SETUP_MODE);
#endif
    printf("\r\n Engineer Mode\r\n");
  }
  
  switch (opMode) {
    case 0 :    // relay on/Off Test
      if (g_remoteFlag & (BUTTON_UP | BUTTON_UP_LONG) ) {
        g_remoteFlag = 0;
        ledControl(port+1,LED_OFF);
        port++;
        if (port>=MAX_RELAY_NUM) port = 0;
        ledControl(port+1,LED_ON);
      }
      else if (g_remoteFlag & (BUTTON_DOWN | BUTTON_DOWN_LONG) ) {
        g_remoteFlag = 0;
        ledControl(port+1,LED_OFF);
        if (port==0) port = MAX_RELAY_NUM-1;
        else          port--;
        ledControl(port+1,LED_ON);
      }
      else if (g_remoteFlag & (BUTTON_RIGHT | BUTTON_RIGHT_LONG) ) {
        g_remoteFlag = 0;
        onOff[port] ^= 1;
        relayControl(port+1,onOff[port]);
      }
      segmentControl((port+1)*10+onOff[port]);
      break;
    case 1 :    // sequential led on/off test
      if (halfSecFlag) {
        halfSecFlag = FALSE;
        printf("\r\n {Here!!! opMode = %d, ledStep = %d\r\n", opMode, ledStep);
        ledSequence(ledStep);
        ledStep++;
        if (ledStep>=97) ledStep = 0;
      }
      break;
  }

  if (g_remoteFlag & (BUTTON_TIME | BUTTON_TIME_LONG)) {
    g_remoteFlag = 0;
    opMode = 0;       // relay on/Off test
    printf("\r\n opMode1 = %d\r\n", opMode);
  }
  if (g_remoteFlag & (BUTTON_INTENSITY | BUTTON_INTENSITY_LONG)) {
    g_remoteFlag = 0;
    opMode = 1;       // 
    printf("\r\n opMode2 = %d\r\n", opMode);
  }
  // S/W upgrade!!!
  if(g_remoteFlag & (BUTTON_PERIOD_LONG)) {
    g_remoteFlag = 0;
    if(m_usbUpgrade == FALSE) {
      //voice play(KEY)
      voicePlay(SWITCH_KEY, DELAY_KEY);
      m_usbUpgrade = TRUE;  
    }
    else {
      //voice play(KEY)
      voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
      voicePlay(SWITCH_KEY, DELAY_KEY); Delay(DELAY_KEY);
      UpgradeEEPWrite(UpgradeEEPdata);
      NVIC_SystemReset();
    }
  }
  //power off
  checkPowerOff(STATE_POWER_OFF);
/*
#ifdef  CHECK_OZONE_LIMIT    
    CheckRoomTempEEPWrite(0x00);
#endif
*/ 
#if 0
#ifdef  CHECK_OZONE_LIMIT
  if(g_remoteFlag & (REMOTE_UP_FLAG | REMOTE_UP_LONG_FLAG)) {
    g_remoteFlag = 0;
    if( CheckRoomTempEEPRead() ) {
      segmentControl(11);
      Delay(1000);
    }
    else {
      segmentControl(00);
      Delay(1000);
    }
  }  
#endif

//      segmentControl( (unsigned int)(dOzoneSensoredValue) );
//      Delay(1000);
      
#ifdef  ADD_REMOTE_OZONE_SENSOR
    if( g_remoteFlag & (REMOTE_OZ1_FLAG | REMOTE_OZ2_FLAG | REMOTE_OZ3_FLAG | REMOTE_OZ4_FLAG | REMOTE_OZ5_FLAG) ) {
      switch(g_remoteFlag) {
        case REMOTE_OZ1_FLAG:
          segmentControl(05);
          RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
          printf("\r\n%0.2d:%0.2d:%0.2d, Remote Ozone Sensor Value = 05ppb", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);  
          (void)RTC->DR;  
          break;
        case REMOTE_OZ2_FLAG:
          segmentControl(15);
          RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
          printf("\r\n%0.2d:%0.2d:%0.2d, Remote Ozone Sensor Value = 15ppb", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);  
          (void)RTC->DR;            
          break;
        case REMOTE_OZ3_FLAG:
          segmentControl(25);
          RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
          printf("\r\n%0.2d:%0.2d:%0.2d, Remote Ozone Sensor Value = 25ppb", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);  
          (void)RTC->DR;            
          break;
        case REMOTE_OZ4_FLAG:
          segmentControl(35);
          RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
          printf("\r\n%0.2d:%0.2d:%0.2d, Remote Ozone Sensor Value = 35ppb", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);  
          (void)RTC->DR;
          break;
        case REMOTE_OZ5_FLAG:
          segmentControl(45);
          RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
          printf("\r\n%0.2d:%0.2d:%0.2d, Remote Ozone Sensor Value = 45ppb", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);  
          (void)RTC->DR;
          break;
        default:
          break;          
      }
      Delay(1000);
      g_remoteFlag = 0;
#ifdef  OZONE_DEPENDENT_DESTRUCTION     // SJM 190711 add '#ifdef' because of g_prvOzoneValue
      g_prvOzoneValue = FALSE;
#endif      
      segmentControl( (unsigned int)(dOzoneSensoredValue) );
      Delay(1000);
    }
    else if( g_remoteFlag & (REMOTE_OZ6_FLAG | REMOTE_OZ7_FLAG | REMOTE_OZ8_FLAG) ) {
      switch(g_remoteFlag) {
        case REMOTE_OZ6_FLAG:
          segmentControl(55);
          RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
          printf("\r\n%0.2d:%0.2d:%0.2d, Remote Ozone Sensor Value = 55ppb", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);  
          (void)RTC->DR;
          break;
        case REMOTE_OZ7_FLAG:
          segmentControl(65);
          RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
          printf("\r\n%0.2d:%0.2d:%0.2d, Remote Ozone Sensor Value = 65ppb", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);  
          (void)RTC->DR;
          break;
        case REMOTE_OZ8_FLAG:
          segmentControl(75);
          RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
          printf("\r\n%0.2d:%0.2d:%0.2d, Remote Ozone Sensor Value = 75ppb", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);  
          (void)RTC->DR;
          break;
        default:
          break;
      }
      Delay(1000);
      CheckRoomTempEEPWrite(CheckOZLimitEEPData);
      g_remoteFlag = 0;
#ifdef  OZONE_DEPENDENT_DESTRUCTION     // SJM 190711 add '#ifdef' because of g_prvOzoneValue
      g_prvOzoneValue = TRUE;
#endif
      // 
      segmentControl( (unsigned int)(dOzoneSensoredValue) );
      Delay(1000);
    }
#endif
#endif // if 0
}

#ifdef  INCLUDE_TIME_SETTING

void copyTime()
{
    RTC_TimeShow();
    RTC_GetTime(RTC_Format_BIN, &curTime);
    printf("\n\r  The current time (Hour-Minute-Second) is :  %0.2d:%0.2d:%0.2d \n\r", 
           curTime.RTC_Hours, curTime.RTC_Minutes, curTime.RTC_Seconds);
    tempHour = (unsigned char)curTime.RTC_Hours;
    tempMin = (unsigned char)curTime.RTC_Minutes;
    printf("\r\n tempTime = %02d:%02d",tempHour,tempMin);
}

void handleSetup()
{
  static unsigned char hourMin, settingMode;
  
  if (isFirstEntrance) {
    isFirstEntrance = FALSE;

//    ledStatus = 0;  
    settingMode = SET_TIME;
    hourMin = 0;    // 0 = hour mode, 1 = minute mode
    copyTime();
    tempRsvTime = plasmaInfo.rsvTime;
    tempRsvFlag = plasmaInfo.rsvOn;
    printf("[1]select rsvOn : %d\r\n", plasmaInfo.rsvOn);
    printf("[1]select rsvTime : %d\r\n", plasmaInfo.rsvTime);
    
    ledAllOff();
    ledControl(LED_POWER_SIG1,LED_ON);
    ledControl(LED_POWER_SIG2,LED_ON);
    ledControl(LED_SETTING_SIG1,LED_ON);
    ledControl(LED_SETTING_SIG2,LED_ON);
#ifdef  ENGLISH_VOICE    
    voicePlay(SWITCH_SETUP_MODE, DELAY_SETUP_MODE);
    Delay(DELAY_SETUP_MODE);
#endif  
  }
  
  if (halfSecFlag) {
    halfSecFlag = FALSE;
    ledStatus ^= 1;
    switch (settingMode) {
      case SET_PLASMA_RESERVATION :
        ledControl(LED_PLASMA_FONT, ledStatus);
        ledControl(LED_RESERVE_FONT,LED_OFF);
        break;
      case SET_TIME :
      default :
        ledControl(LED_PLASMA_FONT, LED_OFF);
        ledControl(LED_RESERVE_FONT,ledStatus);
        break;
    }
  }

  switch (settingMode) {
    case SET_PLASMA_RESERVATION :
      ledControl(LED_PLASMA_ON,tempRsvFlag);
      if (tempRsvFlag)  segmentControl(tempRsvTime);
      else              segmentAlphaControl('n','o');
      break;
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
  
  if ( g_remoteFlag&(BUTTON_CONFIRM|BUTTON_CONFIRM_LONG) ) {
    g_remoteFlag = 0;
    switch (settingMode) {
      case SET_PLASMA_RESERVATION :
        plasmaInfo.rsvOn = tempRsvFlag;
        plasmaInfo.rsvTime = tempRsvTime;  
      printf("[2]select rsvOn : %d\r\n", plasmaInfo.rsvOn);
      printf("[2]select rsvTime : %d\r\n", plasmaInfo.rsvTime);
        systemWrite();
        break;
      case SET_TIME :
      default :
        RTC_TimeShow();
        printf("\r\n tempTime = %02d:%02d",tempHour,tempMin);
        if (hourMin)  curTime.RTC_Minutes = tempMin;
        else          curTime.RTC_Hours = tempHour;
        printf("\n\r  The current time (Hour-Minute-Second) is :  %0.2d:%0.2d:%0.2d \n\r", 
               curTime.RTC_Hours, curTime.RTC_Minutes, curTime.RTC_Seconds);
        RTC_SetTime(RTC_Format_BIN,&curTime);
        RTC_TimeShow();
        break;
    }
    voicePlay(SWITCH_KEY, DELAY_KEY);
    Delay(DELAY_KEY);
  }
  if ( g_remoteFlag&(BUTTON_UP|BUTTON_UP_LONG) ) {
    g_remoteFlag = 0;
    switch (settingMode) {
      case SET_PLASMA_RESERVATION :
        tempRsvTime++;
        tempRsvTime = tempRsvTime%24;
        break;
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
  if ( g_remoteFlag&(BUTTON_DOWN|BUTTON_DOWN_LONG) ) {
    g_remoteFlag = 0;
    switch (settingMode) {
      case SET_PLASMA_RESERVATION :
        if (tempRsvTime==0) tempRsvTime = 23;
        else                tempRsvTime--;
        break;
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
  if ( g_remoteFlag&(BUTTON_RIGHT|BUTTON_RIGHT_LONG) ) {
    g_remoteFlag = 0;
    voicePlay(SWITCH_KEY, DELAY_KEY);
    Delay(DELAY_KEY);
    switch (settingMode) {
      case SET_PLASMA_RESERVATION :
        tempRsvFlag ^= 1;   // toggle On-Off
        break;
      case SET_TIME :
      default :
        hourMin ^= 1;   // toggle btw hour & min
        copyTime();     // refresh time from RTC
        ledControl(LED_RESERVE_ON,hourMin);
        break;
    }
  }
  if ( g_remoteFlag&(BUTTON_PLASMA|BUTTON_PLASMA_LONG) ) {
    g_remoteFlag = 0;
    settingMode = SET_PLASMA_RESERVATION;
    tempRsvTime = plasmaInfo.rsvTime;
    tempRsvFlag = plasmaInfo.rsvOn;
      printf("[3]select rsvOn : %d\r\n", plasmaInfo.rsvOn);
      printf("[3]select rsvTime : %d\r\n", plasmaInfo.rsvTime);
    voicePlay(SWITCH_KEY, DELAY_KEY);
    Delay(DELAY_KEY);
  }
  if ( g_remoteFlag&(BUTTON_TIME|BUTTON_TIME_LONG) ) {
    g_remoteFlag = 0;
    settingMode = SET_TIME;
    hourMin = 0;   // re-initialize
    copyTime();     // refresh time from RTC
    ledControl(LED_RESERVE_ON,hourMin);
    voicePlay(SWITCH_KEY, DELAY_KEY);
    Delay(DELAY_KEY);
  }

  if ( g_remoteFlag&(BUTTON_PERIOD|BUTTON_PERIOD_LONG) ) {
    g_remoteFlag = 0;
    voicePlay(SWITCH_KEY, DELAY_KEY);
    Delay(DELAY_KEY);
    changeState(STATE_ENGINEER_MODE);
  } 
  checkPowerOff(STATE_POWER_OFF);
}
#endif
void handler()
{
  switch(currentState)
  {
  case STATE_POWER_OFF:
    handlePowerOff();
    break;
  case STATE_INIT:
    handleInit();
    break;
  case STATE_READY_STER:
    handleReadySter();
    break;
  case STATE_STER:
    handleSter();
    break;
  case STATE_STER_STOP:
    handleSterStop();
    break;
  case STATE_READY_DIS:
    handleReadyDisinfect();
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
#if 0     // SJM 190716 remove STATE_STER_TIME_SET
  case STATE_STER_TIME_SET:
    handleSterTimeSet();
    break;
#endif
#ifdef  INCLUDE_BD_DEBUG
  case STATE_BD_DEBUG:
    handleBoardDebug();
    break;
#endif
#ifdef  INCLUDE_TIME_SETTING
  case STATE_SETUP_MODE:
//    handleTimeSetting();
    handleSetup();
    break;
#endif
  }
}
