
#include "Shell.h"
#include "eeprom.h"

#ifdef  SJM_DEBUG
unsigned long ttt1=0,ttt2=50, ttt3=0;
#endif
extern unsigned char m_data[4];

#ifdef  INCLUDE_DIP_SWITCH
  extern unsigned char statusDIP1;                   // SJM 200619 default value = 1;
#endif
#ifdef  INCLUDE_STOP_MODE
  extern unsigned char stopFlag;
  extern unsigned char intCounter;
  extern unsigned short idleTimeSec;
#endif


int fputc(int ch, FILE *f)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART3, (uint8_t) ch);
  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET) {}
  return ch;
}

unsigned int getDigit(char digit)
{
    int value=0;

    if (digit>=0x30 && digit<=0x39)         value = digit-0x30;
    else {
        digit &= 0xdf;
        if ( digit >= 'A' && digit <= 'F')  value = digit-'A'+10;
    }
    return value&0xf;
}

int CheckValue(char *str)
{
    unsigned int value=0;
    int length,i;
    char *pdigit;
    
    if (strncmp(str,"0x",2)==0) {
        pdigit = &str[2];
        length = strlen(pdigit);
        value = 0;
        for (i=1;i<=length;i++) {
            value += getDigit(*pdigit++)<<((length-i)*4);
        }
    }
    else {
        length = strlen(str);
        value = atoi(str);
    }
    return value;
}

extern unsigned int m_count;
extern unsigned int m_remoteState;
extern void copyTime();
#ifdef  SJM_DEBUG
extern void voicePlay2(unsigned int voice);     // SJM 190617 for test
extern void readRemote();
extern unsigned char pirPort1, pirPort2;
#if ( MACHINE == HPA_130W)
extern unsigned int dOzoneSensoredValue;
extern unsigned int g_RemoteOzoneSensorValue;
#endif
extern int g_voicePortDelay;
extern unsigned int voicePlayFlag;
extern unsigned int currentState, isFirstEntrance;
#endif
#ifdef  HUNGARIAN_ISSUE
extern unsigned int plasmaOnTime;
extern unsigned int plasmaOffTime;
extern unsigned char fanMode, genMode;
#endif


void command_exec()
{
    char *opcode=pcommand[0];
    int value, value2;    
#ifdef  SJM_DEBUG
    unsigned int endAddr, delay;
#endif
    volatile unsigned int flashVal, reVal;
#ifdef  INCLUDE_DIP_SWITCH
    unsigned char sw1, sw2, sw3;
#endif

//#ifdef  HPA_36C
#if ( MACHINE == HPA_130W)
    if (strcmp(opcode,"chkPwr")==0) {
      value = GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5);
      printf("\r\n Power = %dV\r\n",value);
    }
#endif


#ifdef  HUNGARIAN_ISSUE
    if (strcmp(opcode,"setOnTime")==0) {
      value = CheckValue(pcommand[1]);
      plasmaOnTime = value;
      printf("plasmaOnTime = %d\r\n", plasmaOnTime);
    }
    else if (strcmp(opcode,"setOffTime")==0) {
      value = CheckValue(pcommand[1]);
      plasmaOffTime = value;
      printf("plasmaOffTime = %d\r\n", plasmaOffTime);
    }
    else if (strcmp(opcode,"fanMode")==0) {
      value = CheckValue(pcommand[1]);
      if ((value>=1)&&(value<=4)) {
        fanMode = value;
        printf("fanMode = %d\r\n", fanMode);
      }
      else
        printf("[Usage] fanMode = 1~4\r\n");
    }
    else if (strcmp(opcode,"genMode")==0) {
      value = CheckValue(pcommand[1]);
      if ((value>=1)&&(value<=3)) {
        genMode = value;
        printf("genMode = %d\r\n", genMode);
      }
      else
        printf("[Usage] genMode = 1~3\r\n");
    }
    else if (strcmp(opcode,"helpPlasma")==0) {
      printf("\r\n plasmaOnTime = %d, plasmaOffTime = %d\r\n",plasmaOnTime,plasmaOffTime);
      printf(" fanMode = %d,  genMode = %d\r\n", fanMode,genMode);
    }
#endif
    if (strcmp(opcode,"help")==0) {
		printf("hello, Uart Shell Program!! \r\n");
    }
