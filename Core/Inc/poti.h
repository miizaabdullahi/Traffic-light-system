//************************************************************
// poti.h
// R4.1: LED brightness proportional to potentiometer voltage
//************************************************************

#ifndef POTI_H
#define POTI_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize potentiometer module
 */
void Poti_Init(void);

/**
 * @brief Update brightness from potentiometer reading
 * Call this periodically (e.g., every 50ms)
 */
void Poti_UpdateBrightness(void);

void Poti_Test_PWM(void);

/**
 * @brief Get current brightness level
 * @return Brightness value (0-255)
 */
uint8_t Poti_GetBrightness(void);

/**
 * @brief Check if LEDs should be on based on PWM
 * Call this frequently (every 1-2ms) for smooth PWM
 * @return true if LEDs should be on, false if off
 */
bool Poti_ShouldLEDsBeOn(void);

#endif /* POTI_H */
