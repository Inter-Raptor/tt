#include "game.h"
#include "render.h"     // doit fournir LCD_W, LCD_H, LCD_BUF et render_setPixel(...) / render_clear(...)
#include <stdlib.h>
#include <string.h>

#ifndef LCD_W
#define LCD_W 84
#endif
#ifndef LCD_H
#define LCD_H 48
#endif
#ifndef LCD_BUF
#define LCD_BUF 504
#endif

// ---------- RNG simple (déterministe, léger ESP32) ----------
struct Rng {
  uint32_t s;
  explicit Rng(uint32_t seed): s(seed?seed:1u) {}
  uint32_t next() { s ^= s<<13; s ^= s>>17; s ^= s<<5; return s; }
  int irand(int a, int b) { // inclusif
    uint32_t r = next();
    return a + (int)(r % (uint32_t)(b - a + 1));
  }
  bool coin() { return (next() & 1u) != 0; }
};

static inline int clampi(int v, int a, int b){ return v<a?a:(v>b?b:v); }

static inline int idx(const Game& g, int x, int y){ return y*g.w + x; }
static inline bool inb(const Game& g, int x, int y){ return (x>=0 && y>=0 && x<g.w && y<g.h); }

static void carveCircle(Game& g, int cx, int cy, int r){
  int x0 = clampi(cx-r, 0, g.w-1), x1 = clampi(cx+r, 0, g.w-1);
  int y0 = clampi(cy-r, 0, g.h-1), y1 = clampi(cy+r, 0, g.h-1);
  int rr = r*r;
  for(int y=y0;y<=y1;y++){
    for(int x=x0;x<=x1;x++){
      int dx=x-cx, dy=y-cy;
      if(dx*dx+dy*dy <= rr){
        g.grid[idx(g,x,y)] = 1; // sol
      }
    }
  }
}

// “ver” entre deux points : avance en cassant un peu la trajectoire
static void carveWorm(Game& g, Rng& rng, int x0,int y0,int x1,int y1){
  int x=x0, y=y0;
  for(int steps=0; steps< (g.w+g.h); steps++){
    int dx = x1 - x;
    int dy = y1 - y;
    if(dx==0 && dy==0) break;

    // priorité à l’axe dominant, avec une petite chance de zigzag
    if (abs(dx) >= abs(dy)) {
      x += (dx>0)?1:-1;
      if (rng.irand(0,100) < 25) y += rng.coin()?1:-1;
    } else {
      y += (dy>0)?1:-1;
      if (rng.irand(0,100) < 25) x += rng.coin()?1:-1;
    }

    x = clampi(x, 1, g.w-2);
    y = clampi(y, 1, g.h-2);

    int r = rng.irand(2, 5);
    carveCircle(g, x, y, r);
  }
}

static void generateCave(Game& g){
  // tailles “à la CavRogue” (comme ton python : map_step = min((level+1)//2, 30))
  int map_step = (g.level + 1) / 2;
  if(map_step < 1) map_step = 1;
  if(map_step > 30) map_step = 30;

  // W/H = viewport 84x48
  int target_w = LCD_W * map_step;
  int target_h = LCD_H * map_step;

  // limites raisonnables (tu ajusteras plus tard si besoin)
  const int MAX_W = 1400;
  const int MAX_H = 900;

  g.w = clampi(target_w, LCD_W, MAX_W);
  g.h = clampi(target_h, LCD_H, MAX_H);

  g.grid = (uint8_t*)malloc((size_t)g.w * (size_t)g.h);
  memset(g.grid, 0, (size_t)g.w * (size_t)g.h); // tout mur

  // seed “level_seed” : on mélange seed+level
  uint32_t mix = (uint32_t)g.seed * 2654435761u ^ (uint32_t)g.level * 97531u ^ 0xA53C9E1Du;
  Rng rng(mix);

  int margin = 14;
  int mode = rng.irand(0,3); // 0 LR,1 RL,2 TB,3 BT

  int sx,sy, ex,ey;
  if(mode==0){ // LR
    sx = margin; sy = rng.irand(margin, g.h-1-margin);
    ex = g.w-1-margin; ey = rng.irand(margin, g.h-1-margin);
  } else if(mode==1){ // RL
    sx = g.w-1-margin; sy = rng.irand(margin, g.h-1-margin);
    ex = margin; ey = rng.irand(margin, g.h-1-margin);
  } else if(mode==2){ // TB
    sx = rng.irand(margin, g.w-1-margin); sy = margin;
    ex = rng.irand(margin, g.w-1-margin); ey = g.h-1-margin;
  } else { // BT
    sx = rng.irand(margin, g.w-1-margin); sy = g.h-1-margin;
    ex = rng.irand(margin, g.w-1-margin); ey = margin;
  }

  g.px=sx; g.py=sy;
  g.ex=ex; g.ey=ey;

  // on creuse un “grand ver” + quelques branches
  carveCircle(g, sx, sy, 6);
  carveCircle(g, ex, ey, 6);
  carveWorm(g, rng, sx, sy, ex, ey);

  int branches = rng.irand(6, 14);
  for(int i=0;i<branches;i++){
    int ax = rng.irand(margin, g.w-1-margin);
    int ay = rng.irand(margin, g.h-1-margin);
    int bx = rng.irand(margin, g.w-1-margin);
    int by = rng.irand(margin, g.h-1-margin);
    carveWorm(g, rng, ax, ay, bx, by);
  }
}

