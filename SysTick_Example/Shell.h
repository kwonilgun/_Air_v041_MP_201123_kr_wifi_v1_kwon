#ifndef __SHELL_H
#define __SHELL_H

#include "main.h"

#define COMMAND_STR_MAX          100
#define COMMAND_MAX              30

typedef void (*shell_func)(void);
shell_func analisysFunc();

char    command_str[COMMAND_STR_MAX];
u8      command_str_idx = 0;
char   *pcommand[COMMAND_MAX];
u8      para_idx=0;

extern unsigned char rx_buffer[];
extern unsigned char rx_head;
extern unsigned char rx_tail;
extern unsigned char printfTest;

extern unsigned int G1_SF;
extern unsigned int G2_SF;
extern unsigned int G3_SF;
extern unsigned int G4_SF;
extern unsigned int G5_SF;


extern void relayControl(unsigned char relay, unsigned char onoff);
extern void RTC_TimeRegulate(void);
extern void ledTest();
extern void ledComControl(unsigned char num, unsigned char onoff);
extern void segmentControl(unsigned int num);
extern unsigned char spi(unsigned char data);
extern unsigned short ISD1760_powerUp (void);
extern void TestISD1760 (void);
extern unsigned long ISD1760_readStatus (void);
extern void settingMainMenu(void);

extern void changeState(unsigned char state, unsigned char write);


// kwon:2024-4-21
extern void USART6_SendString(USART_TypeDef* USARTx, uint8_t* string);
extern void UART6_ReceiveData(uint8_t *data, uint16_t size);

#endif //__SHELL_H