#ifdef  INCLUDE_DIP_SWITCH
    else if (strcmp(opcode,"getSW")==0) {
      sw1 = GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_8);
      sw2 = GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9);
      sw3 = GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10);
      printf("\r\n [S/W state = %d %d %d, DIP1 = %d\r\n",sw1,sw2,sw3, statusDIP1);
    }
#endif
#ifdef  LPP_V1_00_BD
    else if (strcmp(opcode,"getBat")==0) {
      sw1 = GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_3);
      printf("\r\n [Bat installed = %d]\r\n",sw1);
    }
  #ifdef  INCLUDE_CHARGE_CONTROL
    else if (strcmp(opcode,"setBat")==0) {
      value = CheckValue(pcommand[1]);
      if ((value==0)||(value==1)) {
        if (value==1) {
          GPIO_WriteBit(GPIOD,GPIO_Pin_4,Bit_RESET);
          printf("\r\n RESET!!");
        }
        else {
          GPIO_WriteBit(GPIOD,GPIO_Pin_4,Bit_SET);
          printf("\r\n SET!!");
        }
        chargeControl = value;
        printf(" [setBat = %d]\r\n",value);
      }
      else
        printf("\r\n [Usage] setBat 0/1\r\n");
    }
  #endif  // INCLUDE_CHARGE_CONTROL
#endif  // LPP_V1_00_BD
    if (strcmp(opcode,"eetest")==0) {	
      printf("eeprom test\r\n");
      value = CheckValue(pcommand[1]);
      value2 = CheckValue(pcommand[2]);

      systemWrite();
      systemRead();
    }
    if (strcmp(opcode,"chgState")==0) {
      value = CheckValue(pcommand[1]);
      value2 = CheckValue(pcommand[2]);
      if (value2 !=1) value2 = 0;
      if ((value<=0)||(value>LAST_STATE))
        printf("\r\n[ Usage ] chgState N 0/1(N=1~%d)\r\n", value);
      else
        changeState(value,value2);
    }
    else if (strcmp(opcode,"shState")==0) {
      printf("\r\n[ Current State = %d, entrance = %d ]", currentState, isFirstEntrance);
    }
    
    if (strcmp(opcode,"relay")==0) {	
      value = CheckValue(pcommand[1]);
      value2 = CheckValue(pcommand[2]);
      relayControl(value, value2);
    }

//========= LED Test ==============================    
    if (strcmp(opcode,"ledtest")==0) {	
      printf("ledTest!\r\n");
      ledTest();
    }    
    if (strcmp(opcode,"led")==0) {	
      printf("led!\r\n");
      value = CheckValue(pcommand[1]);
      value2 = CheckValue(pcommand[2]);
      ledControl(value, value2);
    }
    if (strcmp(opcode,"ledcom")==0) {	
      printf("ledcom!\r\n");
      value = CheckValue(pcommand[1]);
      value2 = CheckValue(pcommand[2]);
      ledComControl(value, value2);
    }
    if(strcmp(opcode, "ledon") == 0)
    {
      printf("led all On\r\n");  
      G1_SF = 0xff;
      G2_SF = 0xff;
      G3_SF = 0xff;
      G4_SF = 0xff;
      G5_SF = 0xff;
    }
    if(strcmp(opcode, "ledoff") == 0)
    {
      printf("led all Off\r\n");  
      G1_SF = 0;
      G2_SF = 0;
      G3_SF = 0;
      G4_SF = 0;
      G5_SF = 0;
    }
    if (strcmp(opcode,"seg")==0) {	
      printf("seg!\r\n");
      value = CheckValue(pcommand[1]);
      segmentControl(value);
    }
#ifdef  SJM_DEBUG
    if (strcmp(opcode,"segA")==0) {	// SJM 190617 for test
      printf("segAlpha %s\r\n",pcommand[1]);
      segmentAlphaControl(pcommand[1][0],pcommand[1][1]);
    }
    if (strcmp(opcode, "testUI")==0) {
      printf("\r\n ttt1 = %u(%x), ttt2 = %u(%x), ttt3 = %u(%x)", ttt1, ttt1, ttt2,ttt2, ttt3, ttt3);
      ttt1--;
      ttt3 = ttt1-ttt2;
      printf("\r\n ttt1 = %u(%x), ttt2 = %u(%x), ttt3 = %u(%x)", ttt1, ttt1, ttt2,ttt2, ttt3, ttt3);
      ttt2 = ttt1 + 100;
      ttt3 = ttt2 - ttt1;
      printf("\r\n ttt1 = %u(%x), ttt2 = %u(%x), ttt3 = %u(%x)", ttt1, ttt1, ttt2,ttt2, ttt3, ttt3);
    }
