#include "WatchyC4.h"

#include <stdlib.h>
#include <time.h>

#include "Roboto_Bold7pt7b.h"
#include "Roboto_Bold24pt7b.h"
#include "RobotoCondensed_Regular7pt7b.h"
#include "RobotoCondensed_Regular10pt7b.h"
#include "c4-calls.h"
#include "c4-icons.h"

#include "OptimaLTStd7pt7b.h"
#include "GetLocation.h"
#include "GetWeather.h"

#define DARKMODE false
#define BG_COLOR (DARKMODE ? GxEPD_BLACK : GxEPD_WHITE)
#define FG_COLOR (DARKMODE ? GxEPD_WHITE : GxEPD_BLACK)

const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;

using namespace Watchy;

// centered in x
void centerjustify(const char *str) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(display.getCursorX() - (w / 2), display.getCursorY());
  display.print(str);
}

// word wrap!
void wordwrap(const char *str, int width, int lineheight, int maxLines) {
  static const char *WS = " \t\r\n\f";
  int x = display.getCursorX(), y = display.getCursorY();
  int16_t x1, y1;
  uint16_t w, h;
  char buf[strlen(str)+1];
  const char *start = str;
  int len, lastLen, lineCount = 0;
  display.setTextWrap(false);
  do {
  len = strcspn(start, WS);
  lastLen = len; // always display at least 1 word
  while(1) {
      strncpy(buf, start, len);
      buf[len] = 0;
      display.getTextBounds(buf, x, y, &x1, &y1, &w, &h);
      if (w >= width) {
          // back up
          len = lastLen;
          buf[len] = 0;
          break;
      }
      if (start[len]=='\0' || start[len]=='\n') {
          break;
      }
      lastLen = len;
      len += strspn(start+len, WS); // skip over whitespace
      len += strcspn(start+len, WS); // find next break
  }
  display.setCursor(x, y);
  display.print(buf);
  // now advance to next line
  y += lineheight;
  start += len;
  start += strspn(start, WS);
  lineCount++;
  } while(*start && lineCount < maxLines);
}

void rightJustify(const char *txt, uint16_t &yPos) {
  int16_t x1, y1;
  uint16_t w, h;
  const uint8_t PADDING = 0; // how much padding to leave around text
  display.getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  // right justify with padding
  display.setCursor(200-x1-w-PADDING, yPos);
  display.print(txt);
}

void WatchyC4::drawCall(tm *t) {
    seed_random(t); // a function of the current time
    int current_def = get_random_int(NUMBER_OF_CALLS);

    const GFXfont *font = &Roboto_Bold7pt7b;
    display.setFont(font); // 14px
    display.setTextWrap(false);
    display.setCursor(DISPLAY_WIDTH/2, 0+13);
    display.setTextColor(FG_COLOR);
    centerjustify(call_list[current_def].call);

    display.setFont(&RobotoCondensed_Regular7pt7b); // 14px
    display.setCursor(0, 16+font->yAdvance);
    wordwrap(call_list[current_def].def, DISPLAY_WIDTH,
             font->yAdvance, 7);
}

void drawTime(tm *t){
    static char buffer[] = "00:00";
    display.setFont(&Roboto_Bold24pt7b); // 49px
    display.setTextWrap(false);
    display.setCursor(DISPLAY_WIDTH/2, DISPLAY_HEIGHT-70+46);
    display.setTextColor(FG_COLOR);
    sprintf(buffer, "%d:%02d", t->tm_hour, t->tm_min);
    centerjustify(buffer);
}

void drawDate(tm *t){
  static char date_text[] = "XxxxxxxxxZZZ 00";
  strftime(date_text, sizeof(date_text), "%B %e", t);
  display.setFont(&RobotoCondensed_Regular10pt7b); // 21px
  display.setTextWrap(false);
  display.setCursor(DISPLAY_WIDTH/2, DISPLAY_HEIGHT-24+19);
  display.setTextColor(FG_COLOR);
  centerjustify(date_text);
}

void drawBattery(){
    const unsigned char* batteryIcon;
    float VBAT = Watchy::getBatteryVoltage();
    // 48x32 px; stick it in bottom left
    if(VBAT > 4.1){
        batteryIcon = icon_battery_4;
    } else if(VBAT > 3.95 && VBAT <= 4.1){
        batteryIcon = icon_battery_3;
    } else if(VBAT > 3.80 && VBAT <= 3.95){
        batteryIcon = icon_battery_2;
    } else if(VBAT > 3.60 && VBAT <= 3.8){
        batteryIcon = icon_battery_1;
    } else {
        batteryIcon = icon_battery_0;
    }
    display.drawBitmap(0, DISPLAY_HEIGHT-WEATHER_ICON_HEIGHT, batteryIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, FG_COLOR);
}

