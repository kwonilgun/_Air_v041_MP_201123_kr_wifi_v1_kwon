#include "control.h"
#include "relay.h"
#include "LED.h"

extern unsigned int plasmaBlinkOnFlag;
extern unsigned int plasmaBlinkOffFlag;
extern unsigned char plasmaBlinkOn;

#ifdef  HUNGARIAN_ISSUE
unsigned char fanMode=4, genMode=1;
#endif
void control_relayAllOff()
{
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  for(int i = 0; i < 8; i++)
    relayControl(i + 1, RELAY_OFF);
//#else
#elif ( MACHINE == HPA_36C )
  relayControl(RELAY_AC_FAN1,RELAY_OFF);
  for(int i = 5; i < 9; i++)
    relayControl(i, RELAY_OFF);
#endif
}

#ifdef  INCLUDE_OZONE_CONTROL
void reduceOzoneLevel()
{
  // Turn Off Ozone Source
//#ifndef HPA_36C
  #if ( MACHINE == HPA_130W)
  relayControl(RELAY_OZONE_LAMP, RELAY_OFF);
  #endif
  relayControl(RELAY_PLASMA2, RELAY_OFF);
  relayControl(RELAY_PLASMA, RELAY_OFF);
  // Turn On OZone Destructor
//#ifndef HPA_36C
  #if ( MACHINE == HPA_130W)
  relayControl(RELAY_AC_UV, RELAY_ON);     //UV Lamp
  #endif
  // Leave Fan1 & Fan2 ON
}  
#endif // INCLUDE_OZONE_CONTROL
//�ö�� ���
void control_sterOn()
{  

  static unsigned char plate = 0;
  
  if (plasmaInfo.pidOn) {
    switch (plasmaInfo.pwr) {                       // SJM 190808 fall-through
      // SJM 200922 1<->3 change because lamp is stronger....
  #if (MACHINE==HPA_130W)
      case 3 :  relayControl(RELAY_OZONE_LAMP, RELAY_ON);      //OZONE Lamp
  #endif
      case 2 :  relayControl(RELAY_PLASMA2, RELAY_ON);          //PLASMA1
      case 1 :  relayControl(RELAY_PLASMA, RELAY_ON);         //PLASMA2
        break;
    }
  }
  else {
    if (plasmaInfo.pwr>=2) {
      relayControl(RELAY_PLASMA, RELAY_ON);
      relayControl(RELAY_PLASMA2, RELAY_ON);
    }
    else {
      if (plate)  relayControl(RELAY_PLASMA2, RELAY_ON);
      else        relayControl(RELAY_PLASMA, RELAY_ON);
      plate ^= 1;           // toggle plate
    }
  }
  relayControl(RELAY_AC_FAN1, RELAY_ON);          //FAN1  
  relayControl(RELAY_AC_FAN2, RELAY_ON);          //FAN2
  relayControl(RELAY_AC_UV, RELAY_OFF);     //UV Lamp
  

}

void control_sterOff()
{
#ifdef  HUNGARIAN_ISSUE
  relayControl(RELAY_PLASMA, RELAY_OFF);
  relayControl(RELAY_PLASMA2, RELAY_OFF);
  relayControl(RELAY_AC_FAN1, RELAY_ON);
  #if (MACHINE==HPA_130W)
  relayControl(RELAY_OZONE_LAMP, RELAY_OFF);
  relayControl(RELAY_AC_FAN2, RELAY_ON);
  #endif
#else
    relayControl(RELAY_PLASMA2, RELAY_OFF);         //PLASMA2
    relayControl(RELAY_PLASMA, RELAY_OFF);          //PLASMA1
  //  #ifndef HPA_36C
  #if ( MACHINE == HPA_130W)
    // SJM 200619 Do not On-Off Ozone Lamp
  //    relayControl(RELAY_OZONE_LAMP, RELAY_OFF);      //OZONE Lamp
    relayControl(RELAY_AC_UV, RELAY_OFF);     //UV Lamp
  #endif
#endif
}

