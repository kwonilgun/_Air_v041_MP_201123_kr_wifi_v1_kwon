/**
  ******************************************************************************
  * @file    SysTick/SysTick_Example/main.h 
  * @author  
  * @version V1.0.0
  * @date    30-September-2011
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 HealthWell Medical</center></h2>
  ******************************************************************************  
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#define MULTI_LANGUAGE              
#define KOREAN                      0
#define ENGLISH                     1
#define JAPANESE                    2
#define LANGUAGE                    KOREAN

#define HPA_130W                    1
#define HPA_36C                     2
#define MONEY_STERILIZER            3
#define MACHINE                     HPA_130W

#define FALSE                       0
#define TRUE                        1
    
// kwon : 2024-4-5  
// SwitchInfo : iot Cloud send command data type
struct SwitchInfo {
    char switchType[20];
    char switchState[20];
};

// AWS iot cloud에서 보내온 명령 세트
struct IotCommandSet
{
    char start[10];
    char power[10];
    char mode[10];    //1-살균, 2-음이온, 3-프라즈마 
    char wind[10];  //1-약하게, 2-중간, 3-강하게
    char duration[10];

};


// ���� ���� ����
extern struct SwitchInfo switchInfo;
//extern struct IotCommandSet iotCommandSet;

// �Լ� ����
// void initializeSwitchInfo(const char *input);


/**** MACHINE Specific Definitions *******************************************/
#if (MACHINE == HPA_36C)
  #define MAX_PLASMA_PWR              2
  #define STER_INFO_DISP_CYCLE        2
  #define INCLUDE_BATTERY_CHECKER
    #ifdef  INCLUDE_BATTERY_CHECKER
      #define BATTERY_CHECK_TIME      1000    // SJM 200416 check battery every 1 sec.
      #define MAX_ADC_INPUT           3300.0  // SJM 200820 = V ref = 3.3V = 3300mV
      #define MAX_ADC_OUTPUT          4095    // SJM 200416 2^12 - 1 = 4096-1
      #define ADC_VOLTAGE             (MAX_ADC_INPUT/MAX_ADC_OUTPUT)
      #define COEF_SLOPE              0.164
      #define COEF_INTERCEPT          7.217
      #define SHUTDOWN_VOLTAGE        1300
      #define LOW_VOLTAGE             1400
      #define LOW_VOLTAGE_WARN        1480         
      #define SAFE_VOLTAGE_WARN       1560
      #define SAFE_VOLTAGE            1580
      #define LEVEL_SHUTDOWN_VOLTAGE    0
      #define LEVEL_LOW_VOLTAGE         1
      #define LEVEL_LOW_VOLTAGE_WARN    2
      #define LEVEL_SAFE_VOLTAGE_WARN   3
      #define LEVEL_SAFE_VOLTAGE        4
      #define MAX_LEVEL_VOLTAGE         5
    #endif
  #define INCLUDE_POWER_CONTROL
  #define INCLUDE_STOP_MODE
    #ifdef  INCLUDE_STOP_MODE
      #define WAIT_AT_POWER_OFF       60  //sec
      #define WAIT_AT_READY           300 //300 //sec
      #define TIME_TO_RESERVATION     3   //min
      #define EXCLUDE_SUPPLIED_POWER      // SJM do not enter STOP mode when ext Power is supplied.
    #endif
  #define LPP_V1_00_BD              // SJM 201014
    #ifdef  LPP_V1_00_BD
//      #define INCLUDE_CHARGE_CONTROL    // SJM 201020 removed by KBK
    #endif
//#define BATTERY_TEST  // SJM 201029 only for Battery Test. HPA-36C only
  #define MAX_CONSUMABLES           2
#elif (MACHINE == HPA_130W)
  #define MAX_PLASMA_PWR            3
  #define STER_INFO_DISP_CYCLE      3
  #define MAX_CONSUMABLES           4
//  #define ADD_REMOTE_OZONE_SENSOR
  #define HUNGARIAN_ISSUE
  #ifdef HUNGARIAN_ISSUE
    #define MAX_DUTY_RATIO            9
    #define MIN_DUTY_RATIO            1
    #define DEF_DUTY_RATIO            1
  #endif
