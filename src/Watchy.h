#ifndef WATCHY_H
#define WATCHY_H

#include <Arduino.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <DS3232RTC.h>
#include <GxEPD2_BW.h>
#include <Wire.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "DSEG7_Classic_Bold_53.h"
#include "BLE.h"
#include "bma.h"
#include "config.h"

class Screen;

typedef struct weatherData{
    int8_t temperature;
    int16_t weatherConditionCode;
}weatherData;

namespace Watchy {
    extern DS3232RTC RTC;
    extern GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display;
    extern tmElements_t currentTime;
    extern Screen *screen;
    extern Screen *watchFace;
    void init(String datetime = "");
    void deepSleep();
    float getBatteryVoltage();
    void vibMotor(uint8_t intervalMs = 100, uint8_t length = 20);

    void handleButtonPress();
    void showMenu(byte menuIndex, bool partialRefresh);
    void showFastMenu(byte menuIndex);
    void showBattery();
    void showBuzz();
    void showAccelerometer();
    void showUpdateFW();
    void setTime();
    void setupWifi();
    bool connectWiFi();
    weatherData getWeatherData();
    void updateFWBegin();

    void showWatchFace(bool partialRefresh);
    void setScreen(Screen *s, bool partialRefresh = false);
};

extern RTC_DATA_ATTR int guiState;
extern RTC_DATA_ATTR int menuIndex;
extern RTC_DATA_ATTR BMA423 sensor;
extern RTC_DATA_ATTR bool WIFI_CONFIGURED;
extern RTC_DATA_ATTR bool BLE_CONFIGURED;

#endif