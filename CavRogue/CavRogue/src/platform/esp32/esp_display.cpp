#include "esp_display.h"

#if defined(CAV_PLATFORM_ESP32)

  #include <Arduino.h>
  #include <U8g2lib.h>
  #include <SPI.h>
  #include <string.h>

  // ⚠️ Mets tes pins ici
  static const uint8_t PIN_CS  = 5;
  static const uint8_t PIN_DC  = 17;
  static const uint8_t PIN_RST = 16;

  static U8G2_PCD8544_84X48_F_4W_HW_SPI u8g2(U8G2_R0, PIN_CS, PIN_DC, PIN_RST);

  void esp_initDisplay() {
    u8g2.begin();
    u8g2.setContrast(140);
  }

  void esp_showBuffer(const uint8_t* buf504) {
    memcpy(u8g2.getBufferPtr(), buf504, 504);
    u8g2.sendBuffer();
  }

#else

  void esp_initDisplay() {}
  void esp_showBuffer(const uint8_t*) {}

#endif
