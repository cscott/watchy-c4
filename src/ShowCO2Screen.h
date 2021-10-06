#pragma once

#include "Screen.h"

class ShowCO2Screen : public Screen {
 public:
  ShowCO2Screen(uint16_t bg = GxEPD_WHITE);
  void show() override;
};
