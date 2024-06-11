
//2024.5.26 kwon creation


// #include  <stdio.h>
// #include <string.h>
#include "main.h"
#include "handler.h"


// Function to format the STATE_DIS message
void formatStateDisMessage(char* status, unsigned int time, char* formatted_str, size_t size) {
    char str[50];  // Ensure this buffer is large enough to hold the entire message
    char str_time[20];  // Buffer to hold the converted time value

    sprintf(str_time, "%u", time);  // unsigned int 값을 문자열로 변환
    strcpy(str, status);
    strcat(str, str_time);  // Concatenate the time value to the status message

    snprintf(formatted_str, size, "[%s]", str);  // Ensure not to overflow formatted_str
}

void formatSystemInfoMessage(char* str, size_t size ){
    // 각 값을 버퍼에 저장

    uint8_t buffer[100] = "";
    systemRead();

    int offset = 0;  // 버퍼에서 현재 위치를 추적


    // 각 값을 버퍼에 연결하여 저장
    offset += snprintf(buffer + offset, size - offset, "SYSINFO:filter:%d", sysConfig.filterCountMin);
    offset += snprintf(buffer + offset, size - offset, ":ozoneLamp:%d", sysConfig.ozoneLampCountMin);
    offset += snprintf(buffer + offset, size - offset, ":uvLamp:%d", sysConfig.uvLampCountMin);
    offset += snprintf(buffer + offset, size - offset, ":RCIcell:%d", sysConfig.rciOperatingMin);

    // 2024.6.11: kwon 버전 번호 EEPROM으로 부터 읽어서 보낸다.
    offset += snprintf(buffer + offset, size - offset, ":version:%d", readVersionFromEeprom());

     snprintf(str, size, "[%s]", buffer);

    // 최종 결과 출력 (디버깅 목적)
    printf("%s", str);
}

// 2024-5-30 : "start" 코맨드 처리 함수 정의
void processStartState( char* switchState, char* action_str, char* duration_str, char* wind_speed_str) {
    int action, duration, wind_speed;

    // sscanf를 사용하여 문자열에서 값을 추출합니다.
    sscanf(switchState, "{\"action\":%d,\"duration\":%d,\"wind_speed\":%d}", &action, &duration, &wind_speed);

    printf("Action: %d\r\n", action);
    printf("Duration: %d\r\n", duration);
    printf("Wind Speed: %d\r\n", wind_speed);

    // int 값을 문자열로 변환합니다.
    sprintf(action_str, "%d", action);
    sprintf(duration_str, "%d", duration);
    sprintf(wind_speed_str, "%d", wind_speed);
}