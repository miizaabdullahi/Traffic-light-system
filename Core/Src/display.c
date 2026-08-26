//************************************************************
// display.c
//************************************************************

#include "display.h"
#include "ssd1306.h"
#include "traffic_ped.h"
#include "traffic_control.h"
#include "traffic_control2.h"
#include "ssd1306_fonts.h"
#include <stdint.h>

/* ================= Layout ================= */

#define BAR_WIDTH     10
#define BAR_HEIGHT    48
#define BAR_TOP       2
#define LABEL_Y       54

#define MAX_BAR_VALUE 50

/* ================= Bar positions (explicit) ================= */

static const uint8_t barPositions[8] = {
    3,   // P1
    19,  // W1 (gap of 6px)
    35,  // P2
    51,  // W2
    67,  // G1
    83,  // R1
    99,  // G2
    115  // R2 (115 + 10 = 125)
};

/* ================= Bar helper ================= */

static void DrawBar(uint8_t barIndex, uint32_t value)
{
    uint8_t x = barPositions[barIndex];

    if (value > MAX_BAR_VALUE)
        value = MAX_BAR_VALUE;

    // Top line
    for (uint8_t px = 0; px < BAR_WIDTH; px++)
        ssd1306_DrawPixel(x + px, BAR_TOP, White);

    // Bottom line
    for (uint8_t px = 0; px < BAR_WIDTH; px++)
        ssd1306_DrawPixel(x + px, BAR_TOP + BAR_HEIGHT - 1, White);

    // Left line
    for (uint8_t py = 0; py < BAR_HEIGHT; py++)
        ssd1306_DrawPixel(x, BAR_TOP + py, White);

    // Right line
    for (uint8_t py = 0; py < BAR_HEIGHT; py++)
        ssd1306_DrawPixel(x + BAR_WIDTH - 1, BAR_TOP + py, White);

    if (value > 0)
    {
        uint8_t fillHeight = (value * (BAR_HEIGHT - 2)) / MAX_BAR_VALUE;
        if (fillHeight > 0)
        {
            // Bottom of interior space
            uint8_t bottomY = BAR_TOP + BAR_HEIGHT - 2;

            // Fill line by line from bottom going UP
            for (uint8_t i = 0; i < fillHeight; i++)
            {
                uint8_t lineY = bottomY - i;
                // Draw horizontal line INSIDE the outline
                for (uint8_t px = 1; px < BAR_WIDTH - 1; px++)
                {
                    ssd1306_DrawPixel(x + px, lineY, White);
                }
            }
        }
    }
}

/* ================= Public API ================= */

void Display_Init(void)
{
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();
}

void Display_Update(void)
{
    ssd1306_Fill(Black);

    /* ================= Get remaining times (Task 1) ================= */

    uint32_t pedRemain  = TrafficPT_GetRemainingPedDelay() / 50;
    uint32_t walkRemain = TrafficPT_GetRemainingWalkDelay() / 50;
    uint32_t greenRemain = TrafficCtrl_GetRemainingGreenDelay() / 50;

    /* ================= Get remaining times (Task 2) ================= */

    // Task 2 has no pedestrians, only car traffic lights
    uint32_t greenRemainVert = TrafficCtrl2_GetRemainingGreenDelay(0) / 50;  // Vertical (TL1, TL3)
    uint32_t redRemainVert   = TrafficCtrl2_GetRemainingRedDelay(0) / 50;
    uint32_t greenRemainHorz = TrafficCtrl2_GetRemainingGreenDelay(1) / 50;  // Horizontal (TL2, TL4)
    uint32_t redRemainHorz   = TrafficCtrl2_GetRemainingRedDelay(1) / 50;

    /* ================= Draw 8 bars ================= */

    // P1 - Pedestrian waiting
    DrawBar(0, pedRemain);
    ssd1306_SetCursor(barPositions[0] + 1, LABEL_Y);
    ssd1306_WriteString("P1", Font_6x8, White);

    // W1 - Pedestrian walking
    DrawBar(1, walkRemain);
    ssd1306_SetCursor(barPositions[1] + 1, LABEL_Y);
    ssd1306_WriteString("W1", Font_6x8, White);

    // P2 - Empty
    DrawBar(2, 0);
    ssd1306_SetCursor(barPositions[2] + 1, LABEL_Y);
    ssd1306_WriteString("P2", Font_6x8, White);

    // W2 - Empty
    DrawBar(3, 0);
    ssd1306_SetCursor(barPositions[3] + 1, LABEL_Y);
    ssd1306_WriteString("W2", Font_6x8, White);

    // G1 - Car green (ACTIVE)
    DrawBar(4, greenRemainVert);
    ssd1306_SetCursor(barPositions[4] + 1, LABEL_Y);
    ssd1306_WriteString("G1", Font_6x8, White);

    // R1
    DrawBar(5, redRemainVert);
    ssd1306_SetCursor(barPositions[5] + 1, LABEL_Y);
    ssd1306_WriteString("R1", Font_6x8, White);

    // G2
    DrawBar(6, greenRemainHorz);
    ssd1306_SetCursor(barPositions[6] + 1, LABEL_Y);
    ssd1306_WriteString("G2", Font_6x8, White);

    // R2
    DrawBar(7, redRemainHorz);
    ssd1306_SetCursor(barPositions[7], LABEL_Y);
    ssd1306_WriteString("R2", Font_6x8, White);

    ssd1306_UpdateScreen();
}
