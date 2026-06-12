#ifndef GLYPH_TABLE_H
#define GLYPH_TABLE_H

#include <stdint.h>

// returns pointer to 5-byte glyph mask, or NULL if undefined
const uint8_t* get_glyph(char c);

#endif // GLYPH_TABLE_H
