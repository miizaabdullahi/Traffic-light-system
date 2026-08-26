//*********************************************************************
// Traffic Control R2
// - Manages car traffic FSM for vertical and horizontal lanes
// - Updates shift register outputs
// - Provides timing helpers for display/testing
//*********************************************************************

#include "traffic_control2.h"
#include "main.h"
#include "gpio.h"
#include "shift_register.h"
#include <stdbool.h>

/* ================= Timing parameters (ms) ================= */

static uint32_t orangeDelay = 3000;    // Yellow phase duration
static uint32_t greenDelay  = 10000;   // Maximum green duration
static uint32_t redDelayMax = 8000;    // Max wait before forcing lane change

/* ================= FSM ================= */

// Traffic light states for vertical and horizontal directions
typedef enum
{
    VERT_GREEN,
    VERT_TO_RED,
    VERT_FROM_RED,
    HORZ_GREEN,
    HORZ_TO_RED,
    HORZ_FROM_RED
} TrafficState_t;

static TrafficState_t state;   // Current FSM state
static uint32_t stateTimer;    // Timestamp when current state started

/* ================= Red-lane timer ================= */

// Used when both directions request green at the same time
static bool redTimerRunning = false;
static uint32_t redRequestTime;

/* ================= Shift register buffer ================= */

static uint8_t srData[SHIFT_REGISTER_COUNT];

/* ================= Car detection ================= */

// Detect cars on vertical lanes
static bool CarVerticalActive(void)
{
    return (HAL_GPIO_ReadPin(TL1_Car_GPIO_Port, TL1_Car_Pin) == GPIO_PIN_RESET) ||
           (HAL_GPIO_ReadPin(TL3_Car_GPIO_Port, TL3_Car_Pin) == GPIO_PIN_RESET);
}

// Detect cars on horizontal lanes
static bool CarHorizontalActive(void)
{
    return (HAL_GPIO_ReadPin(TL2_Car_GPIO_Port, TL2_Car_Pin) == GPIO_PIN_RESET) ||
           (HAL_GPIO_ReadPin(TL4_Car_GPIO_Port, TL4_Car_Pin) == GPIO_PIN_RESET);
}

/* ================= Shift register update ================= */

// Compose LED output based on current FSM state
static void UpdateSRBuffer(void)
{
    // Clear outputs
    for (int i = 0; i < SHIFT_REGISTER_COUNT; i++)
        srData[i] = 0;

    uint8_t vertBits = 0;
    uint8_t horzBits = 0;

    // Select colors for each direction
    switch (state)
    {
        case VERT_GREEN:      vertBits = TL_GREEN;  horzBits = TL_RED;    break;
        case VERT_TO_RED:     vertBits = TL_YELLOW; horzBits = TL_RED;    break;
        case VERT_FROM_RED:   vertBits = TL_YELLOW; horzBits = TL_RED;    break;
        case HORZ_GREEN:      vertBits = TL_RED;    horzBits = TL_GREEN;  break;
        case HORZ_TO_RED:     vertBits = TL_RED;    horzBits = TL_YELLOW; break;
        case HORZ_FROM_RED:   vertBits = TL_RED;    horzBits = TL_YELLOW; break;
    }

    // Apply vertical and horizontal lights
    srData[U1] |= vertBits;    // TL1 (vertical)
    srData[U2] |= horzBits;    // TL2 (horizontal)
    srData[U3] |= vertBits;    // TL3 (vertical)

    // TL4 has a custom bit layout on U3
    if (horzBits & TL_RED)           srData[U3] |= U3_TL4_RED;
    else if (horzBits & TL_YELLOW)   srData[U3] |= U3_TL4_YELLOW;
    else if (horzBits & TL_GREEN)    srData[U3] |= U3_TL4_GREEN;
}

/* ================= Public API ================= */

void TrafficCtrl2_Init(void)
{
    // Start with vertical green
    state = VERT_GREEN;
    stateTimer = HAL_GetTick();
    redTimerRunning = false;

    for (int i = 0; i < SHIFT_REGISTER_COUNT; i++)
        srData[i] = 0;
}

