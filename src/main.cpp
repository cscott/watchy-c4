#include "Screens/BuzzScreen.h"
#include "Screens/CarouselScreen.h"
#include "GetLocation.h"
#include "Screens/GetWeatherScreen.h"
#include "Screens/IconScreen.h"
#include "Screens/ImageScreen.h"
#include "Screens/MenuScreen.h"
#include "Screens/OptimaLTStd22pt7b.h"
#include "Screens/SetLocationScreen.h"
#include "Screens/SetTimeScreen.h"
#include "Screens/SetupWifiScreen.h"
#include "Screens/ShowBatteryScreen.h"
#include "Screens/ShowBluetoothScreen.h"
#include "Screens/ShowOrientationScreen.h"
#include "Screens/ShowStepsScreen.h"
#include "Screens/ShowWifiScreen.h"
#include "SyncTime.h"
#include "Screens/SyncTimeScreen.h"
#include "Screens/TimeScreen.h"
#include "Watchy.h"
#include "WatchyErrors.h"
#include "Screens/WeatherScreen.h"
#include "Screens/icons.h"

#include "WatchyC4.h"
#include "ShowCO2Screen.h"
#include <SensirionI2CScd4x.h>

#include <time.h>

SetTimeScreen setTimeScreen;
SetupWifiScreen setupWifiScreen;
SyncTimeScreen syncTimeScreen;
SetLocationScreen setLocationScreen;
GetWeatherScreen getWeatherScreen;
BuzzScreen buzzScreen;

static const uint8_t co2_icon[] = {
76,4,88,12,81,18,76,9,4,9,73,7,10,7,71,6,14,6,69,6,16,6,67,6,18,6,66,5,20,5,65,5,22,5,64,4,10,5,9,4,64,3,10,7,9,3,63,4,9,3,3,3,8,4,62,3,9,3,5,3,8,3,62,3,9,2,7,2,8,3,62,3,8,3,7,3,7,3,61,3,9,3,7,3,8,3,60,3,9,3,7,3,8,3,60,3,9,3,7,3,8,3,60,3,9,3,7,3,8,3,61,3,9,2,7,2,8,3,62,3,9,3,5,3,8,3,62,3,10,3,3,3,9,3,62,4,10,7,9,4,62,4,11,5,10,3,62,6,24,4,61,8,22,5,60,10,20,5,60,5,1,6,18,6,59,5,3,6,16,6,59,5,5,6,14,6,50,6,3,5,7,7,10,7,48,16,8,10,4,9,47,17,8,22,47,20,6,6,2,12,49,22,4,6,7,4,52,24,2,6,63,32,64,31,64,11,6,14,65,10,9,11,65,10,10,10,66,9,4,4,4,9,66,9,3,6,3,9,65,10,3,19,64,10,3,19,64,10,3,19,64,10,3,19,64,10,3,19,64,10,3,6,3,10,65,9,4,4,4,9,66,10,11,9,66,11,9,10,66,12,6,11,67,29,66,29,66,30,65,30,65,6,2,22,65,6,4,20,54,4,7,6,6,17,52,12,2,6,6,16,51,22,6,6,2,6,52,9,4,10,6,6,60,7,10,7,5,6,60,6,14,6,3,6,60,6,16,6,1,6,60,6,18,11,61,5,20,9,61,5,22,7,62,4,10,5,9,5,63,3,10,7,9,3,63,4,9,3,3,3,8,4,62,3,9,3,5,3,8,3,62,3,9,2,7,2,8,3,62,3,8,3,7,3,7,3,61,3,9,3,7,3,8,3,60,3,9,3,7,3,8,3,60,3,9,3,7,3,8,3,60,3,9,3,7,3,8,3,61,3,9,2,7,2,8,3,62,3,9,3,5,3,8,3,62,3,10,3,3,3,9,3,62,4,10,7,9,4,63,3,11,5,10,3,64,4,24,4,64,5,22,5,65,5,20,5,66,6,18,6,67,6,16,6,69,6,14,6,71,7,10,7,73,9,4,9,76,18,81,12,88,4,76};
static const rle rle_co2_icon = {96, 96, co2_icon};


