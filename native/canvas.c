#include "canvas.h"

void canvas_init(Canvas *canvas, Color *imageData, uint32_t pxPerCu, uint32_t dimensionPx) {
  canvas->imageData = imageData;
  canvas->pxPerCu = pxPerCu;
  canvas->dimensionPx = dimensionPx;
}

void canvas_set_pixel(Canvas *canvas, uint32_t xPx, uint32_t yPx, Color color) {
  canvas->imageData[yPx * canvas->dimensionPx + xPx] = color;
}

void canvas_set_block(Canvas *canvas, uint32_t x, uint32_t y, Color color) {
  uint32_t blockX1Px = x * canvas->pxPerCu;
  uint32_t blockY1Px = y * canvas->pxPerCu;
  uint32_t blockY2Px = blockY1Px + canvas->pxPerCu;

  for (uint32_t i = blockY1Px; i < blockY2Px; i++) {
    Color *row = &canvas->imageData[i * canvas->dimensionPx + blockX1Px];
    for (uint32_t j = 0; j < canvas->pxPerCu; j++)
      row[j] = color;
  }
}

void canvas_fill_rect(Canvas *canvas, uint32_t x, uint32_t y, uint32_t width, uint32_t height, Color color) {
  uint32_t rectX1Px = x * canvas->pxPerCu;
  uint32_t rectY1Px = y * canvas->pxPerCu;
  uint32_t rectY2Px = rectY1Px + height * canvas->pxPerCu;
  uint32_t rectWidthPx = width * canvas->pxPerCu;

  for (uint32_t i = rectY1Px; i < rectY2Px; i++) {
    Color *row = &canvas->imageData[i * canvas->dimensionPx + rectX1Px];
    for (uint32_t j = 0; j < rectWidthPx; j++)
      row[j] = color;
  }
}

void canvas_fill_rect_masked(Canvas *canvas, uint32_t x, uint32_t y, uint32_t maskWidth, uint32_t maskHeight, uint32_t maskSize, const uint8_t *mask, Color color) {
  for (uint32_t i = 0; i < maskHeight; i++) {
    for (uint32_t j = 0; j < maskWidth; j++) {
      if ((mask[i] & (1 << (maskWidth - 1 - j))) == 0)
        continue;
      canvas_fill_rect(canvas, x + j * maskSize, y + i * maskSize, maskSize, maskSize, color);
    }
  }
}

void canvas_draw_glyph(Canvas *canvas, uint32_t x, uint32_t y, uint32_t size, uint8_t c, Color color) {
  if (c >= 'a' && c <= 'z')
    c = c - 'a' + 'A';

  const uint8_t *glyph = get_glyph(c);
  if (!glyph)
    glyph = get_glyph('?');

  canvas_fill_rect_masked(canvas, x, y, 3, 5, size, glyph, color);
}

void canvas_draw_sub_image(Canvas *canvas, uint32_t xPx, uint32_t yPx, uint32_t widthPx, uint32_t heightPx, const Color *subImageData) {
  uint32_t y2Px = yPx + heightPx;
  for (uint32_t i = yPx; i < y2Px; i++) {
    uint32_t subI = i - yPx;
    const Color *srcRow = &subImageData[subI * widthPx];
    Color *dstRow = &canvas->imageData[i * canvas->dimensionPx + xPx];

    for (uint32_t j = 0; j < widthPx; j++) {
      Color src = srcRow[j];
      Color dst = dstRow[j];

      float srcAlpha = color_get_a(src) / 255.0f;
      float invAlpha = 1.0f - srcAlpha;

      uint8_t r = (uint8_t)(color_get_r(src) * srcAlpha + color_get_r(dst) * invAlpha);
      uint8_t g = (uint8_t)(color_get_g(src) * srcAlpha + color_get_g(dst) * invAlpha);
      uint8_t b = (uint8_t)(color_get_b(src) * srcAlpha + color_get_b(dst) * invAlpha);

      dstRow[j] = color_make(r, g, b, color_get_a(dst));
    }
  }
}

void canvas_draw_text(Canvas *canvas, uint32_t x, uint32_t y, uint32_t size, const uint8_t *text, uint32_t textLen, Color color) {
  uint32_t cursor = x;
  for (uint32_t i = 0; i < textLen; i++) {
    canvas_draw_glyph(canvas, cursor, y, size, text[i], color);
    cursor += 4 * size;
  }
}

void canvas_calculate_text_rect_size(uint32_t textLen, uint32_t size, uint32_t *width, uint32_t *height) {
  *width  = textLen > 0 ? (4 * textLen - 1) * size : 0;
  *height = 5 * size;
}

void canvas_draw_text_center_aligned(Canvas *canvas, uint32_t x, uint32_t y, uint32_t size, const uint8_t *text, uint32_t textLen, Color color) {
  uint32_t width, height;
  canvas_calculate_text_rect_size(textLen, size, &width, &height);
  canvas_draw_text(canvas, x - width / 2, y, size, text, textLen, color);
}

void canvas_draw_text_right_aligned(Canvas *canvas, uint32_t x, uint32_t y, uint32_t size, const uint8_t *text, uint32_t textLen, Color color) {
  uint32_t width, height;
  canvas_calculate_text_rect_size(textLen, size, &width, &height);
  canvas_draw_text(canvas, x - width, y, size, text, textLen, color);
}
