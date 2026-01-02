#include <stdint.h>
#include <string.h>

#include "raylib.h"

// Inclure tes headers existants
#include "../src/core/render.h"
#include "../src/platform/pc/pc_input.h"  // si tu veux garder ton InputState

static inline bool getPixel(const uint8_t* b, int x, int y) {
  const int idx = (y >> 3) * LCD_W + x;
  const uint8_t bit = (uint8_t)(1u << (y & 7));
  return (b[idx] & bit) != 0;
}

int main() {
  const int scale = 10; // taille des pixels à l’écran
  InitWindow(LCD_W * scale, LCD_H * scale, "CavRogue (PC)");
  SetTargetFPS(60);

  static uint8_t buf[LCD_BUF];

  int px = LCD_W / 2;
  int py = LCD_H / 2;

  while (!WindowShouldClose()) {
    // input (comme pygame)
    if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) px--;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) px++;
    if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) py--;
    if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) py++;

    if (px < 0) px = 0; if (px > LCD_W - 1) px = LCD_W - 1;
    if (py < 0) py = 0; if (py > LCD_H - 1) py = LCD_H - 1;

    // clear buffer + dessine un joueur 3x3
    memset(buf, 0, sizeof(buf));
    for (int y = -1; y <= 1; y++)
      for (int x = -1; x <= 1; x++)
        render_setPixel(buf, px + x, py + y, true);

    // rendu fenêtre
    BeginDrawing();
    ClearBackground(BLACK);

    for (int y = 0; y < LCD_H; y++) {
      for (int x = 0; x < LCD_W; x++) {
        if (getPixel(buf, x, y)) {
          DrawRectangle(x * scale, y * scale, scale, scale, RAYWHITE);
        }
      }
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