MenuItem menuItems[] = {{"Set Time", &setTimeScreen},
                        {"Setup WiFi", &setupWifiScreen},
                        {"Sync Time", &syncTimeScreen},
                        {"Set Location", &setLocationScreen},
                        {"Get Weather", &getWeatherScreen},
                        {"Buzz", &buzzScreen}};

MenuScreen menu(menuItems, sizeof(menuItems) / sizeof(menuItems[0]));

WatchyC4 timeScreen;
WeatherScreen weatherScreen;
IconScreen co2(&rle_co2_icon, "CO2", OptimaLTStd22pt7b);
IconScreen battery(&rle_battery, "battery", OptimaLTStd22pt7b);
IconScreen steps(&rle_steps, "steps", OptimaLTStd22pt7b);
IconScreen orientation(&rle_orientation, "orientation", OptimaLTStd22pt7b);
IconScreen bluetooth(&rle_bluetooth, "bluetooth", OptimaLTStd22pt7b);
IconScreen wifi(&rle_wifi, "wifi", OptimaLTStd22pt7b);
IconScreen settings(&rle_settings, "settings", OptimaLTStd22pt7b);
IconScreen text(&rle_text, "wrap text", OptimaLTStd22pt7b);
ImageScreen weather(cloud, 96, 96, "weather", OptimaLTStd22pt7b);
ShowBatteryScreen showBattery;
ShowCO2Screen showCO2;
ShowBluetoothScreen showBluetooth;
ShowOrientationScreen showOrientation;
ShowStepsScreen showSteps;
ShowWifiScreen showWifi;
SensirionI2CScd4x scd4x;

RTC_DATA_ATTR struct {
    uint16_t co2;
    float temperature;
    float humidity;
} lastCO2 = { 0, 0, 0 };

CarouselItem carouselItems[] = {{&timeScreen, nullptr},
                                {&co2, &showCO2},
                                {&weather, &weatherScreen},
                                {&battery, &showBattery},
                                {&steps, &showSteps},
                                {&orientation, &showOrientation},
                                {&bluetooth, &showBluetooth},
                                {&wifi, &showWifi},
                                {&settings, &menu}};

CarouselScreen carousel(carouselItems,
                        sizeof(carouselItems) / sizeof(carouselItems[0]));

uint16_t getLastCO2(void) { return lastCO2.co2; }
float getLastTemperature(void) { return lastCO2.temperature; }
float getLastHumidity(void) { return lastCO2.humidity; }

void co2init(const esp_sleep_wakeup_cause_t wakeup_reason) {
    uint16_t dataReady, error;
    scd4x.begin(Wire);
    switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER: //ESP Internal RTC
    case ESP_SLEEP_WAKEUP_EXT0:  // RTC Alarm
    case ESP_SLEEP_WAKEUP_EXT1:  // button Press
        // if there's a measurement available, update the last measurement
        error = scd4x.getDataReadyStatus(dataReady);
        if ((!error) && (dataReady & 0x7FF) != 0) {
            scd4x.readMeasurement(
                lastCO2.co2, lastCO2.temperature, lastCO2.humidity
            );
        }
        break;
    default: // reset
        // stop potentially started measurement
        scd4x.stopPeriodicMeasurement();
        // Start measurements
        scd4x.startLowPowerPeriodicMeasurement();
        break;
    }
}

void setup() {
  LOGD(); // fail if debugging macros not defined

  Watchy::AddOnWakeCallback(co2init);

  // initializing time and location can be a little tricky, because the
  // calls can fail for a number of reasons, but you don't want to just
  // keep trying because you can't know if the error is transient or
  // persistent. So whenever we wake up, try to sync the time and location
  // if they haven't ever been synced. If there is a persistent failure
  // this can drain your battery...
  if (Watchy_SyncTime::lastSyncTimeTS == 0) {
    Watchy_SyncTime::syncTime(Watchy_GetLocation::currentLocation.timezone);
  }
  if (Watchy_GetLocation::lastGetLocationTS == 0) {
    Watchy_GetLocation::getLocation();
  }
  if (Watchy::screen == nullptr) { Watchy::screen = &carousel; }
  Watchy::init();
}

void loop() {}  // this should never run, Watchy deep sleeps after init();
