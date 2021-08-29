#pragma once

#include "Screen.h"

class WatchyC4 : public Screen {
 public:
  WatchyC4(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
 private:
  void drawCall(tm *t);

 private:
  uint16_t lfsr = 0xAACEu;
  void seed_random(tm *t);
  unsigned short get_random_bit();
  uint16_t get_random_bits(unsigned short n);
  uint16_t get_random_int(uint16_t max);
};
