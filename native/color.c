#include "color.h"

uint32_t color_make(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return (uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16) | ((uint32_t)a << 24);
}

uint8_t color_get_r(uint32_t color) { 
  return (uint8_t)(color & 0xFF); 
}

uint8_t color_get_g(uint32_t color) { 
  return (uint8_t)((color >> 8) & 0xFF); 
}

uint8_t color_get_b(uint32_t color) { 
  return (uint8_t)((color >> 16) & 0xFF); 
}

uint8_t color_get_a(uint32_t color) { 
  return (uint8_t)((color >> 24) & 0xFF);
}
