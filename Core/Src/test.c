//************************************************************
// test.c
//************************************************************

#include <string.h>   // memset
#include <stdio.h>    // snprintf
#include "test.h"
#include "traffic_control.h"
#include "traffic_control2.h"
#include "traffic_ped.h"
#include "shift_register.h"
#include "gpio.h"
#include "ssd1306.h"
#include "display.h"
#include "poti.h"
#include "adc.h"
#include "main.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"


#define TEST_DELAY(ms) HAL_Delay(ms)

/* ============================================================
 * Shift Register Test
 * ============================================================ */
void test_sr(void)
{
    /* All LEDs connected to the shift register are initially turned off.
     * Then, exactly one LED turns on at a time.
     * The active LED moves through every bit of every shift register,
     * creating a "walking 1" pattern across all outputs.
     * No two LEDs should be on simultaneously at any point.
     */

    uint8_t srData[SHIFT_REGISTER_COUNT];

    /* Clear all outputs */
    memset(srData, 0, sizeof(srData));
    SR_Send(srData);

    /* Iterate over each shift register */
    for (uint8_t reg = 0; reg < SHIFT_REGISTER_COUNT; reg++)
    {
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            memset(srData, 0, sizeof(srData));
            srData[reg] = (1U << bit);

            SR_Send(srData);
            HAL_Delay(500);

            srData[reg] = 0;
            SR_Send(srData);
            HAL_Delay(100);
        }
    }
}

/* ============================================================
 * Car Presence Switch Test
 * ============================================================ */

void test_car_switch(void)
{
	/* Each traffic light turns green as soon as its corresponding
	 * car presence switch is activated.
	 * If no car is present, the traffic light remains red.
	 */
    uint8_t sr[SHIFT_REGISTER_COUNT] = {0};

    while (1)
    {
        // Read each car switch
        bool car1 = HAL_GPIO_ReadPin(TL1_Car_GPIO_Port, TL1_Car_Pin) == GPIO_PIN_RESET;
        bool car2 = HAL_GPIO_ReadPin(TL2_Car_GPIO_Port, TL2_Car_Pin) == GPIO_PIN_RESET;
        bool car3 = HAL_GPIO_ReadPin(TL3_Car_GPIO_Port, TL3_Car_Pin) == GPIO_PIN_RESET;
        bool car4 = HAL_GPIO_ReadPin(TL4_Car_GPIO_Port, TL4_Car_Pin) == GPIO_PIN_RESET;

        // Clear SR buffer
        for (int i = 0; i < SHIFT_REGISTER_COUNT; i++) sr[i] = 0;

        // Set vertical traffic lights
        sr[U1] = car1 ? TL_GREEN : TL_RED;    // TL1
        sr[U3] = car3 ? TL_GREEN : TL_RED;    // TL3

        // Set horizontal traffic lights
        sr[U2] = car2 ? TL_GREEN : TL_RED;    // TL2
        sr[U3] &= ~(U3_TL4_RED|U3_TL4_GREEN); // Clear TL4 first
        sr[U3] |= car4 ? U3_TL4_GREEN : U3_TL4_RED; // TL4

        // Send SR data to hardware
        SR_Send(sr);
    }
}

/* ============================================================
 * Pedestrian Button Input Test
 * ============================================================ */

void test_ped_buttons(void)
{
	/* Pressing a pedestrian crossing button turns on the
	 * corresponding blue pedestrian lights.
	 * Keep the LED on as long as the button is pressed.
	 */
    uint8_t sr[SHIFT_REGISTER_COUNT] = {0};

    while (1)
    {
        // Read pedestrian buttons
        bool ped1 = HAL_GPIO_ReadPin(PL1_Switch_GPIO_Port, PL1_Switch_Pin) == GPIO_PIN_RESET;
        bool ped2 = HAL_GPIO_ReadPin(PL2_Switch_GPIO_Port, PL2_Switch_Pin) == GPIO_PIN_RESET;

        // Clear SR buffer (only touching pedestrian bits)
        sr[U1] &= ~(PL_BLUE); // Clear PL1 blue
        sr[U2] &= ~(PL_BLUE); // Clear PL2 blue

        // Set pedestrian blue lights based on button press
        if (ped1)
            sr[U1] |= PL_BLUE;
        if (ped2)
            sr[U2] |= PL_BLUE;

        // Send SR data to hardware
        SR_Send(sr);
    }
}

/* ============================================================
 * OLED Display Test
 * ============================================================ */
void test_oled(void)
{
	/* A white rectangular frame is drawn along the edges of the OLED display.
	 * The text "TEST OLED" appears centered inside the frame.
	 * The display should be static, with no flickering.
	 */

    // Clear the display first
    ssd1306_Fill(Black);

    // Draw top and bottom horizontal lines
    for (int x = 0; x < 128; x++)
    {
        ssd1306_DrawPixel(x, 0, White);      // Top
        ssd1306_DrawPixel(x, 63, White);     // Bottom
    }

    // Draw left and right vertical lines
    for (int y = 0; y < 64; y++)
    {
        ssd1306_DrawPixel(0, y, White);      // Left
        ssd1306_DrawPixel(127, y, White);    // Right
    }

    // Draw the text "TEST OLED" in the center
    int textX = 128 / 2 - (6 * 9) / 2;  // 9 characters, Font_6x8 -> 6px per char
    int textY = 64 / 2 - 8 / 2;         // 8px height for Font_6x8
    ssd1306_SetCursor(textX, textY);
    ssd1306_WriteString("TEST OLED", Font_6x8, White);

    // Update the display
    ssd1306_UpdateScreen();
}

/* ============================================================
 * Potentiometer / ADC Test
 * ============================================================ */
void test_potentiometer(void)
{
	/* Red, Yellow or Green traffic lights light up
	 * depending on the potentiometer's position.
	 */
    uint8_t sr[SHIFT_REGISTER_COUNT];

    while (1)
    {
        // Start ADC conversion
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
        {
            uint32_t pot = HAL_ADC_GetValue(&hadc1);
            uint8_t color;

            if (pot <= 1365)
                color = TL_RED;
            else if (pot <= 2730)
                color = TL_YELLOW;
            else
                color = TL_GREEN;

            // Clear shift register
            for (int i = 0; i < SHIFT_REGISTER_COUNT; i++)
                sr[i] = 0;

            // Set vertical lights TL1/TL3
            sr[U1] = color;             // TL1
            sr[U3] &= ~(TL_RED|TL_YELLOW|TL_GREEN); // clear TL3/TL4 bits
            sr[U3] |= color;             // TL3

            // Set horizontal lights TL2/TL4
            sr[U2] = color;             // TL2
            sr[U3] &= ~(U3_TL4_RED|U3_TL4_YELLOW|U3_TL4_GREEN); // clear TL4
            if (color == TL_RED) sr[U3] |= U3_TL4_RED;
            else if (color == TL_YELLOW) sr[U3] |= U3_TL4_YELLOW;
            else sr[U3] |= U3_TL4_GREEN;

            // Send to shift registers
            SR_Send(sr);
        }
        HAL_ADC_Stop(&hadc1);

        HAL_Delay(50); // small delay to stabilize
    }
}