void normalPlasmaSter()
{

      if (plasmaBlinkOnFlag == TRUE) {
 
        if ((plasmaInfo.pidOn==TRUE)&&(plasmaInfo.pwr==MAX_PLASMA_PWR)) {
          control_sterOn();
          //relayControl(RELAY_OZONE_LAMP, RELAY_OFF);       //OZONE Lamp
          // RTC_TimeShow();
          printf("\n\r\n\r control.c : Plasma --> Still ON : %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);          
        }
        else {
          control_sterOff();
          //relayControl(RELAY_OZONE_LAMP, RELAY_OFF);       //OZONE Lamp
          // RTC_TimeShow();

          printf("\n\r\n\r control.c Plasma --> OFF : %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);          
        }

        //kwon: 2024-5-6, timer int에서 이 값이 변경된다. 주기적으로 50초, 10초 간격 진행, plasmaInfo.pidOn가 true인경우는 계속해서 플라즈마가 켜진 상태로 진행이 된다. 

        plasmaBlinkOnFlag = FALSE;
      }
      if (plasmaBlinkOffFlag == TRUE) {
        printf("\n\r");
        control_sterOn();
        plasmaBlinkOffFlag = FALSE;
        // RTC_TimeShow();
        printf("\n\r\n\r control.c Plasma --> ON : %0.2d:%0.2d:%0.2d \n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
      }

}

void control_disRelayOn()
{
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  relayControl(RELAY_AC_FAN2, RELAY_ON);   //FAN2
  relayControl(RELAY_AC_UV, RELAY_ON);     //O3 UV
#endif
  relayControl(RELAY_RCI, RELAY_ON);       //RCI
  relayControl(RELAY_SPI, RELAY_ON);       //SPI
  relayControl(RELAY_AC_FAN1, RELAY_ON);          //FAN1  
}

void control_IonOn()
{
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  relayControl(RELAY_AC_FAN2, RELAY_ON);   //FAN2
  //relayControl(RELAY_AC_UV, RELAY_ON);     //O3 UV
#endif
  relayControl(RELAY_SPI, RELAY_ON);       //SPI
  relayControl(RELAY_AC_FAN1, RELAY_ON);          //FAN1  
}

void control_relayDestruction()
{
//#ifndef HPA_36C

  //2016.12.7 - BJ Kim requested : COMMENT
  //relayControl(RELAY_PLASMA, RELAY_ON);           //PLASMA1  
  relayControl(RELAY_AC_FAN2, RELAY_ON);   //FAN2
  // 2016.12.12 BJ Kim Requested Enable UV Lamp 
  relayControl(RELAY_AC_UV, RELAY_ON);     //UV Lamp
  relayControl(RELAY_AC_FAN1, RELAY_ON);   //FAN1  
}

void control_consumableCheckLed()
{
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  ledControl(LED_PLASMA_FONT, LED_ON);
  ledControl(LED_DIS_FONT, LED_ON);
  ledControl(LED_ION_FONT, LED_ON);
  ledControl(LED_RESERVE_FONT, LED_ON);
  ledControl(LED_POWER_FONT, LED_ON);
  ledControl(LED_PID_FONT, LED_ON);
  ledControl(LED_POWER_SIG1, LED_ON);
  ledControl(LED_POWER_SIG2, LED_ON);
  ledControl(LED_MODE_SIG1, LED_ON);
  ledControl(LED_MODE_SIG2, LED_ON);
  ledControl(LED_SETTING_SIG1, LED_ON);
  ledControl(LED_SETTING_SIG2, LED_ON);
  ledControl(LED_POWER_ON, LED_OFF);
#endif
  ledControl(LED_PLASMA_ON, LED_OFF);
  ledControl(LED_DIS_ON, LED_OFF);
  ledControl(LED_ION_ON, LED_OFF);
  ledControl(LED_RESERVE_ON, LED_OFF);
  ledControl(LED_PID_ON, LED_OFF);
}

void control_sterStartLedOn()
{
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  ledControl(LED_PLASMA_FONT, LED_ON);
  ledControl(LED_DIS_FONT, LED_ON);
  ledControl(LED_ION_FONT, LED_ON);
  ledControl(LED_RESERVE_FONT, LED_ON);
  ledControl(LED_POWER_FONT, LED_ON);
  ledControl(LED_PID_FONT, LED_ON);
  ledControl(LED_POWER_SIG1, LED_ON);
  ledControl(LED_POWER_SIG2, LED_ON);
  ledControl(LED_MODE_SIG1, LED_ON);
  ledControl(LED_MODE_SIG2, LED_ON);
  ledControl(LED_SETTING_SIG1, LED_ON);
  ledControl(LED_SETTING_SIG2, LED_ON);
  ledControl(LED_POWER_ON, LED_OFF);
#endif
  ledControl(LED_PLASMA_ON, LED_ON);
  ledControl(LED_DIS_ON, LED_OFF);
  ledControl(LED_ION_ON, LED_OFF);
  ledControl(LED_RESERVE_ON, LED_OFF);
  ledControl(LED_PID_ON, LED_OFF);
}

void control_sterStopLedOn()
{
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  ledControl(LED_PLASMA_FONT, LED_ON);
  ledControl(LED_DIS_FONT, LED_ON);
  ledControl(LED_ION_FONT, LED_ON);
  ledControl(LED_RESERVE_FONT, LED_ON);
  ledControl(LED_POWER_FONT, LED_ON);
  ledControl(LED_PID_FONT, LED_ON);
  ledControl(LED_POWER_SIG1, LED_ON);
  ledControl(LED_POWER_SIG2, LED_ON);
  ledControl(LED_MODE_SIG1, LED_ON);
  ledControl(LED_MODE_SIG2, LED_ON);
  ledControl(LED_SETTING_SIG1, LED_ON);
  ledControl(LED_SETTING_SIG2, LED_ON);
  ledControl(LED_POWER_ON, LED_OFF);
#endif
  ledControl(LED_PLASMA_ON, LED_ON);
  ledControl(LED_DIS_ON, LED_OFF);
  ledControl(LED_ION_ON, LED_OFF);
  ledControl(LED_RESERVE_ON, LED_OFF);
  ledControl(LED_PID_ON, LED_OFF);
}

void control_disLed()
{
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  ledControl(LED_PLASMA_FONT, LED_ON);
  ledControl(LED_DIS_FONT, LED_ON);
  ledControl(LED_ION_FONT, LED_ON);
  ledControl(LED_RESERVE_FONT, LED_ON);
  ledControl(LED_POWER_FONT, LED_ON);
  ledControl(LED_PID_FONT, LED_ON);
  ledControl(LED_POWER_SIG1, LED_ON);
  ledControl(LED_POWER_SIG2, LED_ON);
  ledControl(LED_MODE_SIG1, LED_ON);
  ledControl(LED_MODE_SIG2, LED_ON);
  ledControl(LED_SETTING_SIG1, LED_ON);
  ledControl(LED_SETTING_SIG2, LED_ON);
  ledControl(LED_POWER_ON, LED_OFF);
#endif
  ledControl(LED_PLASMA_ON, LED_OFF);
  ledControl(LED_DIS_ON, LED_ON);
  ledControl(LED_ION_ON, LED_OFF);
  ledControl(LED_RESERVE_ON, LED_OFF);
  ledControl(LED_PID_ON, LED_OFF);
}

void control_ionLed()
{
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  ledControl(LED_PLASMA_FONT, LED_ON);
  ledControl(LED_DIS_FONT, LED_ON);
  ledControl(LED_ION_FONT, LED_ON);
  ledControl(LED_RESERVE_FONT, LED_ON);
  ledControl(LED_POWER_FONT, LED_ON);
  ledControl(LED_PID_FONT, LED_ON);
  ledControl(LED_POWER_SIG1, LED_ON);
  ledControl(LED_POWER_SIG2, LED_ON);
  ledControl(LED_MODE_SIG1, LED_ON);
  ledControl(LED_MODE_SIG2, LED_ON);
  ledControl(LED_SETTING_SIG1, LED_ON);
  ledControl(LED_SETTING_SIG2, LED_ON);
  ledControl(LED_POWER_ON, LED_OFF);
#endif
  ledControl(LED_PLASMA_ON, LED_OFF);
  ledControl(LED_DIS_ON, LED_OFF);
  ledControl(LED_ION_ON, LED_ON);
  ledControl(LED_RESERVE_ON, LED_OFF);
  ledControl(LED_PID_ON, LED_OFF);
}

void control_desLed()
{
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  ledControl(LED_PLASMA_FONT, LED_ON);
  ledControl(LED_DIS_FONT, LED_ON);
  ledControl(LED_ION_FONT, LED_ON);
  ledControl(LED_RESERVE_FONT, LED_ON);
  ledControl(LED_POWER_FONT, LED_ON);
  ledControl(LED_PID_FONT, LED_ON);
  ledControl(LED_POWER_SIG1, LED_ON);
  ledControl(LED_POWER_SIG2, LED_ON);
  ledControl(LED_MODE_SIG1, LED_ON);
  ledControl(LED_MODE_SIG2, LED_ON);
  ledControl(LED_SETTING_SIG1, LED_ON);
  ledControl(LED_SETTING_SIG2, LED_ON);
  ledControl(LED_POWER_ON, LED_OFF);
#endif
  ledControl(LED_PLASMA_ON, LED_OFF);
  ledControl(LED_DIS_ON, LED_OFF);
  ledControl(LED_ION_ON, LED_OFF);
  ledControl(LED_RESERVE_ON, LED_OFF);
  ledControl(LED_PID_ON, LED_OFF);
}

void control_initLed()
{
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  ledControl(LED_PLASMA_FONT, LED_ON);
  Delay(50);
  ledControl(LED_DIS_FONT, LED_ON);
  Delay(50);
  ledControl(LED_ION_FONT, LED_ON);
  Delay(50);
  ledControl(LED_RESERVE_FONT, LED_ON);
  Delay(50);
  ledControl(LED_POWER_FONT, LED_ON);
  Delay(50);
  ledControl(LED_PID_FONT, LED_ON);
  Delay(50);
  ledControl(LED_POWER_SIG1, LED_ON);
  Delay(50);
  ledControl(LED_POWER_SIG2, LED_ON);
  Delay(50);
  ledControl(LED_MODE_SIG1, LED_ON);
  Delay(50);
  ledControl(LED_MODE_SIG2, LED_ON);
  Delay(50);
  ledControl(LED_SETTING_SIG1, LED_ON);
  Delay(50);
  ledControl(LED_SETTING_SIG2, LED_ON);
  Delay(50);
  ledControl(LED_POWER_ON, LED_ON);
  Delay(50);
#endif
  ledControl(LED_PLASMA_ON, LED_ON);
  Delay(50);
  ledControl(LED_DIS_ON, LED_ON);
  Delay(50);
  ledControl(LED_ION_ON, LED_ON);
  Delay(50);
  ledControl(LED_RESERVE_ON, LED_ON);
  Delay(50);
  ledControl(LED_PID_ON, LED_ON);
  Delay(300);
  
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  ledControl(LED_PLASMA_FONT, LED_OFF);
  Delay(50);
  ledControl(LED_DIS_FONT, LED_OFF);
  Delay(50);
  ledControl(LED_ION_FONT, LED_OFF);
  Delay(50);
  ledControl(LED_RESERVE_FONT, LED_OFF);
  Delay(50);
  ledControl(LED_POWER_FONT, LED_OFF);
  Delay(50);
  ledControl(LED_PID_FONT, LED_OFF);
  Delay(50);
  ledControl(LED_POWER_SIG1, LED_OFF);
  Delay(50);
  ledControl(LED_POWER_SIG2, LED_OFF);
  Delay(50);
  ledControl(LED_MODE_SIG1, LED_OFF);
  Delay(50);
  ledControl(LED_MODE_SIG2, LED_OFF);
  Delay(50);
  ledControl(LED_SETTING_SIG1, LED_OFF);
  Delay(50);
  ledControl(LED_SETTING_SIG2, LED_OFF);
  Delay(50);
  ledControl(LED_POWER_ON, LED_OFF);
  Delay(50);
#endif
  ledControl(LED_PLASMA_ON, LED_OFF);
  Delay(50);
  ledControl(LED_DIS_ON, LED_OFF);
  Delay(50);
  ledControl(LED_ION_ON, LED_OFF);
  Delay(50);
  ledControl(LED_RESERVE_ON, LED_OFF);
  Delay(50);
  ledControl(LED_PID_ON, LED_OFF);
  Delay(50);
}

void control_ledFont()
{
  ledAllOff();
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  ledControl(LED_PLASMA_FONT, LED_ON);
  ledControl(LED_DIS_FONT, LED_ON);
  ledControl(LED_ION_FONT, LED_ON);
  ledControl(LED_RESERVE_FONT, LED_ON);
  ledControl(LED_POWER_FONT, LED_ON);
  ledControl(LED_PID_FONT, LED_ON);
  ledControl(LED_POWER_SIG1, LED_ON);
  ledControl(LED_POWER_SIG2, LED_ON);
  ledControl(LED_MODE_SIG1, LED_ON);
  ledControl(LED_MODE_SIG2, LED_ON);
  ledControl(LED_SETTING_SIG1, LED_ON);
  ledControl(LED_SETTING_SIG2, LED_ON);
#endif
}

void control_engineerLed()
{
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  ledControl(LED_PLASMA_FONT, LED_ON);
  ledControl(LED_DIS_FONT, LED_ON);
  ledControl(LED_ION_FONT, LED_ON);
  ledControl(LED_RESERVE_FONT, LED_ON);
  ledControl(LED_POWER_FONT, LED_ON);
  ledControl(LED_PID_FONT, LED_ON);
  ledControl(LED_POWER_SIG1, LED_ON);
  ledControl(LED_POWER_SIG2, LED_ON);
  ledControl(LED_MODE_SIG1, LED_OFF);
  ledControl(LED_MODE_SIG2, LED_OFF);
  ledControl(LED_SETTING_SIG1, LED_OFF);
  ledControl(LED_SETTING_SIG2, LED_OFF);
  ledControl(LED_POWER_ON, LED_OFF);
#endif
  ledControl(LED_PLASMA_ON, LED_OFF);
  ledControl(LED_DIS_ON, LED_OFF);
  ledControl(LED_ION_ON, LED_OFF);
  ledControl(LED_RESERVE_ON, LED_OFF);
  ledControl(LED_PID_ON, LED_OFF);
}