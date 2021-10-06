#include "ShowCO2Screen.h"

#include <time.h>

#include "fonts.h"
#include "Watchy.h"

extern uint16_t getLastCO2(void);
extern float getLastTemperature(void);
extern float getLastHumidity(void);

const int co2Readingssz = 24 * 7;
static RTC_DATA_ATTR uint16_t co2ReadingMin[co2Readingssz];
static RTC_DATA_ATTR uint16_t co2ReadingMax[co2Readingssz];
static RTC_DATA_ATTR int nextReading = 0;
static RTC_DATA_ATTR time_t lastReadingTS = 0;
const time_t updateInterval = SECS_PER_HOUR;

static void OnWake(esp_sleep_wakeup_cause_t wakeup_reason) {
    // setup
    uint16_t last_co2 = getLastCO2();
    if ((lastReadingTS != 0) && (now() < lastReadingTS + updateInterval)) {
        // just update min and max of current reading
        if (last_co2 < co2ReadingMin[nextReading]) {
            co2ReadingMin[nextReading] = last_co2;
        }
        if (last_co2 > co2ReadingMax[nextReading]) {
            co2ReadingMax[nextReading] = last_co2;
        }
        return;  // hasn't been an updateInterval yet
    }
    do {
        nextReading = (nextReading + 1) % co2Readingssz;
        co2ReadingMin[nextReading] = last_co2;
        co2ReadingMax[nextReading] = last_co2;
        if (lastReadingTS == 0) {
            // first time
            lastReadingTS = now();
        } else {
            // take another reading one updateInterval after the last one
            lastReadingTS += updateInterval;
        }
        // catch up in case we slept for a very long time
        // XXX handle case where we just set to RTC and our interval
        // is off by a *lot*!
    } while (now() >= lastReadingTS + updateInterval);
}

ShowCO2Screen::ShowCO2Screen(uint16_t bg) : Screen(bg) {
  Watchy::AddOnWakeCallback(&OnWake);
}

static float mapRange(const float v, const float inMax, const float inMin,
               const float outMax, const float outMin) {
  return (min(inMax, max(inMin, v)) - inMin) / (inMax - inMin) *
             (outMax - outMin) +
         outMin;
}

void ShowCO2Screen::show() {
  float fgColor = (bgColor == GxEPD_WHITE ? GxEPD_BLACK : GxEPD_WHITE);
  Watchy::display.fillScreen(bgColor);
  Watchy::display.setFont(&RobotoCondensed_Regular10pt7b);
  Watchy::display.setCursor(40, 38); // y=25 is good if you want to squeeze in another line
  uint16_t co2_reading = getLastCO2();
  Watchy::display.printf("CO2 %4d ppm", co2_reading);
  Watchy::display.setFont(&RobotoCondensed_Regular7pt7b);
  //Watchy::display.printf(co2_reading <= 1000 ? "(Good)" : (co2_reading <= 2000 : "(okay)" : "(bad)"));
  Watchy::display.setCursor(40, 200-3);
  Watchy::display.setFont(&RobotoCondensed_Regular7pt7b);
  Watchy::display.printf("%3.0f deg F %2.0f%% Humidity\n", (getLastTemperature()*9/5)+32, getLastHumidity());
  // draw the history graph
  const int16_t margin = 16;
  const int16_t height = 120;
  const int16_t width = co2Readingssz;
  const int16_t x0 = margin + 1;  // left edge of graph
  const int16_t y0 = DISPLAY_HEIGHT - margin - height - 1;  // top of graph
  // draw a border around it
  Watchy::display.drawRect(x0 - 1, y0 - 1, width + 2, height + 2, fgColor);
  for (int i = 0; i < co2Readingssz; i++) {
      const uint16_t vMin = co2ReadingMin[(nextReading + i + 1) % co2Readingssz];
      const uint16_t vMax = co2ReadingMax[(nextReading + i + 1) % co2Readingssz];
      // map [3000 .. 0) onto [100..0)
      const int h = min(int(mapRange(vMin, 3000, 0, height, 0.0) + 0.5), 100);
      if (h > 0) {
          Watchy::display.drawLine(x0 + i, y0 + height - h, x0 + i, y0 + height,
                                  fgColor);
      }
      // also display max
      const int h2 = min(int(mapRange(vMax, 3000, 0, height, 0.0) + 0.5), 100);
      if (h2 > 0) {
          Watchy::display.drawPixel(x0 + i, y0 + height - h2, fgColor);
      }
  }
}
