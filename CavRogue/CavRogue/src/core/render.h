#pragma once
#include <stdint.h>

static const int LCD_W = 84;
static const int LCD_H = 48;
static const int LCD_BUF = 504; // 84*48/8

void render_clear(uint8_t* buf504);
void render_setPixel(uint8_t* b, int x, int y, bool on);
void render_test(uint8_t* buf504, int frame);
void render_drawBitmapRowMajor(uint8_t* buf, int x0, int y0, const uint8_t* bmp, int w, int h);
void render_clear(uint8_t* buf504);
void render_drawSprite1bpp(uint8_t* buf504,
                           const uint8_t* sprite,
                           int w, int h,
                           int x, int y);