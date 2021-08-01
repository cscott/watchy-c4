#include "Watchy_C4.h"
#include "Roboto_Bold7pt7b.h"
#include "Roboto_Bold24pt7b.h"
#include "RobotoCondensed_Regular10pt7b.h"
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
    /*
    drawSteps();
    drawWeather();
    drawBattery();
    display.drawBitmap(120, 77, WIFI_CONFIGURED ? wifi : wifioff, 26, 18, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    if(BLE_CONFIGURED){
        display.drawBitmap(100, 75, bluetooth, 13, 21, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
    */
}

// centered in both x and y
void WatchyC4::centerjustify(const char *str) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(display.getCursorX() - (w / 2), display.getCursorY());
  display.print(str);
}

void WatchyC4::drawCall() {
    display.setFont(&Roboto_Bold7pt7b); // 14px
    display.setTextWrap(false);
    display.setCursor(DISPLAY_WIDTH/2, 0+18);
    display.setTextColor(FG_COLOR);
    centerjustify("spin the top");
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

    //display.setFont(&DSEG7_Classic_Regular_39);
    int16_t  x1, y1;
    uint16_t w, h;
    display.getTextBounds(String(temperature), 100, 150, &x1, &y1, &w, &h);
    display.setCursor(155 - w, 150);
    display.println(temperature);
    display.drawBitmap(165, 110, strcmp(TEMP_UNIT, "metric") == 0 ? celsius : fahrenheit, 26, 20, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
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
    display.drawBitmap(145, 158, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
}
