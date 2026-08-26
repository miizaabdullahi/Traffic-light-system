//************************************************************
// poti.c
// R4.1: LED brightness proportional to potentiometer voltage
// Using hardware PWM on shift register OE pin
//************************************************************

#include "poti.h"
#include "adc.h"
#include "tim.h"
#include "main.h"

/* ================= Configuration ================= */
// PC7 is connected to TIM3_CH2
#define PWM_TIMER        htim3
#define PWM_CHANNEL      TIM_CHANNEL_2
#define PWM_MAX_VALUE    999       // ARR value from timer config

/* ================= Public API ================= */

/**
 * @brief Initialize potentiometer module and PWM
 */
void Poti_Init(void)
{
    // Start PWM on OE pin
    HAL_TIM_PWM_Start(&PWM_TIMER, PWM_CHANNEL);

    // Set initial brightness to full (duty cycle 0 = full brightness)
    __HAL_TIM_SET_COMPARE(&PWM_TIMER, PWM_CHANNEL, 0);
}

/**
 * Update LED brightness from potentiometer
 */
void Poti_UpdateBrightness(void)
{
    // Start ADC conversion
    HAL_ADC_Start(&hadc1);

    // Wait for conversion
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
    {
        // Read ADC value
        uint32_t adcValue = HAL_ADC_GetValue(&hadc1);

        // Convert to PWM duty cycle
        uint32_t dutyCycle = (adcValue * PWM_MAX_VALUE) / 4095;

        // Set PWM duty cycle
        __HAL_TIM_SET_COMPARE(&PWM_TIMER, PWM_CHANNEL, dutyCycle);
    }

    HAL_ADC_Stop(&hadc1);
}




