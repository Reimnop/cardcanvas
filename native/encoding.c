#include "encoding.h"

static uint32_t calculate_border_usable_pixel_count(uint32_t pxPerCu) {
  uint32_t borderDimension = DIMENSION_IN_CU - BORDER_MARGIN * 2;
  uint32_t borderCuCount = borderDimension * BORDER_THICKNESS * 4 - BORDER_THICKNESS * BORDER_THICKNESS * 4;
  return borderCuCount * pxPerCu * pxPerCu;
}

static uint32_t calculate_overflow_usable_pixel_count(uint32_t pxPerCu) {
  return OVERFLOW_W * OVERFLOW_H * pxPerCu * pxPerCu;
}

static uint32_t calculate_total_usable_pixel_count(uint32_t pxPerCu) {
  return calculate_border_usable_pixel_count(pxPerCu) + calculate_overflow_usable_pixel_count(pxPerCu);
}

static bool is_pixel_in_border(uint32_t pxPerCu, uint32_t x, uint32_t y) {
  uint32_t dimensionPx = DIMENSION_IN_CU * pxPerCu;
  uint32_t borderMarginPx = BORDER_MARGIN * pxPerCu;
  uint32_t borderThicknessPx = BORDER_THICKNESS * pxPerCu;
  uint32_t panelStartPx = borderMarginPx + borderThicknessPx;
  uint32_t panelEndPx = dimensionPx - borderMarginPx - borderThicknessPx;

  if (x >= panelStartPx && x < panelEndPx && y >= panelStartPx && y < panelEndPx)
    return false;

  uint32_t borderStartPx = borderMarginPx;
  uint32_t borderEndPx = dimensionPx - borderMarginPx;

  return x >= borderStartPx && x < borderEndPx && y >= borderStartPx && y < borderEndPx;
}

static uint8_t color_to_nibble(uint32_t color) {
  uint8_t r = color_get_r(color);
  uint8_t g = color_get_g(color);
  uint8_t b = color_get_b(color);

  if (r == g || r == b) return r / 17;
  if (g == b) return g / 17;

  // median
  uint8_t lo = r < g ? r : g;
  uint8_t hi = r > g ? r : g;
  uint8_t median = b < lo ? lo : (b > hi ? hi : b);
  return median / 17;
}

static uint8_t get_nibble(uint32_t i, uint32_t inputLen, const uint8_t *input) {
  uint32_t byteIdx = i / 2;
  uint8_t byteVal = byteIdx < 4
    ? (uint8_t)((inputLen >> (24 - byteIdx * 8)) & 0xFF)
    : input[byteIdx - 4];
  return (i % 2 == 0) ? ((byteVal >> 4) & 0xF) : (byteVal & 0xF);
}

static void consume_nibble(uint8_t n, uint8_t *pendingNibble, uint32_t *nibbleOffset, uint8_t *output, uint32_t *byteOffset) {
  if (*nibbleOffset % 2 == 0) {
    *pendingNibble = n & 0xF;
  } else {
    output[(*byteOffset)++] = (*pendingNibble << 4) | (n & 0xF);
  }
  (*nibbleOffset)++;
}

uint32_t encoding_calculate_dimension_px(uint32_t pxPerCu) {
  return DIMENSION_IN_CU * pxPerCu;
}

uint32_t encoding_calculate_total_capacity(uint32_t pxPerCu) {
  return calculate_total_usable_pixel_count(pxPerCu) / 2;
}

uint32_t encoding_calculate_required_px_per_cu(uint32_t size) {
  size += 4;
  uint32_t result = 1;
  while (encoding_calculate_total_capacity(result) < size)
    result *= 2;
  return result;
}

