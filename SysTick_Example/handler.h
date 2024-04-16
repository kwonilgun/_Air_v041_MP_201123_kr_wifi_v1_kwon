#ifndef __HANDLER_H
#define __HANDLER_H

#include "main.h"

/* Value Types  ------------------------------------------------------------*/
extern unsigned int currentState;
extern unsigned int destState;            // SJM 201121 added for consumable
extern unsigned int isFirstEntrance;
//extern unsigned int isReadyEntrance;    SJM 190711 never used
extern unsigned int g_remoteFlag;
extern SYSTEMINFO sysConfig;
extern PLASMAINFO plasmaInfo;
extern DISINFO disInfo;
extern IONINFO ionInfo;
extern unsigned int segBlinkOnTime;

extern unsigned int G1_SF;
extern unsigned int G2_SF;
extern unsigned int G3_SF;
extern unsigned int G4_SF;

extern unsigned char pidDetect;
/* Function Types  ------------------------------------------------------------*/
void handler();

// kwon 2024-4-15
void serial_handler(struct IotCommandSet * );

extern void control_sterOn();
extern void control_ledFont();
extern void control_initLed();
extern void control_disLed();
extern void segmentOff();
extern void control_relayAllOff();
extern void control_relayDestruction();
extern void control_desLed();
extern void control_sterStartLedOn();
extern void control_sterStopLedOn();
extern void control_consumableCheckLed();
extern void systemWrite();

//Disinfect
extern void control_disRelayOn();
//extern void control_disRelayOff();    SJM 190711 never used
extern void control_disLed();

//Ion
extern void control_IonOn();
//extern void control_IonOff();         SJM 190711 never used
extern void control_ionLed();

//timer
extern int prepareTimer;
extern int destructionTimer;

//plasma blink
extern unsigned int plasmaBlinkOn;
extern unsigned int plasmaBlinkOnTimer;
extern unsigned int plasmaBlinkOnFlag;
extern unsigned int plasmaBlinkOffTimer;
extern unsigned int plasmaBlinkOffFlag;

//extern unsigned int pidLEDOnFlag;     SJM 190711 always assigned to 'FALSE' & never used
//unsigned char sterOzoneDetectFlag;        SJM 190711 never used
//#ifndef HPA_36C
#if ( MACHINE == HPA_130W)
  extern unsigned int dOzoneSensoredValue;
#endif
//void
extern void voicePlay(unsigned int voice, unsigned int delay);

//engineer
extern void control_engineerLed();
#endif
