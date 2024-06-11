#include "eeprom.h"
#include "24LC512.h"

#define sEE_WRITE_ADDRESS1        0x200
#define sEE_READ_ADDRESS1         0x200

void UpgradeEEPWrite(uint16_t data)
{
  uint8_t   buf[2];

  Delay(10);
  
  printf("data = %x \r\n", data);
  buf[0] = (data & 0xff00) >> 8;
  buf[1] = (data & 0x00ff);

  sEE_WriteBuffer(buf, UpgradeEEP, 2);
  sEE_WaitEepromStandbyState();
  Delay(100);
  sEE_WriteBuffer(buf, UpgradeEEP2, 2);
  sEE_WaitEepromStandbyState();
}

void systemWrite()
{
  unsigned char DataBuffer[SYSTEM_SIZE]; 
  
  sysConfig.plasmaInfo = plasmaInfo;
  
  printf("SYSTEM_SIZE : %d\r\n", SYSTEM_SIZE);
  memcpy(DataBuffer, &sysConfig, SYSTEM_SIZE);
  Delay(100);

  sEE_WriteBuffer(DataBuffer, sEE_WRITE_ADDRESS1, SYSTEM_SIZE);
  Delay(100);
  /* Wait for EEPROM standby state */
  sEE_WaitEepromStandbyState();
  Delay(100);
  
  printf("eeWrite!!\r\n");
}

void systemRead()
{
  unsigned char buffer[SYSTEM_SIZE];
  unsigned short NumDataRead = SYSTEM_SIZE;

  printf("System size : %d\r\n", NumDataRead);
  
  sEE_WaitEepromStandbyState();
  sEE_ReadBuffer(buffer, sEE_READ_ADDRESS1, &NumDataRead);
  Delay(100);
    
  memcpy(&sysConfig, buffer, SYSTEM_SIZE);
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  if ((sysConfig.ozoneLimit<OZONE_LIMIT_MIN)||(sysConfig.ozoneLimit>OZONE_LIMIT_MAX))
    sysConfig.ozoneLimit = OZONE_LIMIT_DEFAULT;
#endif
  if ((sysConfig.prevState<1)||(sysConfig.prevState>LAST_STATE))
    sysConfig.prevState = DEFAULT_STATE;
#ifdef  INCLUDE_RESERVATION_OPERATION_CONTROL
  if ((sysConfig.revOperMin<MIN_RESERVE_OPER_MIN)||(sysConfig.revOperMin>MAX_RESERVE_OPER_MIN))
    sysConfig.revOperMin = DEF_RESERVE_OPER_MIN;
#endif
#ifdef  HUNGARIAN_ISSUE
  if ((sysConfig.dutyRatio<MIN_DUTY_RATIO)||(sysConfig.dutyRatio>MAX_DUTY_RATIO))
    sysConfig.dutyRatio = DEF_DUTY_RATIO;
#endif

/* KWON :2024.6.11 modify, 플라즈마 동작 시에, systemRead()를 수행하면 시간이 리셋이 된다. 이것을 해결하고 기존 코드를 유지하기 위해서 변경
  if ((sysConfig.plasmaInfo.pidOn!=0)&&(sysConfig.plasmaInfo.pidOn!=1))
    sysConfig.plasmaInfo.pidOn = TRUE;
  if ((sysConfig.plasmaInfo.pwr<1)||(sysConfig.plasmaInfo.pwr>MAX_PLASMA_PWR))
    sysConfig.plasmaInfo.pwr = MAX_PLASMA_PWR;
  if ((sysConfig.plasmaInfo.rsvOn!=0)&&(sysConfig.plasmaInfo.rsvOn!=1))
    sysConfig.plasmaInfo.rsvOn = FALSE;
  if ((sysConfig.plasmaInfo.isScheduled!=0)&&(sysConfig.plasmaInfo.isScheduled!=1))
    sysConfig.plasmaInfo.isScheduled = FALSE;
  if ((sysConfig.plasmaInfo.rsvTime<0)||(sysConfig.plasmaInfo.rsvTime>23))
    sysConfig.plasmaInfo.rsvTime = 1;     // to avoid RTC Fail...


  plasmaInfo = sysConfig.plasmaInfo;
*/

  if ((sysConfig.plasmaInfo.pidOn!=0)&&(sysConfig.plasmaInfo.pidOn!=1))
    plasmaInfo.pidOn = TRUE;
  if ((sysConfig.plasmaInfo.pwr<1)||(sysConfig.plasmaInfo.pwr>MAX_PLASMA_PWR))
    plasmaInfo.pwr = MAX_PLASMA_PWR;
  if ((sysConfig.plasmaInfo.rsvOn!=0)&&(sysConfig.plasmaInfo.rsvOn!=1))
    plasmaInfo.rsvOn = FALSE;
  if ((sysConfig.plasmaInfo.isScheduled!=0)&&(sysConfig.plasmaInfo.isScheduled!=1))
    plasmaInfo.isScheduled = FALSE;
  if ((sysConfig.plasmaInfo.rsvTime<0)||(sysConfig.plasmaInfo.rsvTime>23))
    plasmaInfo.rsvTime = 1;     // to avoid RTC Fail...


  printf("modify eeread !!!\r\n");

  if (plasmaInfo.rsvOn != TRUE) {   // SJM 200415 initialize
    if (plasmaInfo.rsvOn != FALSE) {
      printf("\r\n Initialize : %d,%d\r\n", plasmaInfo.rsvOn, plasmaInfo.rsvTime);
      plasmaInfo.rsvOn =  FALSE;
      plasmaInfo.rsvTime = 12;
    }
    else {
      printf("\r\n Ozone reservation setting : %d,%d\r\n", plasmaInfo.rsvOn, plasmaInfo.rsvTime);
    }
  }

}

// 2024.6.11 : kwon , version number from eeprom
int readVersionFromEeprom() {
   unsigned char buffer[SYSTEM_SIZE];
  unsigned short NumDataRead = SYSTEM_SIZE;

  printf("System size : %d\r\n", NumDataRead);
  
  sEE_WaitEepromStandbyState();
  sEE_ReadBuffer(buffer, sEE_READ_ADDRESS1, &NumDataRead);
  Delay(100);
    
  memcpy(&sysConfig, buffer, SYSTEM_SIZE);

  return sysConfig.versionNumber;
}