void encoding_encode_card_data(uint32_t pxPerCu, const uint8_t *input, uint32_t inputLen, Color *output) {
  uint32_t dimensionPx = DIMENSION_IN_CU * pxPerCu;
  uint32_t pixelCount = dimensionPx * dimensionPx;

  // build payload: 4-byte big-endian length prefix + input
  uint32_t payloadLen = 4 + inputLen;
  // caller must ensure payloadLen <= encoding_calculate_total_capacity(pxPerCu)

  // write nibbles directly without allocating a separate payload buffer
  // we iterate border then overflow, writing nibble pairs as we go
  uint32_t totalNibbles = payloadLen * 2;

  uint32_t nibbleOffset = 0;

  // border region
  for (uint32_t y = 0; y < dimensionPx; y++) {
    for (uint32_t x = 0; x < dimensionPx; x++) {
      if (!is_pixel_in_border(pxPerCu, x, y))
        continue;
      uint8_t nibble = get_nibble(nibbleOffset++ % totalNibbles, inputLen, input);
      uint8_t v = nibble * 17;
      output[y * dimensionPx + x] = color_make(v, v, v, 0xFF);
    }
  }

  // overflow region
  if (nibbleOffset < totalNibbles) {
    uint32_t overflowXPx = OVERFLOW_X * pxPerCu;
    uint32_t overflowYPx = OVERFLOW_Y * pxPerCu;
    uint32_t overflowWPx = OVERFLOW_W * pxPerCu;
    uint32_t rowMajorIndex = 0;

    while (nibbleOffset < totalNibbles) {
      uint32_t x = overflowXPx + rowMajorIndex % overflowWPx;
      uint32_t y = overflowYPx + rowMajorIndex / overflowWPx;
      uint8_t nibble = get_nibble(nibbleOffset++ % totalNibbles, inputLen, input);
      uint8_t v = nibble * 17;
      output[y * dimensionPx + x] = color_make(v, v, v, 0xFF);
      rowMajorIndex++;
    }
  }
}

void encoding_decode_card_data(const Color *input, uint32_t dimensionPx, uint8_t *output, uint32_t *outputLen) {
  uint32_t pxPerCu = dimensionPx / DIMENSION_IN_CU;
  uint32_t nibbleCount = calculate_total_usable_pixel_count(pxPerCu);
  uint32_t byteCount = nibbleCount / 2;
  // caller provides output buffer of at least byteCount bytes

  // collect nibbles into output buffer reinterpreted as nibble pairs
  uint32_t nibbleOffset = 0;
  uint8_t pendingNibble = 0;
  uint32_t byteOffset = 0;

  // border region
  for (uint32_t y = 0; y < dimensionPx && nibbleOffset < nibbleCount; y++) {
    for (uint32_t x = 0; x < dimensionPx && nibbleOffset < nibbleCount; x++) {
      if (!is_pixel_in_border(pxPerCu, x, y))
        continue;
      consume_nibble(color_to_nibble(input[y * dimensionPx + x]), &pendingNibble, &nibbleOffset, output, &byteOffset);
    }
  }

  // overflow region
  uint32_t overflowX1Px = OVERFLOW_X * pxPerCu;
  uint32_t overflowY1Px = OVERFLOW_Y * pxPerCu;
  uint32_t overflowX2Px = overflowX1Px + OVERFLOW_W * pxPerCu;
  uint32_t overflowY2Px = overflowY1Px + OVERFLOW_H * pxPerCu;

  for (uint32_t y = overflowY1Px; y < overflowY2Px && nibbleOffset < nibbleCount; y++) {
    for (uint32_t x = overflowX1Px; x < overflowX2Px && nibbleOffset < nibbleCount; x++) {
      consume_nibble(color_to_nibble(input[y * dimensionPx + x]), &pendingNibble, &nibbleOffset, output, &byteOffset);
    }
  }

  // read length prefix from first 4 bytes of decoded payload
  uint32_t size = ((uint32_t)output[0] << 24) | ((uint32_t)output[1] << 16) | ((uint32_t)output[2] << 8) | (uint32_t)output[3];

  // shift output left by 4 to drop the length prefix
  for (uint32_t i = 0; i < size; i++)
    output[i] = output[i + 4];

  *outputLen = size;
}