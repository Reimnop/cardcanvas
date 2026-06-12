#ifndef CANVAS_H
#define CANVAS_H

#include <stdint.h>

#include "color.h"
#include "glyph_table.h"

typedef struct {
  Color *imageData;
  uint32_t pxPerCu;
  uint32_t dimensionPx;
} Canvas;

void canvas_init(Canvas *canvas, Color *imageData, uint32_t pxPerCu, uint32_t dimensionPx);
void canvas_set_pixel(Canvas *canvas, uint32_t xPx, uint32_t yPx, Color color);
void canvas_set_block(Canvas *canvas, uint32_t x, uint32_t y, Color color);
void canvas_fill_rect(Canvas *canvas, uint32_t x, uint32_t y, uint32_t width, uint32_t height, Color color);
void canvas_fill_rect_masked(Canvas *canvas, uint32_t x, uint32_t y, uint32_t maskWidth, uint32_t maskHeight, uint32_t maskSize, const uint8_t *mask, Color color);
void canvas_draw_glyph(Canvas *canvas, uint32_t x, uint32_t y, uint32_t size, uint8_t c, Color color);
void canvas_draw_sub_image(Canvas *canvas, uint32_t xPx, uint32_t yPx, uint32_t widthPx, uint32_t heightPx, const Color *subImageData);
void canvas_draw_text(Canvas *canvas, uint32_t x, uint32_t y, uint32_t size, const uint8_t *text, uint32_t textLen, Color color);
void canvas_draw_text_center_aligned(Canvas *canvas, uint32_t x, uint32_t y, uint32_t size, const uint8_t *text, uint32_t textLen, Color color);
void canvas_draw_text_right_aligned(Canvas *canvas, uint32_t x, uint32_t y, uint32_t size, const uint8_t *text, uint32_t textLen, Color color);
void canvas_calculate_text_rect_size(uint32_t textLen, uint32_t size, uint32_t *width, uint32_t *height);

#endif // CANVAS_H