#elif (MACHINE == MONEY_STERILIZER)
  #undef  INCLUDE_BATTERY_CHECKER
  #define INCLUDE_KEY_CONTROL   // SJM 200415
  #define MAX_STER_TIME             90
  #define MIN_STER_TIME             10
  #define DEF_STER_TIME             50
  #define DECOMP_TIME               10
#endif

/**** Common Definitions *******************************************/

#define ENABLE_DP
//  #define FIX_INTENSITY_CONTROL           // SJM 200904 fix bug & display -> 201117 Commonly Used
#define EXTEND_PIR_LAMP                 // SJM 200911 extend PIR lamp usage to easy to understand
#define USE_TNY_1311S_REMOTE              // SJM 200929
#define FIX_REMOTE_KEY                    // SJM 201115 usage of g_remoteFlag
#define INCLUDE_IWDG                      // SJM 201103 WatchDog ftn.

/********************** Modified by SJM **************************************/
#define SJM_DEBUG
#define PREPARATION_TIME         30   // SJM 190710 shorten for Test. original was 30sec.
#define USE_119SP_REMOTE              // SJM 190716 use RemoCon for OKC-119SP together.
#define INCLUDE_STRENGTHEN_OZONE      // SJM 200511 strengthen ozone upon JHK's request
#define INCLUDE_RETURN_TO_CONTINUOUS_MODE // SJM 191018 return to continuous operation
                                          //        after completing the reserved plasma sterilization
                                          //        according to DHKwon's request from ver 29-30
#define VOICE_FIRST                       // SJM 200421 play voice before relay control due to noise

#define INCLUDE_RESERVATION_OPERATION_CONTROL
  #ifdef INCLUDE_RESERVATION_OPERATION_CONTROL
    #define MIN_RESERVE_OPER_MIN      30
    #define MAX_RESERVE_OPER_MIN      240
    #define DEF_RESERVE_OPER_MIN      90
    #define DEL_RESERVE_OPER_MIN      30
  #endif

#define OZONE_TIME_TEST
  #ifdef OZONE_TIME_TEST
    #ifdef  BATTERY_TEST
      #define MAX_PLASMA_OP_TIME        240//590 7-segment limit to '9H' (<600)
    #else
      #define MAX_PLASMA_OP_TIME        240
    #endif
      #define MIN_PLASMA_OP_TIME         10
      #define DEL_PLASMA_OP_TIME         10
  #else
      #define MAX_PLASMA_OP_TIME         60
      #define MIN_PLASMA_OP_TIME          5
      #define DEL_PLASMA_OP_TIME          5
  #endif
/********************** Modified by SJM **************************************/

//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  //#define USE_100PPB_SENSOR             // SJM 190812 use 100ppb sensor instead of 250ppb
  #define USE_250PPB_SENSOR
  #define ADC_REFERENCE_3V              // SJM 200507 in case of Vref = 3V 

  #define INCLUDE_OZONE_CONTROL         // SJM 190808 add to control ozone concentration
  #ifdef  INCLUDE_OZONE_CONTROL
    #define OZONE_LIMIT_MAX             90
    #define OZONE_LIMIT_MIN             40
    #define OZONE_LIMIT_DEFAULT         50

    #define OZONE_SAFE_LEVEL            (sysConfig.ozoneLimit-10)
    #define OZONE_RISK_LEVEL            (sysConfig.ozoneLimit+20)
    #define OZONE_OVER_LIMIT          240  // SJM 200511 ozone over time, 240*0.5 = 120sec.
  #endif

//#define BASE_VERSION              31 
//#define BASE_VERSION              33      // SJM 200421 play voice before relay control due to noise 

//#define BASE_VERSION              35      // SJM 200506 
                                          // refer to  Version History.txt

//  #define BASE_VERSION              37      // SJM 200526 include reservation operating time
  #define INCLUDE_DIP_SWITCH            // SJM 200619 DIP SW was added since main B/D ver 2.1
