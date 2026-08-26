#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#include <stdint.h>

#define SHIFT_REGISTER_COUNT 3

/* ================= Shift register order ================= */

#define U1  2
#define U2  1
#define U3  0


/* ================= Bit definitions ================= */

/* U1 + U2 share same layout */
#define TL_RED      (1U << 0)
#define TL_YELLOW   (1U << 1)
#define TL_GREEN    (1U << 2)
#define PL_RED      (1U << 3)
#define PL_GREEN    (1U << 4)
#define PL_BLUE     (1U << 5)

/* U3 special case */
#define U3_TL4_RED     (1U << 3)
#define U3_TL4_YELLOW  (1U << 4)
#define U3_TL4_GREEN   (1U << 5)

void SR_Init(void);
void SR_Send(uint8_t *data);

#endif