void drawWeather(){
    auto currentWeather = Watchy_GetWeather::getWeather();

    //int8_t temperature = currentWeather.temperature;
    int16_t weatherConditionCode = currentWeather.weatherConditionCode;

    const unsigned char* weatherIcon;

    //https://openweathermap.org/weather-conditions
    if(weatherConditionCode > 801){//Cloudy
    weatherIcon = cloudy;
    }else if(weatherConditionCode == 801){//Few Clouds
    weatherIcon = cloudsun;
    }else if(weatherConditionCode == 800){//Clear
    weatherIcon = sunny;
    }else if(weatherConditionCode >=700){//Atmosphere
    weatherIcon = cloudy;
    }else if(weatherConditionCode >=600){//Snow
    weatherIcon = snow;
    }else if(weatherConditionCode >=500){//Rain
    weatherIcon = rain;
    }else if(weatherConditionCode >=300){//Drizzle
    weatherIcon = rain;
    }else if(weatherConditionCode >=200){//Thunderstorm
    weatherIcon = rain;
    }else
    return;
    // 48x32 px, so stick it in bottom right
    display.drawBitmap(DISPLAY_WIDTH-WEATHER_ICON_WIDTH, DISPLAY_HEIGHT-WEATHER_ICON_HEIGHT, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, FG_COLOR);
}

void drawCO2Separator(){
  // Separator line / CO2 gauge
  // 60px = 1000ppm, range is 0-3000pm. 0-1000=green, 1000-2000=yellow
  // See https://www.werma.com/en/products/system/co2_traffic_light.php
  const int GAUGE_POS = DISPLAY_HEIGHT - 63;
  const int GAUGE_SEG_WIDTH = 60;
  const int GAUGE_OFFSET = (DISPLAY_WIDTH - 3*GAUGE_SEG_WIDTH) / 2;
  const int TICK_HEIGHT = 2;
  for (int i=1; i<=2; i++) {
      display.drawLine(GAUGE_OFFSET+(i*GAUGE_SEG_WIDTH), GAUGE_POS - TICK_HEIGHT,
                       GAUGE_OFFSET+(i*GAUGE_SEG_WIDTH), GAUGE_POS + TICK_HEIGHT,
                       FG_COLOR );
  }
  for (int i=GAUGE_POS-1; i<=GAUGE_POS+1; i++) {
      display.drawLine(GAUGE_OFFSET, i,
                       GAUGE_OFFSET + 3*GAUGE_SEG_WIDTH, i,
                       i==GAUGE_POS ? BG_COLOR : FG_COLOR);
  }
  // for now, draw battery voltage on this graph
  float VBAT = Watchy::getBatteryVoltage();
  float percent = (VBAT-3.6)/(4.2-3.6);
  if (percent > 1) { percent = 1; }
  if (percent < 0) { percent = 0; }
  display.drawLine(GAUGE_OFFSET, GAUGE_POS,
                   GAUGE_OFFSET + (int)((3*GAUGE_SEG_WIDTH)*percent), GAUGE_POS,
                   FG_COLOR);
}

void WatchyC4::show() {
  tm t;
  time_t tt = now();
  localtime_r(&tt, &t);

  Watchy::display.fillScreen(BG_COLOR);
  Watchy::display.setTextColor(FG_COLOR);

  drawCO2Separator();
  drawWeather();
  drawBattery();
  drawTime(&t);
  drawDate(&t);
  drawCall(&t);
}


// RNG stuff
void WatchyC4::seed_random(tm *t) {
  // overflow is fine here. seed is ~ # secs since start of month
  // seconds are quantized to 10 second intervals
  lfsr=(t->tm_sec/10 + 60*(t->tm_min + 60*(t->tm_hour + 24*t->tm_mday)));
  if (lfsr==0) { lfsr++; }
}

/* generate a uniform random number in the range [0,1] */
unsigned short WatchyC4::get_random_bit() {
  /* 16-bit galois LFSR, period 65535. */
  /* see http://en.wikipedia.org/wiki/Linear_feedback_shift_register */
  /* taps: 16 14 13 11; feedback polynomial: x^16 + x^14 + x^13 + x^11 + 1 */
  unsigned short out = lfsr & 1u;
  lfsr = (lfsr >> 1) ^ (-(out) & 0xB400u);
  return out;
}

/* generate a uniform random number in the range [0, 2^n) */
uint16_t WatchyC4::get_random_bits(unsigned short n) {
  uint16_t out = 0;
  while (n--) { out = (out << 1) | get_random_bit(); }
  return out;
}

/* generate a uniform random number in the range [0, max) */
// max should be in range (1,1024]
uint16_t WatchyC4::get_random_int(uint16_t max) {
  uint16_t val;
  uint16_t low;
  low = 1024 % max;
  do {
    // this loop is necessary to ensure the numbers are uniformly
    // distributed.
    val = get_random_bits(10);
  } while (val < low);
  return val % max;
}
