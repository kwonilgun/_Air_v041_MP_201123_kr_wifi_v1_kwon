/**
  ******************************************************************************
  * @file    eeprom.h 
  * @author  sbKim
  * @version V1.0.0
  * @date    2015/05/21
  * @brief   Header for eeprom.h   module
  ******************************************************************************
**/
#ifndef __EEPROM_H
#define __EEPROM_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void systemWrite();
void systemRead();
void UpgradeEEPWrite(uint16_t data);

// 2024.6.11 : kwon 
int readVersionFromEeprom();
#endif /* __EEPROM_H */