#include "shift_register.h"
#include "main.h"
#include "spi.h"
#include "gpio.h"

#define LOW   GPIO_PIN_RESET
#define HIGH  GPIO_PIN_SET

static void SR_Latch(void)
{
    HAL_GPIO_WritePin(GPIOB, STCP_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, STCP_Pin, GPIO_PIN_SET);
}

void SR_Init(void)
{
    HAL_GPIO_WritePin(GPIOC, Enable_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, Disp_Reset_Pin, GPIO_PIN_SET);
}

void SR_Send(uint8_t *data)
{
    HAL_SPI_Transmit(&hspi3, data, SHIFT_REGISTER_COUNT, HAL_MAX_DELAY);
    SR_Latch();
}
