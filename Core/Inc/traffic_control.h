#pragma once
#include <stdint.h>

/* Traffic light colors for test API */
typedef enum {
    TL_RED    = 0x01,
    TL_YELLOW = 0x02,
    TL_GREEN  = 0x04
} TrafficLightColor_t;

/* Standard API */
void TrafficCtrl_Init(void);
void TrafficCtrl_Update(void);
uint8_t* TrafficCtrl_GetSRData(void);
uint32_t TrafficCtrl_GetRemainingRedDelay(void);
uint32_t TrafficCtrl_GetRemainingGreenDelay(void);

/* ====================== Test Helpers ====================== */

/* Immediately set all traffic lights to red */
void TrafficCtrl_AllRed(void);

/* Set individual traffic light (0..3) to specified color */
void TrafficCtrl_SetLight(uint8_t index, TrafficLightColor_t color);

/* Force update car inputs from switches (poll GPIO pins) */
void TrafficCtrl_UpdateCarInputs(void);