//  #define SW_VERSION                40      // SJM 200902 Hungarian Issues
  #define SW_VERSION                41      // SJM 201117 Unification & USE_TNY_1311S_REMOTE

#else
//  #define BASE_VERSION              13    // SJM 200826 use RCI in Destruction Mode
                                          // INCLUDE_STOP_MODE etc.
  #define SW_VERSION              15    // SJM 201013 MULTI_LANGUAGE
  #define INCLUDE_DIP_SWITCH            // SJM 201013 DIP SW was added since main B/D 200929
                                        // currently it will affect readOzoneSensor()
                                        // which is not called/used in HPA-36C
#endif

//#define SAFETY_TEST                 // SJM 201015 for Ssfety Test
//#define EMC_TEST                    // SJM 201015 for EMC Test

#ifdef  SAFETY_TEST
  #undef  SW_VERSION
  #define SW_VERSION              98
#endif
#ifdef  EMC_TEST
  #undef  SW_VERSION
  #define SW_VERSION              97
#endif

#define SET_TIME                  0
#define SET_PLASMA_RESERVATION    1
#ifdef INCLUDE_RESERVATION_OPERATION_CONTROL
  #define SET_RESERVE_OPER_MIN    2
#endif
#ifdef HUNGARIAN_ISSUE
  #define SET_DUTY_RATIO          3
#endif

/* Handler Types  ------------------------------------------------------------*/
#define STATE_READY_STER            1
#define STATE_READY_DIS             2
#define STATE_READY_ION             3
#define STATE_INIT                  4
#define STATE_STER                  5
#define STATE_DIS                   6
#define STATE_ION                   7
#define STATE_POWER_OFF             8
#define STATE_STER_STOP             9
#define STATE_DIS_STOP              10
#define STATE_ION_STOP              11
#define STATE_CONSUMABLE_WARNING    12
#define STATE_DESTRUCTION           13
#define STATE_PREPARE               14
#define STATE_SETUP_MODE            15
#define STATE_ENGINEER_MODE         16

//kwon: 2024-4-11: serial 입력 state
#define STATE_SERIAL_READY_DIS      30

/*
#if ( MACHINE == MONEY_STERILIZER )
  #define STATE_MONEY_STANDBY       17
  #define STATE_MONEY_STER          18
  #define STATE_MONEY_STOP          19
  #define STATE_MONEY_DECOMP        20

  #define LAST_STATE                20      // SJM 190703 it should revised when new State added.
  #define DEFAULT_STATE             STATE_MONEY_STANDBY   // SJM 190723 for prevState
*/
#ifdef USE_TNY_1311S_REMOTE
  #define STATE_CONCENTRATION       17
  #define STATE_TIME_SETUP          18
  #define STATE_ENGINEER_SETUP      19
  #define STATE_BOOK_TIME           20
  #define STATE_BOOK_CONFIG         21

  #define LAST_STATE                40      // SJM 190703 it should revised when new State added.
  #define DEFAULT_STATE             STATE_READY_DIS   // SJM 190723 for prevState
#else
  #define LAST_STATE                16      // SJM 190703 it should revised when new State added.
  #define DEFAULT_STATE             STATE_READY_DIS   // SJM 190723 for prevState
#endif

/* Consumables Define ------------------------------------------------------------*/
/* 2016.12.13. DH Kwon Requested  */
//#define CHANGE_OZONE_LAMP     4000
//#define CHANGE_UV_LAMP        4000
//#define CHANGE_FILTER         4000
// SJM 190723 commented 
//  #define CHANGE_OZONE_LAMP     25000
//  #define CHANGE_UV_LAMP        8000

//#ifndef HPA_36C  // SJM 200413 No UV-Lamp and Ozpne-Lamp in HPA_36C
#if ( MACHINE == HPA_130W)
  #define CHANGE_OZONE_LAMP    480000            // SJM 190722 OzoneLamp  8000Hx60M
  #define CHANGE_UV_LAMP       480000            // SJM 190722 UvLamp     8000Hx60M