static inline bool isWall(const Game& g, int x,int y){
  if(!inb(g,x,y)) return true;
  return g.grid[idx(g,x,y)] == 0;
}

static void updateCamera(Game& g){
  g.camx = g.px - LCD_W/2;
  g.camy = g.py - LCD_H/2;
  g.camx = clampi(g.camx, 0, g.w - LCD_W);
  g.camy = clampi(g.camy, 0, g.h - LCD_H);
}

void game_init(Game& g, int seed, int level){
  g.seed = seed;
  g.level = level;
  g.frame = 0;
  g.grid = nullptr;
  generateCave(g);
  updateCamera(g);
}

void game_free(Game& g){
  if(g.grid){
    free(g.grid);
    g.grid = nullptr;
  }
}

void game_update(Game& g, const InputState& in){
  int dx=0, dy=0;
  if(in.left) dx=-1;
  else if(in.right) dx=+1;
  else if(in.up) dy=-1;
  else if(in.down) dy=+1;

  int nx = g.px + dx;
  int ny = g.py + dy;
  if(dx!=0 || dy!=0){
    if(!isWall(g, nx, ny)){
      g.px = nx; g.py = ny;
    }
  }

  // sortie -> niveau suivant
  if (abs(g.px - g.ex) <= 1 && abs(g.py - g.ey) <= 1) {
    int newSeed = g.seed;
    int newLevel = g.level + 1;
    game_free(g);
    game_init(g, newSeed, newLevel);
    return;
  }

  updateCamera(g);
  g.frame++;
}

static void drawPlayer(uint8_t* buf, int x,int y){
  // petit sprite 5x5 “croix” (on mettra tes vrais sprites après)
  for(int i=-2;i<=2;i++){
    render_setPixel(buf, x+i, y, true);
    render_setPixel(buf, x, y+i, true);
  }
}

static void drawExit(uint8_t* buf, int x,int y){
  // petit “O”
  for(int i=-2;i<=2;i++){
    render_setPixel(buf, x+i, y-2, true);
    render_setPixel(buf, x+i, y+2, true);
    render_setPixel(buf, x-2, y+i, true);
    render_setPixel(buf, x+2, y+i, true);
  }
}

void game_render(Game& g, uint8_t* buf504){
  render_clear(buf504);

  // dessiner les murs du viewport (84x48)
  for(int sy=0; sy<LCD_H; sy++){
    for(int sx=0; sx<LCD_W; sx++){
      int mx = g.camx + sx;
      int my = g.camy + sy;
      if(isWall(g,mx,my)){
        // pixel mur = allumé
        render_setPixel(buf504, sx, sy, true);
      }
    }
  }

  // exit + player en coords écran
  int exs = g.ex - g.camx;
  int eys = g.ey - g.camy;
  if(exs>=0 && eys>=0 && exs<LCD_W && eys<LCD_H) drawExit(buf504, exs, eys);

  int pxs = g.px - g.camx;
  int pys = g.py - g.camy;
  if(pxs>=0 && pys>=0 && pxs<LCD_W && pys<LCD_H) drawPlayer(buf504, pxs, pys);
}