#endif      
//========= Voice Test ==============================    
    if (strcmp(opcode,"vvtest")==0) {
      printf("spi\r\n");
      ISD1760_powerUp();
      Delay(10);
      value = ISD1760_readStatus();
      printf("value : 0x%x", value);
    }
    if (strcmp(opcode,"stop")==0) {
      printf("spi\r\n");
      ISD1760_stop();
      Delay(10);
      value = ISD1760_readStatus();
      printf("value : 0x%x", value);
    }
#ifdef  SJM_DEBUG
    if (strcmp(opcode,"vp")==0) {          // SJM 190621 for voice setting
      value = CheckValue(pcommand[1]);
      if ((value <= 0)|| (value > 20))
        printf("[Usage : vp voice# start end delay ]\r\n");
      else {
        value2 = CheckValue(pcommand[2]);
        endAddr = CheckValue(pcommand[3]);
        delay = CheckValue(pcommand[4]);
        printf("VoicePlay %d %d[0x%x] %d[0x%x] %d\r\n", value, value2, value2, endAddr, endAddr, delay);
        
        g_voicePortDelay = delay;
        printf("g_voicePortDelay : %d\r\n", g_voicePortDelay);
        Delay(10);
        voicePlayFlag = TRUE;
        ISD1760_setPlay(value2, endAddr, 15);
      }
    }
    if (strcmp(opcode,"vplay")==0) {      // SJM 190617 for test
      value = CheckValue(pcommand[1]);
      printf("voice Play=%d!\r\n",value);
      voicePlay2(value);
    }
    if (strcmp(opcode,"setVol")==0) {
      value = CheckValue(pcommand[1]);
      g_volume = (unsigned char)value;
      printf("set volume = %d(%04x)!\r\n",g_volume,g_volume);
    }

    else if (strcmp(opcode,"chkPir")==0) {      // SJM 190617 for test
      printf("pirPort1 = %d , pirPort2 = %d\r\n",pirPort1, pirPort2);
    }
    else if (strcmp(opcode,"testPir")==0) {      // SJM 190617 for test
      value = getPirPort1();
      value2 = getPirPort2();
      printf("pirPort1 = %d , pirPort2 = %d, getPort1 = %d, getPort2 = %d\r\n",pirPort1, pirPort2, value, value2);
    }
//  #ifndef HPA_36C
  #if ( MACHINE == HPA_130W)
    else if (strcmp(opcode,"chkOzone")==0) {      // SJM 190617 for test
      printf("Local = %d , Remote = %d\r\n",dOzoneSensoredValue, g_RemoteOzoneSensorValue);
    }
    else if (strcmp(opcode,"testOzone")==0) {      // SJM 190617 for test
      readRemote();
      printf("Local = %d , Remote = %d\r\n",dOzoneSensoredValue, g_RemoteOzoneSensorValue);
    }
  #endif
    if (strcmp(opcode,"keyCode")==0) {      // SJM 190617 for test
      printf("Received = 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n",m_data[0],m_data[1],m_data[2],m_data[3]);
    }
#endif
    
    if(strcmp(opcode, "state") == 0){
      printf("state : %d, count : %d\r\n", m_remoteState, m_count); 
      m_remoteState = 0;
      m_count = 0;
    }
    if(strcmp(opcode, "ptest") == 0) {
      printf("print On\r\n");  
      printfTest = TRUE;
    }
    if(strcmp(opcode, "1690") == 0) {
      printf("setting!\r\n");
      settingMainMenu();  
    }
    if(strcmp(opcode, "time") == 0) {
      printf("time!\r\n");
      RTC_TimeShow();  
    }
    if(strcmp(opcode, "copytime") == 0)  copyTime();  
    if(strcmp(opcode, "date") == 0) {
      printf("date!\r\n");
      RTC_DateShow();  
    }
    if (strcmp(opcode,"settime")==0) {	
      printf("setting Time\r\n");
      RTC_TimeRegulate();
    }