#endif
#define CHANGE_RCI_CELL     1500000            // SJM 190722 RCIcell   25000Hx60M
#define CHANGE_FILTER        600000            // SJM 190723 Filter    10000Hx60M (DH Kwon request)

//#ifndef HPA_36C  // SJM 200413 No UV-Lamp and Ozpne-Lamp in HPA_36C
#if ( MACHINE == HPA_130W)
  /*2016.12.13. DH Kwon Requested
  #define WARNING_NUMBER        3
  */
  //#define WARNING_NUMBER        3           // SJM 190722 add RCIcell as a consumable
  #define WARNING_NUMBER        4           // SJM 190723 add Filter as a consumable again

  #define WARNING_OZONE_LAMP    0x0001
  #define WARNING_UV_LAMP       0x0002
  #define WARNING_RCI_CELL    0x0004
  #define WARNING_FILTER      0x0008        // SJM 190723 add Filter as a consumable again as DH Kwon Requested
  //#define WARNING_FILTER        0x0004        2016.12.13. DH Kwon Requested
//#else // HPA_36C
#elif ( MACHINE == HPA_36C)
  #define WARNING_NUMBER        2           // SJM 190723 add Filter as a consumable again

  #define WARNING_RCI_CELL    0x0004
  #define WARNING_FILTER      0x0008        // SJM 190723 add Filter as a consumable again as DH Kwon Requested
#endif

#ifdef  INCLUDE_STRENGTHEN_OZONE
  #define PLASMA_BLINK_ON_TIME    50    //50sec
  #define PLASMA_BLINK_OFF_TIME   10    //10sec
#else
  #define PLASMA_BLINK_ON_TIME    30    //30sec
  #define PLASMA_BLINK_OFF_TIME   30    //30sec
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm324xg_eval.h"
#include "Peripheral.h"
#include "24LC512.h"
#include "Key.h"
#include "relay.h"
#include "RTC.h"
#include "LED.h"
#include "remote.h"
#include "isd1760.h"
#include "pir.h"
#include "eeprom.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct {
  unsigned char pidOn;
  unsigned char pwr;
  unsigned char rsvOn;
  unsigned char isScheduled;        // SJM 190724 added to replace 'Mode'. 1=scheduled.
  int rsvTime;
//  unsigned int modeSelect;        // SJM 190715 useless??
  unsigned int blinkOnTimer;
  unsigned int blinkOffTimer;
  unsigned char blinkOnFlag;
  unsigned char blinkOffFlag;
//  unsigned int Mode;              // SJM 190724 replaced by 'isScheduled
  unsigned int plasmaOneSec;
  unsigned int plasmaTimer;
  unsigned char continuation;
}PLASMAINFO;

typedef struct {
  unsigned int blinkOnTimer;
  unsigned int blinkOffTimer;
  unsigned char blinkOnFlag;
  unsigned char blinkOffFlag;
  unsigned int disOneSec;
  unsigned int disTimer;
  unsigned char continuation;
}DISINFO;

typedef struct {
  unsigned int blinkOnTimer;
  unsigned int blinkOffTimer;
  unsigned char blinkOnFlag;
  unsigned char blinkOffFlag;
  unsigned int ionOneSec;
  unsigned int ionTimer;
  unsigned char continuation;
}IONINFO;

typedef struct {
  unsigned int rciOperatingMin;           //minute
//  unsigned int totalOperatingMin;         //Minite
//#ifndef HPA_36C  // SJM 200413 No UV-Lamp and Ozpne-Lamp in HPA_36C  
//#if ( MACHINE == HPA_130W)
  unsigned int uvLampCountMin;            //Minite
  unsigned int ozoneLampCountMin;         //Minite
//#endif
  unsigned int filterCountMin;            //Minite
  unsigned int prevState;
  unsigned int blinkOnTimer;
  unsigned int blinkOffTimer;
  unsigned char blinkOnFlag;
  unsigned char blinkOffFlag;
//  unsigned int disOneSec;             never used.
  PLASMAINFO plasmaInfo;  
  unsigned int  ozoneLimit;
//#ifdef  INCLUDE_RESERVATION_OPERATION_CONTROL
  unsigned int  revOperMin;
//#ifdef  HUNGARIAN_ISSUE
  unsigned short dutyRatio;             // SJM 200921 added 1~9
//#endif
//#ifdef  INCLUDE_STOP_MODE
  unsigned char stopped;
//#endif
#if ( MACHINE == MONEY_STERILIZER )
  unsigned short sterTime;
  unsigned short decompTime;
#endif
}SYSTEMINFO;
//#ifndef HPA_36C
#if ( MACHINE != HPA_36C)
  #define     UpgradeEEP           61110
  #define     UpgradeEEP2          31110
  #define     UpgradeEEPdata       0xa55a
