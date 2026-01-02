#pragma once
#include <stdint.h>

#if defined(CAV_PLATFORM_ESP32)
  #include <pgmspace.h>
#else
  // PC: PROGMEM = rien, lecture directe
  #ifndef PROGMEM
    #define PROGMEM
  #endif
  #ifndef pgm_read_byte
    #define pgm_read_byte(addr) (*(const uint8_t*)(addr))
  #endif
#endif
