#include "pc_input.h"

#ifdef _WIN32
  #include <conio.h>
#endif

void pc_readInput(InputState& in) {
  in = {}; // reset

#ifdef _WIN32
  while (_kbhit()) {
    int c = _getch();

    if (c == 27 || c == 'q' || c == 'Q') { // ESC ou Q
      in.quit = true;
      return;
    }

    // Flèches (touches étendues)
    if (c == 0 || c == 224) {
      int k = _getch();
      if (k == 75) in.left  = true;
      if (k == 77) in.right = true;
      if (k == 72) in.up    = true;
      if (k == 80) in.down  = true;
    }

    if (c == ' ') in.a = true;
    if (c == 'x' || c == 'X') in.b = true;
  }
#endif
}
