#include "pc_display.h"
#include "../../core/render.h"

#include <stdint.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
#endif

static inline bool getPixel(const uint8_t* b, int x, int y) {
  const int idx = (y >> 3) * LCD_W + x;
  const uint8_t bit = (uint8_t)(1u << (y & 7));
  return (b[idx] & bit) != 0;
}

void pc_showBuffer(const uint8_t* buf504) {
#ifdef _WIN32
  static HANDLE hOut = NULL;
  static bool inited = false;

  if (!inited) {
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Cache le curseur (ça enlève une grosse partie du "clignotement")
    CONSOLE_CURSOR_INFO ci;
    ci.dwSize = 1;
    ci.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &ci);

    inited = true;
  }

  // Construit un seul gros buffer texte : (84 + \n) * 48
  static char screen[(LCD_W + 1) * LCD_H + 1];
  char* p = screen;

  for (int y = 0; y < LCD_H; y++) {
    for (int x = 0; x < LCD_W; x++) {
      *p++ = getPixel(buf504, x, y) ? '#' : ' ';
    }
    *p++ = '\n';
  }
  *p = '\0';

  // Réécrit par-dessus depuis le coin haut gauche
  COORD c = {0, 0};
  SetConsoleCursorPosition(hOut, c);

  DWORD written = 0;
  WriteConsoleA(hOut, screen, (DWORD)(p - screen), &written, NULL);

#else
  // Fallback basique
  for (int y = 0; y < LCD_H; y++) {
    for (int x = 0; x < LCD_W; x++) {
      putchar(getPixel(buf504, x, y) ? '#' : ' ');
    }
    putchar('\n');
  }
#endif
}