void TrafficCtrl2_Update(void)
{
    uint32_t now = HAL_GetTick();
    bool vertActive = CarVerticalActive();
    bool horzActive = CarHorizontalActive();

    // FSM handling with car-activity-based decisions
    switch (state)
    {
        case VERT_GREEN:
            // Horizontal cars waiting -> prepare switch
            if (horzActive && !vertActive) { state = VERT_TO_RED; stateTimer = now; redTimerRunning = false; break; }

            // Both directions active -> start fairness timer
            if (vertActive && horzActive)
            {
                if (!redTimerRunning) { redTimerRunning = true; redRequestTime = now; }
                else if (now - redRequestTime >= redDelayMax) { state = VERT_TO_RED; stateTimer = now; redTimerRunning = false; }
                break;
            }

            // Only vertical traffic -> stay green
            if (vertActive && !horzActive) { redTimerRunning = false; break; }

            // No traffic -> timeout-based switch
            if (!vertActive && !horzActive && (now - stateTimer >= greenDelay)) { state = VERT_TO_RED; stateTimer = now; }
            redTimerRunning = false;
            break;

        case VERT_TO_RED:
            // Vertical yellow phase
            if (now - stateTimer >= orangeDelay) {
                state = HORZ_FROM_RED;
                stateTimer = now;
            }
            break;

        case HORZ_FROM_RED:
            // Horizontal red ->yellow -> green
            if (now - stateTimer >= orangeDelay) {
                state = HORZ_GREEN;
                stateTimer = now;
            }
            break;

        case HORZ_GREEN:
            // Vertical cars waiting -> prepare switch
            if (vertActive && !horzActive) { state = HORZ_TO_RED; stateTimer = now; redTimerRunning = false; break; }

            // Both directions active -> start fairness timer
            if (vertActive && horzActive)
            {
                if (!redTimerRunning) { redTimerRunning = true; redRequestTime = now; }
                else if (now - redRequestTime >= redDelayMax) { state = HORZ_TO_RED; stateTimer = now; redTimerRunning = false; }
                break;
            }

            // Only horizontal traffic -> stay green
            if (horzActive && !vertActive) { redTimerRunning = false; break; }

            // No traffic -> timeout-based switch
            if (!vertActive && !horzActive && (now - stateTimer >= greenDelay)) { state = HORZ_TO_RED; stateTimer = now; }
            redTimerRunning = false;
            break;

        case HORZ_TO_RED:
            // Horizontal yellow phase
            if (now - stateTimer >= orangeDelay) {
                state = VERT_FROM_RED;
                stateTimer = now;
            }
            break;

        case VERT_FROM_RED:
            // Vertical red -> yellow -> green
            if (now - stateTimer >= orangeDelay) {
                state = VERT_GREEN;
                stateTimer = now;
            }
            break;
    }

    UpdateSRBuffer();
}

uint8_t* TrafficCtrl2_GetSRData(void)
{
    return srData;
}

/* ================= Display helpers ================= */

// Remaining green time for selected direction
uint32_t TrafficCtrl2_GetRemainingGreenDelay(uint8_t direction)
{
    uint32_t now = HAL_GetTick();
    uint32_t elapsed;

    if (direction == 0)  // Vertical
    {
        if (state != VERT_GREEN) return 0;

        bool vertActive = CarVerticalActive();
        bool horzActive = CarHorizontalActive();

        if (vertActive && !horzActive) return greenDelay;
        if (vertActive && horzActive && redTimerRunning) {
            elapsed = now - redRequestTime;
            return (elapsed >= redDelayMax) ? 0 : redDelayMax - elapsed;
        }
        if (!vertActive && !horzActive) {
            elapsed = now - stateTimer;
            return (elapsed >= greenDelay) ? 0 : greenDelay - elapsed;
        }
        return 0;
    }
    else  // Horizontal
    {
        if (state != HORZ_GREEN) return 0;

        bool vertActive = CarVerticalActive();
        bool horzActive = CarHorizontalActive();

        if (horzActive && !vertActive) return greenDelay;
        if (vertActive && horzActive && redTimerRunning) {
            elapsed = now - redRequestTime;
            return (elapsed >= redDelayMax) ? 0 : redDelayMax - elapsed;
        }
        if (!vertActive && !horzActive) {
            elapsed = now - stateTimer;
            return (elapsed >= greenDelay) ? 0 : greenDelay - elapsed;
        }
        return 0;
    }
}

// Remaining forced-red wait time for selected direction
uint32_t TrafficCtrl2_GetRemainingRedDelay(uint8_t direction)
{
    uint32_t now = HAL_GetTick();
    uint32_t elapsed;

    if (direction == 0)  // Vertical
    {
        if (state == HORZ_GREEN && redTimerRunning) {
            elapsed = now - redRequestTime;
            return (elapsed >= redDelayMax) ? 0 : redDelayMax - elapsed;
        }
        return 0;
    }
    else  // Horizontal
    {
        if (state == VERT_GREEN && redTimerRunning) {
            elapsed = now - redRequestTime;
            return (elapsed >= redDelayMax) ? 0 : redDelayMax - elapsed;
        }
        return 0;
    }
}
