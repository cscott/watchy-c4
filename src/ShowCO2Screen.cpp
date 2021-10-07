#include "ShowCO2Screen.h"

#include <time.h>

#include "fonts.h"
#include "Watchy.h"

extern uint16_t getLastCO2(void);
extern float getLastTemperature(void);
extern float getLastHumidity(void);

//#define WEEKLY_GRAPH
#define DAILY_GRAPH
//#define HOURLY_GRAPH // useful for debugging

#if defined(WEEKLY_GRAPH)
const time_t updateInterval = SECS_PER_HOUR;
const int co2Readingssz = 24 * 7;
const int graphDivisions = 7; // days
#elif defined(DAILY_GRAPH)
const time_t updateInterval = 10 * 60 /*SECS_PER_MINUTE*/; // 10 minutes
const int co2Readingssz = 24 * 60 * 60 / updateInterval;
const int graphDivisions = 24; // hours
#elif defined(HOURLY_GRAPH)
const time_t updateInterval = 20; // 20 seconds
const int co2Readingssz = 60 * 60 / updateInterval;
const int graphDivisions = 4; // 15 minutes
#endif
static RTC_DATA_ATTR uint16_t co2ReadingMin[co2Readingssz];
static RTC_DATA_ATTR uint16_t co2ReadingMax[co2Readingssz];
static RTC_DATA_ATTR int nextReading = 0;
static RTC_DATA_ATTR time_t lastReadingTS = 0;

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

static int mapReading(uint16_t reading, const int16_t height) {
    return min(int(mapRange(reading, 3000, 0, height, 0) + 0.5), int(height));
}

void printfcomma(uint16_t n) {
}

void ShowCO2Screen::show() {
  float fgColor = (bgColor == GxEPD_WHITE ? GxEPD_BLACK : GxEPD_WHITE);
  Watchy::display.fillScreen(bgColor);
  Watchy::display.setFont(&RobotoCondensed_Regular10pt7b);
  const int headingX = 40, headingY = 24;
  Watchy::display.setCursor(headingX, headingY);
  uint16_t co2_reading = getLastCO2();
  Watchy::display.printf("CO  ");
  if (co2_reading < 1000) {
      Watchy::display.printf("%5d", co2_reading);
  } else {
      Watchy::display.printf("%1d,%03d", co2_reading/1000, co2_reading%1000);
  }
  Watchy::display.printf(" ppm");
  Watchy::display.setFont(&RobotoCondensed_Regular7pt7b);
  const int xoffset = 23, yoffset = 4;
  Watchy::display.setCursor(headingX + xoffset, headingY + yoffset);
  Watchy::display.printf("2");
  //Watchy::display.printf(co2_reading <= 1000 ? "(Good)" : (co2_reading <= 2000 : "(okay)" : "(bad)"));
  Watchy::display.setCursor(40, 200-3);
  Watchy::display.setFont(&RobotoCondensed_Regular7pt7b);
  Watchy::display.printf("%3.0f deg F %2.0f%% Humidity\n", (getLastTemperature()*9/5)+32, getLastHumidity());
  // draw the history graph
  const int16_t height = 144;
  const int16_t width = co2Readingssz;
  const int16_t margin = (200-width)/2;
  const int16_t x0 = margin + 1;  // left edge of graph
  const int16_t y0 = DISPLAY_HEIGHT - 16/*bottom margin*/ - height - 1;  // top of graph
  // draw a border around it
  Watchy::display.drawRect(x0 - 1, y0 - 1, width + 2, height + 2, fgColor);
  // draw dotted lines for 1000 (green-yellow) and 2000 (yellow-red)
  for (int i = 0; i < co2Readingssz; i+=2) {
      Watchy::display.drawPixel(x0 + i, y0 + height - mapReading(1000, height), fgColor);
      Watchy::display.drawPixel(x0 + i, y0 + height - mapReading(2000, height), fgColor);
  }
  // draw xaxis ticks
  for (int i = 1; i<graphDivisions; i++) {
      int x = x0 + int((float(i)*co2Readingssz/graphDivisions)+0.5);
      Watchy::display.drawLine(x, y0, x, y0 - 2, fgColor);
      Watchy::display.drawLine(x, y0 + height - 1, x, y0 + height + 1, fgColor);
  }
  for (int i = 0; i < co2Readingssz; i++) {
      const uint16_t vMin = co2ReadingMin[(nextReading + i + 1) % co2Readingssz];
      const uint16_t vMax = co2ReadingMax[(nextReading + i + 1) % co2Readingssz];
      // map [3000 .. 0) onto [height..0)
      const int hMin = mapReading(vMin, height);
      const int hMax = mapReading(vMax, height);

      if (hMin != 0 && hMax != 0) { // suppress uninit values
          Watchy::display.drawLine(x0 + i, y0 + height - hMax, x0 + i, y0 + height - hMin,
                                  fgColor);
      }
  }
}
