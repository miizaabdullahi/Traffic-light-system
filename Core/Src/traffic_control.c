//*********************************************************************
// Main traffic control logic:
// - Runs the car traffic light FSM
// - Combines car and pedestrian LEDs into shift-register output data
//*********************************************************************

#include "traffic_control.h"
#include "traffic_ped.h"
#include "main.h"
#include "shift_register.h"

/* ================= Timing ================= */

// Duration of the orange/yellow phase (ms)
#define ORANGE_DELAY 1500

/* ================= Car FSM ================= */

// Car traffic light states
typedef enum
{
    CAR_GREEN,
    CAR_ORANGE_TO_RED,
    CAR_RED,
    CAR_ORANGE_TO_GREEN
} CarState_t;

static CarState_t carState;   // Current car FSM state
static uint32_t carTimer;     // Timestamp when a transition started

/* ================= Output buffer ================= */

// Shift register output data
static uint8_t srData[SHIFT_REGISTER_COUNT];

/* ================= Init ================= */

void TrafficCtrl_Init(void)
{
    // Cars start with green light
    carState = CAR_GREEN;
}

/* ================= Update ================= */

void TrafficCtrl_Update(void)
{
    uint32_t now = HAL_GetTick();

    /* -------- Pedestrian FSM -------- */
    TrafficPT_Update();

    /* -------- Car FSM -------- */
    switch (carState)
    {
        case CAR_GREEN:
            // Pedestrians request cars to stop
            if (TrafficPT_RequestCarsRed())
            {
                carState = CAR_ORANGE_TO_RED;
                carTimer = now;
            }
            break;

        case CAR_ORANGE_TO_RED:
            // Wait before switching to red
            if (now - carTimer >= ORANGE_DELAY)
                carState = CAR_RED;
            break;

        case CAR_RED:
            // Stay red until pedestrians are done
            if (!TrafficPT_RequestCarsRed())
            {
                carState = CAR_ORANGE_TO_GREEN;
                carTimer = now;
            }
            break;

        case CAR_ORANGE_TO_GREEN:
            // Wait before switching back to green
            if (now - carTimer >= ORANGE_DELAY)
                carState = CAR_GREEN;
            break;
    }

    /* -------- LED composition -------- */
    // Clear outputs
    srData[U1] = 0;
    srData[U2] = 0;
    srData[U3] = 0;

    // Select car light color
    uint8_t carBits =
        (carState == CAR_GREEN) ? TL_GREEN :
        (carState == CAR_RED)   ? TL_RED   :
                                  TL_YELLOW;

    // Car lights on both directions
    srData[U1] |= carBits;
    srData[U3] |= carBits;

    // Pedestrian LEDs
    srData[U1] |= TrafficPT_GetPedLedMask();
}

/* ================= Accessor ================= */

uint8_t* TrafficCtrl_GetSRData(void)
{
    return srData;
}

/* ================= Delay helpers ================= */

uint32_t TrafficCtrl_GetRemainingRedDelay(void)
{
    if (carState != CAR_RED)
        return 0;

    uint32_t elapsed = HAL_GetTick() - carTimer;
    return (elapsed >= ORANGE_DELAY) ? 0 : (ORANGE_DELAY - elapsed);
}

uint32_t TrafficCtrl_GetRemainingGreenDelay(void)
{
    if (carState == CAR_ORANGE_TO_GREEN)
    {
        uint32_t elapsed = HAL_GetTick() - carTimer;
        return (elapsed >= ORANGE_DELAY) ? 0 : (ORANGE_DELAY - elapsed);
    }

    return 0;
}