#elif ( MACHINE == HPA_36C)
  #define     UpgradeEEP           0xFF10
  #define     UpgradeEEP2          0x0010
  #define     UpgradeEEPdata       0xa55a
#endif
    
/* Global define ------------------------------------------------------------*/
#define MAX_RX_BUFFER             50
#define SYSTEM_SIZE               sizeof(SYSTEMINFO)
#define ONESEC                    1000
//#define FIVESEC                   5000      SJM190724 currently NoUse
#define MAX_SIZE_PRINT            500

extern unsigned int g_keyFlag;

extern SYSTEMINFO sysConfig;
extern PLASMAINFO plasmaInfo;
extern DISINFO disInfo;
extern IONINFO ionInfo;

extern RTC_TimeTypeDef RTC_TimeStructure;
extern RTC_InitTypeDef RTC_InitStructure;
extern RTC_DateTypeDef RTC_DateStructure;
extern RTC_TimeTypeDef RTC_TimeStampStructure;
extern RTC_DateTypeDef RTC_TimeStampDateStructure;

extern unsigned char g_volume;        // SJM 190716 added by SJM. originally it was 15(constant)

/* Exported constants --------------------------------------------------------*/
  //======================================================================================================================
  // Voice Track(��������� Ʈ����ȣ)
  //======================================================================================================================
  #define VOICE_POWERON_START                 0x10		
  #define VOICE_POWERON_END                   0x2D		
  #define VOICE_POWER_OFF_START               0x2E           
  #define VOICE_POWER_OFF_END                 0x3B
  #define VOICE_KEY_START                     0x3C          
  #define VOICE_KEY_END                       0x40

  #define DELAY_POWERON                       3643        
  #define DELAY_POWER_OFF                     1654
  #define DELAY_KEY                           523

#if (LANGUAGE == KOREAN)
  #include "voice_korean.h"
#elif (LANGUAGE == ENGLISH)
  #include "voice_english.h"
#elif (LANGUAGE == JAPANESE)
  #include "voice_japanese.h"
#endif
//======================================================================================================================
// Voice Switch
//======================================================================================================================
#define SWITCH_POWER_ON                     1
#define SWITCH_POWER_OFF                    2
#define SWITCH_KEY                          3
#define SWITCH_PLASMA_MODE                  4
#define SWITCH_DISINFECT_MODE               5
#define SWITCH_ANION_MODE                   6
#define SWITCH_SETUP_MODE                   7
#define SWITCH_PLASMA_START                 8
#define SWITCH_PREPARE                      9
#define SWITCH_PLASMA_STOP                  10
#define SWITCH_STERILIZATION_START          11
#define SWITCH_STERILIZATION_STOP           12
#define SWITCH_ION_START                    13
#define SWITCH_ION_STOP                     14
#define SWITCH_DESTRUCTION_START            15
#define SWITCH_DESTRUCTION_STOP             16
#define SWITCH_CONSUMABLE_CHECK             17
#define SWITCH_PIR_DETECT                   18
#define SWITCH_OZONE_DETECT                 19
#define SWITCH_ENGINEER_MODE                20

/* Exported functions ------------------------------------------------------- */

void TimingDelay_Decrement(void);
uint32_t sEE_TIMEOUT_UserCallback(void);
void Delay(__IO uint32_t nTime);
#endif /* __MAIN_H */

/******************* (C) COPYRIGHT 2011 HealthWell *****END OF FILE****/
