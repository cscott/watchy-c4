#include "Watchy_C4.h"
#include "Roboto_Bold7pt7b.h"
#include "Roboto_Bold24pt7b.h"
#include "RobotoCondensed_Regular7pt7b.h"
#include "RobotoCondensed_Regular10pt7b.h"
#include "c4calls.h"
#include "icons.h"


#define DARKMODE false
#define BG_COLOR (DARKMODE ? GxEPD_BLACK : GxEPD_WHITE)
#define FG_COLOR (DARKMODE ? GxEPD_WHITE : GxEPD_BLACK)

const uint8_t BATTERY_SEGMENT_WIDTH = 7;
const uint8_t BATTERY_SEGMENT_HEIGHT = 11;
const uint8_t BATTERY_SEGMENT_SPACING = 9;
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;

const uint8_t DIVIDER = 100;

WatchyC4::WatchyC4(){} //constructor

void WatchyC4::drawWatchFace(){
    display.fillScreen(BG_COLOR);
    display.setTextColor(FG_COLOR);
    for (int i=62; i<=63; i++) {
        display.drawLine(8, DISPLAY_HEIGHT-i,
                         DISPLAY_WIDTH-8, DISPLAY_HEIGHT-i,
                         FG_COLOR);
    }
    drawTime();
    drawDate();
    drawCall();
    drawWeather();
    /*
    drawSteps();
    drawBattery();
    display.drawBitmap(120, 77, WIFI_CONFIGURED ? wifi : wifioff, 26, 18, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    if(BLE_CONFIGURED){
        display.drawBitmap(100, 75, bluetooth, 13, 21, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
    */
}

// centered in x
void WatchyC4::centerjustify(const char *str) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(display.getCursorX() - (w / 2), display.getCursorY());
  display.print(str);
}

// word wrap!
void WatchyC4::wordwrap(const char *str, int width, int lineheight, int maxLines) {
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
  } while(start[len] && lineCount <= maxLines);
}

void WatchyC4::drawCall() {
    seed_random(); // a function of the current time
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

void WatchyC4::drawTime(){
    static char buffer[] = "00:00";
    display.setFont(&Roboto_Bold24pt7b); // 49px
    display.setTextWrap(false);
    display.setCursor(DISPLAY_WIDTH/2, DISPLAY_HEIGHT-70+46);
    display.setTextColor(FG_COLOR);
    sprintf(buffer, "%d:%02d", currentTime.Hour, currentTime.Minute);
    centerjustify(buffer);
}

void WatchyC4::drawDate(){
  static char date_text[] = "XxxxxxxxxZZZ 00";
  sprintf(date_text, "%s %d", monthStr(currentTime.Month), currentTime.Day);
  display.setFont(&RobotoCondensed_Regular10pt7b); // 21px
  display.setTextWrap(false);
  display.setCursor(DISPLAY_WIDTH/2, DISPLAY_HEIGHT-24+19);
  display.setTextColor(FG_COLOR);
  centerjustify(date_text);
}

void WatchyC4::drawSteps(){
    uint32_t stepCount = sensor.getCounter();
    display.drawBitmap(10, 165, steps, 19, 23, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.setCursor(35, 190);
    display.println(stepCount);
}
void WatchyC4::drawBattery(){
    display.drawBitmap(154, 73, battery, 37, 21, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.fillRect(159, 78, 27, BATTERY_SEGMENT_HEIGHT, DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);//clear battery segments
    int8_t batteryLevel = 0;
    float VBAT = getBatteryVoltage();
    if(VBAT > 4.1){
        batteryLevel = 3;
    }
    else if(VBAT > 3.95 && VBAT <= 4.1){
        batteryLevel = 2;
    }
    else if(VBAT > 3.80 && VBAT <= 3.95){
        batteryLevel = 1;
    }    
    else if(VBAT <= 3.80){
        batteryLevel = 0;
    }

    for(int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++){
        display.fillRect(159 + (batterySegments * BATTERY_SEGMENT_SPACING), 78, BATTERY_SEGMENT_WIDTH, BATTERY_SEGMENT_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
}

void WatchyC4::drawWeather(){

    weatherData currentWeather = getWeatherData();

    int8_t temperature = currentWeather.temperature;
    int16_t weatherConditionCode = currentWeather.weatherConditionCode;

    /*
    //display.setFont(&DSEG7_Classic_Regular_39);
    int16_t  x1, y1;
    uint16_t w, h;
    display.getTextBounds(String(temperature), 100, 150, &x1, &y1, &w, &h);
    display.setCursor(155 - w, 150);
    display.println(temperature);
    display.drawBitmap(165, 110, strcmp(TEMP_UNIT, "metric") == 0 ? celsius : fahrenheit, 26, 20, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    */
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


// RNG stuff
void WatchyC4::seed_random(void) {
  // overflow is fine here. seed is ~ # secs since start of month
  // seconds are quantized to 10 second intervals
  lfsr=(currentTime.Second/10 + 60*(currentTime.Minute + 60*(currentTime.Hour + 24*currentTime.Day)));
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
