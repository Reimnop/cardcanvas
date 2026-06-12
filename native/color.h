#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef uint32_t Color; // 0xAABBGGRR

uint32_t color_make(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint8_t color_get_r(uint32_t color);
uint8_t color_get_g(uint32_t color);
uint8_t color_get_b(uint32_t color);
uint8_t color_get_a(uint32_t color);

#endif // COLOR_H