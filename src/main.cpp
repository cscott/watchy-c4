#include <Watchy.h>

#include "WatchyC4.h"

WatchyC4 watchyC4;

void setup() {
  // only set it if not already set so that changing
  // screens persists over deep sleep
  if (Watchy::screen == nullptr) {
    Watchy::screen = &watchyC4;
  }
  Watchy::init();
}

void loop() {}
