#include "render.h"
#include <cstring>

void render_clear(uint8_t* buf504) {
  std::memset(buf504, 0, LCD_BUF);
}

void render_setPixel(uint8_t* b, int x, int y, bool on) {
  if (x < 0 || x >= LCD_W || y < 0 || y >= LCD_H) return;
  const int idx = (y >> 3) * LCD_W + x;
  const uint8_t bit = (uint8_t)(1u << (y & 7));
  if (on) b[idx] |= bit;
  else    b[idx] &= (uint8_t)~bit;
}

void render_drawSprite1bpp(uint8_t* buf504,
                           const uint8_t* sprite,
                           int w, int h,
                           int x, int y)
{
  const int bytesPerRow = (w + 7) / 8;

  for (int sy = 0; sy < h; sy++) {
    for (int sx = 0; sx < w; sx++) {
      int byteIndex = sy * bytesPerRow + (sx >> 3);
      uint8_t v = pgm_read_byte(sprite + byteIndex);
      bool on = (v & (0x80 >> (sx & 7))) != 0;
      if (on) render_setPixel(buf504, x + sx, y + sy, true);
    }
  }
}
