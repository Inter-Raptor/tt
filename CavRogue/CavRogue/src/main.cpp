#include <stdint.h>
#include "core/render.h"

#if defined(CAV_PLATFORM_ESP32)

  #include <Arduino.h>
  #include "core/render.h"
  #include "platform/esp32/esp_display.h"

  static uint8_t g_buf[LCD_BUF];
  static int g_frame = 0;

  void setup() {
    Serial.begin(115200);
    esp_initDisplay();
  }

  void loop() {
    render_test(g_buf, g_frame++);
    esp_showBuffer(g_buf);
    delay(60);
  }

#else // PC

  #include <thread>
  #include <chrono>

  #include "core/game.h"
  #include "platform/pc/pc_display.h"
  #include "platform/pc/pc_input.h"

  static uint8_t g_buf[LCD_BUF];
  static Game g_game;

  int main() {
    game_init(g_game, 12345, 1);

    while (true) {
      InputState in{};
      pc_readInput(in);
      if (in.quit) break;

      game_update(g_game, in);
      game_render(g_game, g_buf);

      pc_showBuffer(g_buf);
      std::this_thread::sleep_for(std::chrono::milliseconds(16)); // 60 FPS
    }
    return 0;
  }

#endif
