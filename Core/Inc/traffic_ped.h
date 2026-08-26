#ifndef TRAFFIC_PED_H
#define TRAFFIC_PED_H

#include <stdint.h>
#include <stdbool.h>

void TrafficPT_Init(void);
void TrafficPT_Update(void);
bool TrafficPT_RequestCarsRed(void);
bool TrafficPT_IsPedGreen(void);
uint8_t TrafficPT_GetPedLedMask(void);

uint32_t TrafficPT_GetRemainingPedDelay(void);   // ms
uint32_t TrafficPT_GetRemainingWalkDelay(void);  // ms

void PedLight_BlueBlink(uint8_t crossing, uint32_t duration_ms);
void PedLight_ToggleBlink(uint8_t crossing);
void PedButton_Task(void);
bool PedButton_WasPressed(uint8_t button);
bool PedButton_IsPressed(uint8_t button);
void PedLight_BlueOn(uint8_t crossing);
void PedLight_BlueOff(uint8_t crossing);


#endif
