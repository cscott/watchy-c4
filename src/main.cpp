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

#include <time.h>

SetTimeScreen setTimeScreen;
SetupWifiScreen setupWifiScreen;
SyncTimeScreen syncTimeScreen;
SetLocationScreen setLocationScreen;
GetWeatherScreen getWeatherScreen;
BuzzScreen buzzScreen;

MenuItem menuItems[] = {{"Set Time", &setTimeScreen},
                        {"Setup WiFi", &setupWifiScreen},
                        {"Sync Time", &syncTimeScreen},
                        {"Set Location", &setLocationScreen},
                        {"Get Weather", &getWeatherScreen},
                        {"Buzz", &buzzScreen}};

MenuScreen menu(menuItems, sizeof(menuItems) / sizeof(menuItems[0]));

WatchyC4 timeScreen;
WeatherScreen weatherScreen;
IconScreen battery(&rle_battery, "battery", OptimaLTStd22pt7b);
IconScreen steps(&rle_steps, "steps", OptimaLTStd22pt7b);
IconScreen orientation(&rle_orientation, "orientation", OptimaLTStd22pt7b);
IconScreen bluetooth(&rle_bluetooth, "bluetooth", OptimaLTStd22pt7b);
IconScreen wifi(&rle_wifi, "wifi", OptimaLTStd22pt7b);
IconScreen settings(&rle_settings, "settings", OptimaLTStd22pt7b);
IconScreen text(&rle_text, "wrap text", OptimaLTStd22pt7b);
ImageScreen weather(cloud, 96, 96, "weather", OptimaLTStd22pt7b);
ShowBatteryScreen showBattery;
ShowBluetoothScreen showBluetooth;
ShowOrientationScreen showOrientation;
ShowStepsScreen showSteps;
ShowWifiScreen showWifi;

CarouselItem carouselItems[] = {{&timeScreen, nullptr},
                                {&weather, &weatherScreen},
                                {&battery, &showBattery},
                                {&steps, &showSteps},
                                {&orientation, &showOrientation},
                                {&bluetooth, &showBluetooth},
                                {&wifi, &showWifi},
                                {&settings, &menu}};

CarouselScreen carousel(carouselItems,
                        sizeof(carouselItems) / sizeof(carouselItems[0]));

void setup() {
  LOGD(); // fail if debugging macros not defined

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
