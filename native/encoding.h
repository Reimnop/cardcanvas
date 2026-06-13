#ifndef ENCODING_H
#define ENCODING_H

#include <stdint.h>
#include <stdbool.h>

#include "color.h"

#define DIMENSION_IN_CU  256
#define BORDER_MARGIN    2
#define BORDER_THICKNESS 10
#define OVERFLOW_X       120
#define OVERFLOW_Y       95
#define OVERFLOW_W       116
#define OVERFLOW_H       43

uint32_t encoding_calculate_dimension_px(uint32_t pxPerCu);
uint32_t encoding_calculate_total_capacity(uint32_t pxPerCu);
uint32_t encoding_calculate_required_px_per_cu(uint32_t size);
void encoding_encode_card_data(uint32_t pxPerCu, const uint8_t *input, uint32_t inputLen, Color *output);
void encoding_decode_card_data(const Color *input, uint32_t dimensionPx, uint8_t *output, uint32_t *outputLen);

#endif // ENCODING_H