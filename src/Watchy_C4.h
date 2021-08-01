#ifndef WATCHY_C4_H
#define WATCHY_C4_H

#include <Watchy.h>

class WatchyC4 : public Watchy{
    public:
        WatchyC4();
        void centerjustify(const char *str);
        void drawWatchFace();
        void drawCall();
        void drawTime();
        void drawDate();
        void drawSteps();
        void drawWeather();
        void drawBattery();
};

#endif
