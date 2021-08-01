#ifndef WATCHY_C4_H
#define WATCHY_C4_H

#include <Watchy.h>
#include "Seven_Segment10pt7b.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_39.h"
#include "icons.h"

class WatchyC4 : public Watchy{
    public:
        WatchyC4();
        void drawWatchFace();
        void drawTime();
        void drawDate();
        void drawSteps();
        void drawWeather();
        void drawBattery();
};

#endif
