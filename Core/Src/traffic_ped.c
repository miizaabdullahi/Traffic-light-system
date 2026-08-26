//***********************************************************************************
// Pedestrian traffic control:
// - Handles pedestrian button input
// - Runs the pedestrian FSM
// - Requests cars to stop and controls pedestrian LEDs
//***********************************************************************************

#include "traffic_ped.h"
#include "main.h"
#include <stdbool.h>

/* ================= Timing parameters ================= */

uint32_t toggleFreq        = 500;   // Blue LED blink rate
uint32_t pedestrianDelay  = 4000;  // Wait time before allowing crossing
uint32_t walkingDelay     = 6000;  // Pedestrian green duration

/* ================= Pedestrian LEDs ================= */

#define PL_RED    (1U << 3)
#define PL_GREEN  (1U << 4)
#define PL_BLUE   (1U << 5)

/* ================= FSM ================= */

// Pedestrian FSM states
typedef enum
{
    PT_IDLE,      // No request
    PT_WAITING,   // Button pressed, waiting for cars to stop
    PT_GREEN      // Pedestrians may cross
} PT_State_t;

/* ================= Internal state ================= */

static PT_State_t state;        // Current FSM state
static uint32_t buttonTime;     // Time when button was pressed
static uint32_t lastToggleTime; // LED blink timing
static uint32_t greenStartTime; // Start of pedestrian green

static bool requestCarsRed;     // Signal to car controller
static bool pedGreen;           // Pedestrian crossing active
static uint8_t pedLedMask;      // LED output mask

/* ================= Button ================= */

// Read pedestrian button (active low)
static inline bool ButtonPressed(void)
{
    return HAL_GPIO_ReadPin(PL1_Switch_GPIO_Port, PL1_Switch_Pin) == GPIO_PIN_RESET;
}

/* ================= Init ================= */

void TrafficPT_Init(void)
{
    state = PT_IDLE;
    requestCarsRed = false;
    pedGreen = false;
    pedLedMask = PL_RED;   // Default: do not cross
}

/* ================= Public API ================= */

bool TrafficPT_RequestCarsRed(void)
{
    return requestCarsRed;
}

bool TrafficPT_IsPedGreen(void)
{
    return pedGreen;
}

uint8_t TrafficPT_GetPedLedMask(void)
{
    return pedLedMask;
}

// Remaining wait time before pedestrian green
uint32_t TrafficPT_GetRemainingPedDelay(void)
{
    if (state != PT_WAITING)
        return 0;

    uint32_t elapsed = HAL_GetTick() - buttonTime;
    return (elapsed >= pedestrianDelay) ? 0 : (pedestrianDelay - elapsed);
}

// Remaining time pedestrians may walk
uint32_t TrafficPT_GetRemainingWalkDelay(void)
{
    if (state != PT_GREEN)
        return 0;

    uint32_t elapsed = HAL_GetTick() - greenStartTime;
    return (elapsed >= walkingDelay) ? 0 : (walkingDelay - elapsed);
}

/* ================= FSM ================= */

void TrafficPT_Update(void)
{
    uint32_t now = HAL_GetTick();

    switch (state)
    {
        case PT_IDLE:
            // Default state: cars may drive, pedestrians wait
            pedGreen = false;
            requestCarsRed = false;
            pedLedMask = PL_RED;

            // Button press starts waiting phase
            if (ButtonPressed())
            {
                buttonTime = now;
                lastToggleTime = now;
                state = PT_WAITING;
            }
            break;

        case PT_WAITING:
            // Request cars to stop, show waiting indication
            pedGreen = false;
            requestCarsRed = true;

            // Red + blinking blue
            pedLedMask = PL_RED;
            if (now - lastToggleTime >= toggleFreq)
            {
                lastToggleTime = now;
                pedLedMask ^= PL_BLUE;
            }

            // After delay, allow crossing
            if (now - buttonTime >= pedestrianDelay)
            {
                greenStartTime = now;
                state = PT_GREEN;
            }
            break;

        case PT_GREEN:
            // Pedestrians may cross, cars remain stopped
            pedGreen = true;
            requestCarsRed = true;
            pedLedMask = PL_GREEN;

            // Return to idle after walking time
            if (now - greenStartTime >= walkingDelay)
            {
                pedGreen = false;
                state = PT_IDLE;
            }
            break;
    }
}
