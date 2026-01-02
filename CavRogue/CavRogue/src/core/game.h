#pragma once
#include <stdint.h>

struct InputState {
  bool left=false, right=false, up=false, down=false;
  bool a=false, b=false;
  bool quit=false;
};

enum class GameMode : uint8_t { INTRO, PLAY };

struct Game {
  GameMode mode = GameMode::INTRO;
  int frame = 0;

  int px = 34, py = 16;  // player pos
  int ex = 56, ey = 16;  // enemy pos
  int enemyType = 0;
};

void game_init(Game& g, uint32_t seed, int difficulty);
void game_update(Game& g, const InputState& in);
void game_render(const Game& g, uint8_t* buf504);