//#ifndef HPA_36C  // SJM 200413 No UV-Lamp and Ozpne-Lamp in HPA_36C
#if ( MACHINE == HPA_130W )
    if(strcmp(opcode, "uv") == 0) {
      value = CheckValue(pcommand[1]);
      printf("uvlamp : %d\r\n", value);
      sysConfig.uvLampCountMin = value;
      systemWrite();
    }
    if(strcmp(opcode, "ozone") == 0) {
      value = CheckValue(pcommand[1]);
      printf("ozone : %d\r\n", value);
      sysConfig.ozoneLampCountMin = value;
      systemWrite();
    }
#endif
    if(strcmp(opcode, "filter") == 0) {
      value = CheckValue(pcommand[1]);
      printf("filter : %d\r\n", value);
      sysConfig.filterCountMin = value;
      systemWrite();
    }
    if(strcmp(opcode, "RCI") == 0) {
      value = CheckValue(pcommand[1]);
      printf("RCI : %d\r\n", value);
      sysConfig.rciOperatingMin = value;
      systemWrite();
    }
    
    if(strcmp(opcode, "consume") == 0) {
//      systemRead();
      printf("Filter    : %d:%02d\r\n", sysConfig.filterCountMin/60, sysConfig.filterCountMin%60);

      printf("OzoneLamp : %d:%02d\r\n", sysConfig.ozoneLampCountMin/60, sysConfig.ozoneLampCountMin%60);
      printf("UvLamp    : %d:%02d\r\n", sysConfig.uvLampCountMin/60, sysConfig.uvLampCountMin%60);
      printf("RCI cell  : %d:%02d\r\n", sysConfig.rciOperatingMin/60, sysConfig.rciOperatingMin%60);
    }
    if(strcmp(opcode, "consume2") == 0) {

      systemRead();

      printf("filter    : %d:%02d\r\n", sysConfig.filterCountMin/60, sysConfig.filterCountMin%60);
      printf("ozoneLamp : %d:%02d\r\n", sysConfig.ozoneLampCountMin/60, sysConfig.ozoneLampCountMin%60);
      printf("uvLamp    : %d:%02d\r\n", sysConfig.uvLampCountMin/60, sysConfig.uvLampCountMin%60);
      printf("RCI cell  : %d:%02d\r\n", sysConfig.rciOperatingMin/60, sysConfig.rciOperatingMin%60);
    }
    if(strcmp(opcode, "usb") == 0) {
      UpgradeEEPWrite(UpgradeEEPdata);
      NVIC_SystemReset();
    }
}

void command_analysis()
{
    u8   i;
    u8   space_find = 1;
    char *str;
    
    para_idx=0;
    str = &command_str[0];
    for (i=0;i<COMMAND_MAX;i++) pcommand[i] = 0;
    
    while(*str)
    {
        if (*str != ' ') {
            if (space_find) {
                pcommand[para_idx++] = str;
                space_find = 0;
                if (para_idx>COMMAND_MAX) break;
            }
        }
        else {
            if (space_find==0) {
                *str = 0;
            }
            space_find = 1;
        }
        str++;
    }
    if (para_idx>0) {
        command_exec();
    }
}

void GetUARTData()
{
    char ch;
	
    if (rx_head!=rx_tail) {
#ifdef INCLUDE_STOP_MODE
                idleTimeSec = 0;
#endif
        ch = rx_buffer[rx_tail];
        if (++rx_tail>=MAX_RX_BUFFER) rx_tail=0;
        if (ch==0xd || ch==0xa) {
            if (ch==0xd) {
                command_str[command_str_idx] = 0;
                if (command_str_idx!=0) {
                    printf("\r\n");
                    command_analysis();
                } 
                command_str_idx = 0;
                printf("\r\n=> test ");
            }
        }
        else {
            if (ch==0x8) {
                if (command_str_idx>0) {
                    --command_str_idx;
                    command_str[command_str_idx] = 0;
                    printf("%c %c",ch,ch);
                }  
            }
            else {
                if (ch == 9) ch=0x20;
                if (command_str_idx<COMMAND_STR_MAX-2) {
                    if (ch==0x20 || (ch>='0' && ch <='9') || (ch>='a' && ch <='z')
                        || (ch>='A' && ch <='Z') || ch == '?') {
							printf("%c",ch);
							command_str[command_str_idx++] = ch;
							command_str[command_str_idx] = 0;
						}
                }
            }
        }
    }
}