#ifndef TRAFFIC_CONTROL2_H
#define TRAFFIC_CONTROL2_H

#include <stdint.h>

/* ================= Direction constants ================= */
#define DIR_VERTICAL     0
#define DIR_HORIZONTAL   1

/* ================= FSM state constants ================= */
#define STATE_GO         0
#define STATE_TRANSITION 1
#define STATE_STOP       2

/* Initialize traffic controller (R2.8) */
void TrafficCtrl2_Init(void);

/* Call periodically from main loop */
void TrafficCtrl2_Update(void);

/* Get shift register output buffer */
uint8_t* TrafficCtrl2_GetSRData(void);


uint32_t TrafficCtrl2_GetRemainingGreenDelay(uint8_t direction);
uint32_t TrafficCtrl2_GetRemainingRedDelay(uint8_t direction);

void TrafficCtrl2_SetDirection(uint8_t direction);
void TrafficCtrl2_ForceState(uint8_t fsmState);



#endif /* TRAFFIC_CONTROL2_H */
